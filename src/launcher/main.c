/*
 * HG.EXE: the Heavy Gear launcher.
 *
 * Creates the game window, then alternates between the front end
 * (HGSHELL.DLL) and the simulation (HGSIM.DLL). The shell writes the mission
 * setup to hgprm.cfg before returning; the launcher hands it to the sim.
 */
#include <windows.h>

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../hooks/hook.h"
#include "debug.h"
#include "lastwriter.h"
#include "strtable.h"

#define APP_NAME "Heavy Gear"

/* Messages the DLLs send to pick who handles the shared window. */
#define WM_HG_ROUTE_DEFAULT (WM_USER + 30)
#define WM_HG_ROUTE_SIM     (WM_USER + 31)
#define WM_HG_ROUTE_SHELL   (WM_USER + 32)

enum { ROUTE_DEFAULT, ROUTE_SIM, ROUTE_SHELL };

/* hgprm.cfg, written by the shell. Only the sim command line is read here. */
/*
 * Round 33, 1.2's launcher: the last argument of SimMain and ShellMain is (flags & 2), flags being a build option
 * stored in 1.2's hg.exe after the bytes 34 12 98 40 f9 37 88 e9 (1.0 passed 1), shipped as 0. The sim reads bit 2 as
 * "no CD": without it it runs its CD search and stops when it finds no CD (1.2 0x1006fb4a, [0x1014a590]); it gets
 * bit 2 here. The shell gets the shipped 0 (RunShell's call; DECISIONS.md, round 33).
 */
#define HG_LAUNCH_FLAGS 2

#define PRM_SIZE        0x344
#define PRM_SIM_CMDLINE 0x240

#define ICON_DEFAULT 103
#define ICON_ALT     108

/* hInstance, hPrevInstance (always NULL), script or command line, ... */
typedef int(WINAPI *ShellMainFn)(HINSTANCE, HINSTANCE, LPSTR, int, HWND);
typedef int(WINAPI *SimMainFn)(HINSTANCE, HINSTANCE, LPSTR, int, int, HWND);

/*
 * The ANET2 networking library is loaded by name only so it stays resident
 * while a network game runs. HG.EXE never fills this table of 55 entry
 * points; it only clears it before unloading the DLL.
 */
// GLOBAL: HG 0x40e02c
static void *g_anetProcs[55];
// GLOBAL: HG 0x40e108
static HMODULE g_hAnet;

// GLOBAL: HG 0x40e114
static StrTable *g_strings;
// GLOBAL: HG 0x40e118
static WNDPROC g_shellWndProc;
// GLOBAL: HG 0x40e11c
static WNDPROC g_simWndProc;
// GLOBAL: HG 0x40e120
static int g_route;
// GLOBAL: HG 0x40e124
static int g_classRegistered;

/* The original read the name into the 12-byte "shell.txt" literal itself. */
// GLOBAL: HG 0x40b610
static char g_shellFile[MAX_PATH] = "shell.txt";

// GLOBAL: HG 0x40e424
static const char *g_msgInsertCd;
// GLOBAL: HG 0x40e428
static const char *g_msgSetupFailed;
// GLOBAL: HG 0x40e42c
static const char *g_msgAlreadyRunning;
// GLOBAL: HG 0x40e430
static const char *g_msgNetMechBroken;
// GLOBAL: HG 0x40e434
static const char *g_msgSeeTechHelp;
// GLOBAL: HG 0x40e438
static const char *g_msgMissingComponents;
// GLOBAL: HG 0x40e43c
static const char *g_msgUse256Colors;
// GLOBAL: HG 0x40e440
static const char *g_msgDllsMissing;
// GLOBAL: HG 0x40e444
static const char *g_msg16Colors;
// GLOBAL: HG 0x40e448
static const char *g_msgNetMechMissing;

// FUNCTION: HG 0x4011e0
static void UnloadAnet(void)
{
    memset(g_anetProcs, 0, sizeof(g_anetProcs));
    while (FreeLibrary(g_hAnet))
        ;
    g_hAnet = NULL;
}

// FUNCTION: HG 0x401350
static HMODULE LoadAnet(const char *name)
{
    g_hAnet = LoadLibraryA(name);
    return g_hAnet;
}

/* Finds "shell file = <name>" in lang.ini. Leaves out untouched if absent. */
// FUNCTION: HG 0x402030
static void ReadShellFileName(char *out)
{
    char line[256];
    FILE *fp;
    char *value;

    fp = fopen("lang.ini", "r");
    if (fp == NULL)
        return;
    memset(line, 0, sizeof(line));
    while (fgets(line, sizeof(line), fp) != NULL) {
        if (strstr(line, "shell file =") != NULL) {
            value = strchr(strtok(line, "\t\r\n"), '=');
            while (!isalpha((unsigned char)*value))
                value++;
            strcpy(out, value);
            break;
        }
    }
    fclose(fp);
}

// FUNCTION: HG 0x402150
static void LoadMessages(void)
{
    static char use256AndHelp[1024];

    ReadShellFileName(g_shellFile);
    g_strings = StrTable_Load(g_shellFile);

    g_msgAlreadyRunning = StrTable_Lookup(g_strings, "Heavy Gear is already running.");
    g_msgNetMechMissing = StrTable_Lookup(g_strings, "NetMech has not been installed.");
    g_msgNetMechBroken = StrTable_Lookup(g_strings, "NetMech has not been installed properly.");
    g_msgMissingComponents = StrTable_Lookup(g_strings,
        "Heavy Gear is unable to locate necessary program components.  Please check the "
        "\"Start In:\" directory specified in you HEAVY GEAR shortcut, or reinstall HEAVY GEAR.");
    g_msgInsertCd = StrTable_Lookup(g_strings,
        "\nPlease insert Heavy Gear CD 2: Gameplay CD into your CD-ROM drive.");
    g_msgSetupFailed = StrTable_Lookup(g_strings, "Unable to execute Heavy Gear setup program.");
    g_msg16Colors = StrTable_Lookup(g_strings,
        "Your Display Properties setting supports only 16 colors.  HEAVY GEAR requires a color "
        "palette of 256 colors.  Change your Display Properties using the Windows Control "
        "Panel, and run HEAVY GEAR again.");
    g_msgUse256Colors = StrTable_Lookup(g_strings,
        "Setting your Display Properties to use a 256-color palette may improve the "
        "performance of HEAVY GEAR: Heavy Gear movie playback.  ");
    g_msgSeeTechHelp = StrTable_Lookup(g_strings,
        "See Technical Help for more information.\n\nYou may change your Display Properties "
        "using the Windows Control Panel.  Would you\nlike to quit and change your settings now?");

    /*
     * The original strcat'ed straight onto the looked-up string. Untranslated,
     * that overwrote the literals stored after it, garbling the 16-color
     * message. Joined in a separate buffer here.
     */
    snprintf(use256AndHelp, sizeof(use256AndHelp), "%s%s", g_msgUse256Colors, g_msgSeeTechHelp);
    g_msgUse256Colors = use256AndHelp;

    g_msgDllsMissing = StrTable_Lookup(g_strings,
        "One or more DLLs not found. Please reinstall HEAVY GEAR: Heavy Gear.");

    /*
     * The original freed the table here, leaving translated messages pointing
     * at freed memory. It is kept alive instead.
     */
}

// FUNCTION: HG 0x4022a0
static LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_HG_ROUTE_DEFAULT:
        g_route = ROUTE_DEFAULT;
        break;
    case WM_HG_ROUTE_SIM:
        g_route = ROUTE_SIM;
        break;
    case WM_HG_ROUTE_SHELL:
        g_route = ROUTE_SHELL;
        break;
    }

    switch (g_route) {
    case ROUTE_SIM:
        return g_simWndProc(hwnd, msg, wParam, lParam);
    case ROUTE_SHELL:
        /* 1.2's shell draws only while it holds the focus: logged (round 33) */
        if (msg == WM_SETFOCUS || msg == WM_KILLFOCUS || msg == WM_DESTROY)
            DebugPrintf("shell window %p: %s\n", (void *)hwnd,
                        msg == WM_SETFOCUS ? "focus gained" : msg == WM_KILLFOCUS ? "focus lost" : "destroyed");
        return g_shellWndProc(hwnd, msg, wParam, HooksDDraw_TranslateMouse(msg, lParam));
    default:
        return DefWindowProcA(hwnd, msg, wParam, lParam);
    }
}

/*
 * A captioned window centred on the desktop, or a borderless one covering it
 * when the desktop is no bigger than the game.
 */
// FUNCTION: HG 0x402330
static HWND CreateMainWindow(HINSTANCE hInstance, int width, int height, int altIcon)
{
    RECT rect;
    DWORD style = WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
    int screenW;
    int screenH;
    HWND hwnd;

    if (!g_classRegistered) {
        WNDCLASSA wc;

        g_classRegistered = 1;
        wc.style = CS_VREDRAW | CS_HREDRAW | CS_BYTEALIGNCLIENT;
        wc.lpfnWndProc = MainWndProc;
        wc.cbClsExtra = 0;
        wc.cbWndExtra = 0;
        wc.hInstance = hInstance;
        wc.hIcon = LoadIconA(hInstance, MAKEINTRESOURCEA(altIcon ? ICON_ALT : ICON_DEFAULT));
        wc.hCursor = LoadCursorA(NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
        wc.lpszMenuName = NULL;
        wc.lpszClassName = APP_NAME;
        RegisterClassA(&wc);
    }

    rect.left = 0;
    rect.top = 0;
    rect.right = width;
    rect.bottom = height;
    screenW = GetSystemMetrics(SM_CXSCREEN);
    screenH = GetSystemMetrics(SM_CYSCREEN);

    if (screenW <= width && screenH <= height) {
        style = WS_POPUP;
    } else {
        AdjustWindowRect(&rect, WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, FALSE);
        rect.bottom -= rect.top;   /* height */
        rect.right -= rect.left;   /* width */
        rect.top = (screenH - rect.bottom) / 2;
        rect.left = (screenW - rect.right) / 2;
    }

    hwnd = CreateWindowExA(0, APP_NAME, APP_NAME, style, rect.left, rect.top, rect.right,
                           rect.bottom, NULL, NULL, hInstance, NULL);
    if (hwnd == NULL)
        return NULL;
    SetMenu(hwnd, NULL);
    ShowWindow(hwnd, SW_SHOWMAXIMIZED);
    UpdateWindow(hwnd);
    return hwnd;
}

// FUNCTION: HG 0x4024b0
static int RunSim(HWND hwnd, LPSTR cmdLine, int arg3, int arg4)
{
    HMODULE hSim;
    SimMainFn simMain;
    int result;

    hSim = LoadLibraryA("HGSIM.DLL");
    if (hSim == NULL) {
        DebugPrintf("LoadLibrary(HGSIM.DLL) failed, error %lu\n", GetLastError());
        return 1;
    }
    DebugPrintf("HGSIM.DLL loaded at %p%s\n", (void *)hSim,
                (UINT_PTR)hSim == 0x10000000u ? "" : " - NOT its own address 0x10000000: hooks will not fit");
    Hooks_OnModuleLoaded("HGSIM.DLL", hSim);
    simMain = (SimMainFn)(void *)GetProcAddress(hSim, "SimMain");
    if (simMain == NULL)
        return 1;
    g_simWndProc = (WNDPROC)(void *)GetProcAddress(hSim, "SimWindowProc");
    if (g_simWndProc == NULL)
        return 1;

    DebugPrintf("Entering Sim...\n");
    result = simMain(hSim, NULL, cmdLine, arg3, arg4, hwnd);
    DebugPrintf("Returned from Sim, retval=%d.\n", result);
    HooksDispProbe_OnSimExit();
    HooksConv_OnSimExit();
    HooksDamageLog_OnSimExit();
    HooksUninit_OnSimExit();
    HooksAudio_OnSimExit();
    HooksInput_OnSimExit();

    while (FreeLibrary(hSim))
        ;
    return result;
}

// FUNCTION: HG 0x402550
static int RunShell(HWND hwnd, LPSTR script, int arg3)
{
    HMODULE hShell;
    ShellMainFn shellMain;
    int result;

    hShell = LoadLibraryA("HGSHELL.DLL");
    if (hShell == NULL) {
        DebugPrintf("LoadLibrary(HGSHELL.DLL) failed, error %lu\n", GetLastError());
        MessageBoxA(NULL, g_msgDllsMissing, APP_NAME, MB_ICONHAND);
        return 1;
    }
    Hooks_OnModuleLoaded("HGSHELL.DLL", hShell);
    shellMain = (ShellMainFn)(void *)GetProcAddress(hShell, "ShellMain");
    if (shellMain == NULL)
        return 1;
    g_shellWndProc = (WNDPROC)(void *)GetProcAddress(hShell, "ShellWindowProc");
    if (g_shellWndProc == NULL)
        return 1;

    DebugPrintf("Entering Shell...\n");
    result = shellMain(hShell, NULL, script, arg3, hwnd);
    DebugPrintf("Returned from Shell, retval=%d.\n", result);
    HooksDDraw_OnShellExit();

    while (FreeLibrary(hShell))
        ;
    return result;
}

// FUNCTION: HG 0x402600
static int IsAlreadyRunning(void)
{
    if (FindWindowA(APP_NAME, APP_NAME) != NULL)
        return 1;
    return FindWindowA(APP_NAME, APP_NAME) != NULL;
}

/*
 * A DLL returns -1 to report a fatal error; any value whose low byte is 0xFF
 * means the player quit.
 */
static int IsFatal(int result) { return result == -1; }
static int IsQuit(int result) { return (result & 0xff) == 0xff; }

static void FatalDllError(void)
{
    /*
     * The original passes the number 0x485 where MessageBoxA expects a text
     * pointer; most likely a resource id that was never loaded.
     */
    MessageBoxA(NULL, (LPCSTR)(UINT_PTR)0x485, APP_NAME, MB_ICONHAND);
    exit(1);
}

/* True when hgrc.cfg (next to the exe) contains the given word. */
static int ConfigHasWord(const char *word)
{
    char text[256], *tok;
    size_t n;
    FILE *fp = fopen("hgrc.cfg", "r");

    if (fp == NULL)
        return 0;
    n = fread(text, 1, sizeof(text) - 1, fp);
    fclose(fp);
    text[n] = '\0';
    for (tok = strtok(text, " \t\r\n"); tok != NULL; tok = strtok(NULL, " \t\r\n")) {
        if (_stricmp(tok, word) == 0)
            return 1;
    }
    return 0;
}

/*
 * Added for the port. The DLLs draw through DirectDraw in 8-bit colour, which
 * stays blank on modern Windows unless the process runs with the "256Color"
 * compatibility layer. Windows applies it to the original HG.EXE by itself;
 * a rebuilt exe has to ask. The layer is read only when a process starts, so
 * relaunch once with __COMPAT_LAYER set. HG_NO_COMPAT=1 skips this.
 * Returns 1 when a relaunched copy has taken over.
 */
static int RelaunchWith256Color(void)
{
    char layer[512];
    char exePath[MAX_PATH];
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    DWORD len;

    if (getenv("HG_NO_COMPAT") != NULL || getenv("HG_RELAUNCHED") != NULL ||
        ConfigHasWord("nocompat") || Hooks_DisplayWanted())
        return 0;

    len = GetEnvironmentVariableA("__COMPAT_LAYER", layer, sizeof(layer));
    if (len >= sizeof(layer))
        return 0;
    if (len > 0 && strstr(layer, "256Color") != NULL)
        return 0;
    if (len > 0)
        strncat(layer, " 256Color", sizeof(layer) - len - 1);
    else
        strcpy(layer, "256Color");

    if (GetModuleFileNameA(NULL, exePath, sizeof(exePath)) == 0)
        return 0;
    SetEnvironmentVariableA("__COMPAT_LAYER", layer);
    SetEnvironmentVariableA("HG_RELAUNCHED", "1");

    memset(&si, 0, sizeof(si));
    si.cb = sizeof(si);
    if (!CreateProcessA(exePath, GetCommandLineA(), NULL, NULL, FALSE, 0, NULL, NULL, &si,
                        &pi)) {
        DebugPrintf("Relaunch with 256Color failed, error %lu\n", GetLastError());
        return 0;
    }
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    return 1;
}

/* Prints "MODULE.DLL+0xRVA" for an address inside a loaded module. */
static void LogAddress(FILE *fp, DWORD addr)
{
    HMODULE mod;
    char path[MAX_PATH];
    const char *name;

    if (GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                               GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                           (LPCSTR)(UINT_PTR)addr, &mod) &&
        GetModuleFileNameA(mod, path, sizeof(path)) != 0) {
        name = strrchr(path, '\\');
        fprintf(fp, "%s+0x%lx", name != NULL ? name + 1 : path,
                (unsigned long)(addr - (DWORD)(UINT_PTR)mod));
    } else {
        fprintf(fp, "(no module)");
    }
}

/*
 * Added for the port. The 1997 DLLs crash without saying where, so an
 * unhandled exception is written to crash.log: the faulting address, the
 * registers, and every value on the stack that points into a loaded module
 * (the call chain, roughly). Windows still shows its own crash report after.
 */
static LONG WINAPI CrashFilter(EXCEPTION_POINTERS *info)
{
    const EXCEPTION_RECORD *rec = info->ExceptionRecord;
    const CONTEXT *ctx = info->ContextRecord;
    FILE *fp = fopen("crash.log", "at");
    DWORD stack[48];
    SIZE_T got = 0;
    unsigned i;

    if (fp == NULL)
        return EXCEPTION_CONTINUE_SEARCH;

    fprintf(fp, "Exception 0x%08lx at 0x%08lx ", (unsigned long)rec->ExceptionCode,
            (unsigned long)(UINT_PTR)rec->ExceptionAddress);
    LogAddress(fp, (DWORD)(UINT_PTR)rec->ExceptionAddress);
    fprintf(fp, "\n");
    if (rec->ExceptionCode == EXCEPTION_ACCESS_VIOLATION && rec->NumberParameters >= 2)
        fprintf(fp, "  %s address 0x%08lx\n",
                rec->ExceptionInformation[0] == 0   ? "read from"
                : rec->ExceptionInformation[0] == 1 ? "write to"
                                                    : "execute at",
                (unsigned long)rec->ExceptionInformation[1]);
    fprintf(fp, "  eax=%08lx ebx=%08lx ecx=%08lx edx=%08lx\n", ctx->Eax, ctx->Ebx, ctx->Ecx,
            ctx->Edx);
    fprintf(fp, "  esi=%08lx edi=%08lx ebp=%08lx esp=%08lx\n", ctx->Esi, ctx->Edi, ctx->Ebp,
            ctx->Esp);

    if (ReadProcessMemory(GetCurrentProcess(), (LPCVOID)(UINT_PTR)ctx->Esp, stack, sizeof(stack),
                          &got)) {
        fprintf(fp, "  stack values that point into modules:\n");
        for (i = 0; i < got / sizeof(DWORD); i++) {
            HMODULE mod;

            if (GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                                       GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                                   (LPCSTR)(UINT_PTR)stack[i], &mod)) {
                fprintf(fp, "    esp+0x%02x: 0x%08lx ", i * 4, (unsigned long)stack[i]);
                LogAddress(fp, stack[i]);
                fprintf(fp, "\n");
            }
        }
    }
    fprintf(fp, "\n");
    fclose(fp);
    return EXCEPTION_CONTINUE_SEARCH;
}

/*
 * Added for the port. The shell asks the sim for the 3dfx renderer (/3dfx)
 * and the sim picks its renderer plugin by that switch alone: /3dfx is the
 * Glide plugin, /d3d the DirectDraw/Direct3D one, no switch the software
 * ones. Glide needs glide2x.dll, a 1997 driver that no longer exists, and the
 * sim crashes without saying why when the plugin cannot load. So /3dfx becomes
 * /d3d unless Glide is installed. HG_RENDERER (or hgrc.cfg) overrides: glide, d3d, soft or hgvk.
 * `cmd` is upper case and has room to grow by a few characters.
 */
/*
 * Set by ChooseRenderer when the Vulkan or the Direct3D plugin is in use; the sim then runs in a second copy of
 * this exe. Vulkan: the Windows 256-colour compatibility layer the menus need makes the 32-bit NVIDIA Vulkan driver
 * crash, and the child runs without it. Direct3D: after a mission that ends with code 0 the menu stays black when
 * the sim's DirectDraw ran in the menu's process (the original code too; DECISIONS.md, round 21), so the sim's
 * DirectDraw lives and ends in the child.
 */
static int g_isolateSim;

/*
 * Runs the sim in a child process that does not have the compatibility layer.
 * The child takes the same command line (the "development shortcut" path of
 * WinMain) and returns the sim's result as its exit code. Returns 0 when the
 * child could not be started, so the caller can run the sim in this process.
 */
static int RunSimIsolated(HWND hwnd, const char *cmd, int *result)
{
    char exePath[MAX_PATH];
    char *cmdLine, *envBlock, *dst;
    char *env;
    const char *src;
    size_t envLen = 0, cmdLen;
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    DWORD code = 1;
    BOOL ok;

    /* Round 24 diagnostic: the same child under lastwriter.c's debugger (hgrc.cfg "lastwriter"). */
    if (LastWriter_Wanted()) {
        ShowWindow(hwnd, SW_HIDE);
        ok = LastWriter_RunSim(cmd, result);
        ShowWindow(hwnd, SW_SHOWMAXIMIZED);
        if (ok)
            return 1;
    }
    if (GetModuleFileNameA(NULL, exePath, sizeof(exePath)) == 0)
        return 0;
    cmdLen = strlen(exePath) + strlen(cmd) + 8;
    cmdLine = (char *)malloc(cmdLen);
    env = GetEnvironmentStringsA();
    if (cmdLine == NULL || env == NULL) {
        free(cmdLine);
        if (env != NULL)
            FreeEnvironmentStringsA(env);
        return 0;
    }
    sprintf(cmdLine, "\"%s\" %s", exePath, cmd);

    /* The environment without __COMPAT_LAYER, plus a marker for the child. */
    for (src = env; *src != '\0'; src += strlen(src) + 1)
        envLen += strlen(src) + 1;
    envBlock = (char *)malloc(envLen + sizeof("HG_CHILD=1") + 1);
    if (envBlock == NULL) {
        free(cmdLine);
        FreeEnvironmentStringsA(env);
        return 0;
    }
    dst = envBlock;
    for (src = env; *src != '\0'; src += strlen(src) + 1) {
        if (_strnicmp(src, "__COMPAT_LAYER=", 15) == 0)
            continue;
        strcpy(dst, src);
        dst += strlen(src) + 1;
    }
    strcpy(dst, "HG_CHILD=1");
    dst += sizeof("HG_CHILD=1");
    *dst = '\0';
    FreeEnvironmentStringsA(env);

    memset(&si, 0, sizeof(si));
    si.cb = sizeof(si);
    ok = CreateProcessA(exePath, cmdLine, NULL, NULL, FALSE, 0, envBlock, NULL, &si, &pi);
    free(envBlock);
    free(cmdLine);
    if (!ok) {
        DebugPrintf("Starting the sim without the compatibility layer failed, error %lu\n",
                    GetLastError());
        return 0;
    }
    DebugPrintf("Sim started in a separate process: %s\n", cmd);

    ShowWindow(hwnd, SW_HIDE);
    for (;;) {
        MSG msg;
        DWORD wait = MsgWaitForMultipleObjects(1, &pi.hProcess, FALSE, INFINITE, QS_ALLINPUT);

        if (wait == WAIT_OBJECT_0)
            break;
        while (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
    }
    GetExitCodeProcess(pi.hProcess, &code);
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    ShowWindow(hwnd, SW_SHOWMAXIMIZED);
    /* round 33: as in 1998, when the game window was the sim's own and kept the focus: the 1.2 shell that starts
       next reads its focus once (GetFocus() == its window) and draws nothing without it */
    SetForegroundWindow(hwnd);
    SetActiveWindow(hwnd);              /* GetFocus / GetActiveWindow are per thread: make them ours again */
    SetFocus(hwnd);
    DebugPrintf("Sim process ended: game window %p brought back, focus %p, foreground %p\n", (void *)hwnd,
                (void *)GetFocus(), (void *)GetForegroundWindow());
    *result = (int)code;
    return 1;
}

/* Always-on note in trace.log, next to the hook taps, of what the launcher decided. */
static void LogRenderer(const char *when, const char *want, const char *cmd)
{
    FILE *fp = fopen("trace.log", "a");

    if (fp != NULL) {
        fprintf(fp, "ChooseRenderer %s: HG_RENDERER=%s cmd='%s'\n", when,
                want != NULL ? want : "(unset)", cmd);
        fclose(fp);
    }
}

/*
 * The renderer choice: the HG_RENDERER environment variable, or else the first
 * word of hgrc.cfg next to the exe (tools/dev/dev.cmd writes it). Returns
 * NULL when neither is set.
 */
static const char *GetRendererSetting(char *buf, size_t size)
{
    const char *env = getenv("HG_RENDERER");
    FILE *fp;

    if (env != NULL)
        return env;
    fp = fopen("hgrc.cfg", "r");
    if (fp == NULL)
        return NULL;
    buf[0] = '\0';
    if (fscanf(fp, "%31s", buf) != 1 || size < 2)
        buf[0] = '\0';
    fclose(fp);
    return buf[0] != '\0' ? buf : NULL;
}

static void ChooseRenderer(char *cmd)
{
    static const char *const switches[] = {"/3DFX", "-3DFX", "/D3D", "-D3D"};
    char setting[32];
    const char *want = GetRendererSetting(setting, sizeof(setting));
    const char *replacement;
    size_t i;

    LogRenderer("before", want, cmd);
    if (want == NULL) {
        HMODULE glide = LoadLibraryA("glide2x.dll");

        if (glide != NULL) {
            FreeLibrary(glide);
            return;
        }
        want = "d3d";
    }
    if (_stricmp(want, "glide") == 0)
        return;
    /* HGVK.DLL (src/plugins/hgvk) speaks the /3dfx display contract. */
    replacement = _stricmp(want, "hgvk") == 0 ? "/3DFX" : _stricmp(want, "soft") == 0 ? "" : "/D3D";

    g_isolateSim = _stricmp(want, "hgvk") == 0 || _stricmp(want, "d3d") == 0;
    if (_stricmp(want, "hgvk") == 0) {
        /* The shell rewrites DLLFILE.INI with the plugin it picked (DX5DRAW). */
        FILE *ini = fopen("DLLFILE.INI", "w");

        if (ini != NULL) {
            fputs("HGVK.DLL\n", ini);
            fclose(ini);
        }
    }
    for (i = 0; i < sizeof(switches) / sizeof(switches[0]); i++) {
        char *found = strstr(cmd, switches[i]);

        if (found != NULL) {
            size_t oldLen = strlen(switches[i]);
            size_t newLen = strlen(replacement);

            memmove(found + newLen, found + oldLen, strlen(found + oldLen) + 1);
            memcpy(found, replacement, newLen);
        }
    }
    if (replacement[0] != '\0' && strstr(cmd, replacement) == NULL)
        strcat(cmd, replacement[1] == '3' ? " /3DFX" : " /D3D");
    LogRenderer("after", want, cmd);
    DebugPrintf("sim command line: %s\n", cmd);
}

// FUNCTION: HG 0x402640
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    unsigned char prm[PRM_SIZE];
    char *simCmdLine = (char *)prm + PRM_SIM_CMDLINE;
    LPSTR script;
    HMODULE hAnet;
    HWND hwnd;
    FILE *fp;
    int netGame;
    int result;
    int isChild;

    (void)hPrevInstance;
    (void)nCmdShow;

    /* Added for the port: HG_DEBUG=2 (debugger output) or 4 (debug.log). */
    if (getenv("HG_DEBUG") != NULL)
        g_debugMode = atoi(getenv("HG_DEBUG"));
    if (RelaunchWith256Color())
        return 0;
    SetUnhandledExceptionFilter(CrashFilter);

    /* The isolated sim child (RunSimIsolated) must not undo the parent's state. */
    isChild = getenv("HG_CHILD") != NULL;
    if (!isChild) {
        DeleteFileA("nmsave.cfg");
        DeleteFileA("hgprm.cfg");
    }
    LoadMessages();

    if (GetDeviceCaps(GetDC(NULL), BITSPIXEL) < 8) {
        MessageBoxA(NULL, g_msg16Colors, APP_NAME, MB_ICONHAND);
        exit(1);
    }
    if (!isChild && IsAlreadyRunning()) {
        MessageBoxA(NULL, g_msgAlreadyRunning, APP_NAME, MB_ICONHAND);
        exit(1);
    }

    hAnet = LoadAnet("ANET2.DLL");
    if (hAnet == NULL)
        DebugPrintf("LoadLibrary(ANET2.DLL) failed, error %lu\n", GetLastError());

    /* With a command line, go straight into the sim (development shortcut). */
    if (_stricmp(lpCmdLine, "") != 0) {
        hwnd = CreateMainWindow(hInstance, 640, 480, 0);
        _strupr(lpCmdLine);
        ChooseRenderer(lpCmdLine);
        /* The original tests for /N and -N here but ignores the result. */
        if (strstr(lpCmdLine, "/N") == NULL)
            (void)strstr(lpCmdLine, "-N");
        if (hAnet != NULL)
            UnloadAnet();
        /* Round 24 diagnostic: in a watcher run the sim runs in a child under lastwriter.c's debugger. */
        if (LastWriter_Wanted()) {
            ShowWindow(hwnd, SW_HIDE);
            if (LastWriter_RunSim(lpCmdLine, &result))
                return result;
            ShowWindow(hwnd, SW_SHOWMAXIMIZED);
        }
        result = RunSim(hwnd, lpCmdLine, 0, HG_LAUNCH_FLAGS & 2);
        /* round 33: an isolated sim child holds the foreground; let the parent's game window have it back (the
           1.2 shell reads its focus once when it starts: without it, a black menu after a mission) */
        AllowSetForegroundWindow(ASFW_ANY);
        return result;
    }

    hwnd = CreateMainWindow(hInstance, 640, 480, 0);
    if (hAnet == NULL)
        hAnet = LoadAnet("ANET2.DLL");

    script = "intro";
    for (;;) {
        /* the shell gets 1.2's shipped 0: it sets its flag [0x10259598] to (arg == 1 ? 0 : !(arg & 2)), so only the
           shipped 0 gives its full setup (a list of 0x49 entries, as 1.0's shell always has; 1 or 2 give 0x33); its
           CD search with that flag runs only for the DVD edition's movie path (wavmap.ini), which this install has not */
        result = RunShell(hwnd, script, 0);
        if (IsFatal(result))
            FatalDllError();
        else if (IsQuit(result))
            return result;

        fp = fopen("hgprm.cfg", "rb");
        if (fp == NULL) {
            MessageBoxA(NULL, "Error opening Heavy Gear parameter file.", APP_NAME, MB_ICONHAND);
            exit(1);
        }
        if (fread(prm, sizeof(prm), 1, fp) != 1) {
            MessageBoxA(NULL, "Error reading Heavy Gear parameter file.", APP_NAME, MB_ICONHAND);
            exit(1);
        }
        fclose(fp);

        /* /N or -N marks a network (NetMech) game, which needs ANET2. */
        _strupr(simCmdLine);
        ChooseRenderer(simCmdLine);
        netGame = strstr(simCmdLine, "/N") != NULL || strstr(simCmdLine, "-N") != NULL;
        if (hAnet != NULL && !netGame) {
            UnloadAnet();
            hAnet = NULL;
        }

        /* Round 28: also with the SDL3 menus (no compatibility layer to get away from): in this process, after the
         * menus, SDL3 and the renderer, HGSIM.DLL's own address can be taken; it then loads elsewhere, the hooks
         * whose first bytes hold an address do not fit and the game crashed (Diego's option 1 run). */
        if (!(g_isolateSim && !netGame && RunSimIsolated(hwnd, simCmdLine, &result)))
            result = RunSim(hwnd, simCmdLine, 0, HG_LAUNCH_FLAGS & 2);
        if (IsFatal(result))
            FatalDllError();
        else if (IsQuit(result))
            return result;

        if (hAnet == NULL)
            hAnet = LoadAnet("ANET2.DLL");
        script = "sim";
    }
}
