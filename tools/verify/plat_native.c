/*
 * Round P1 / P2: the platform functions moved from Windows calls to SDL3, against the Windows calls they replace,
 * natively on Windows (not the emulated checker: the checker stands in for the platform layer itself).
 *   P1: the current folder (and a drive's root), the program's folder, libraries (load / function / free, a missing
 *       one, the built-in table), the coarse clock over a sleep, the debug text (captured from SDL's log).
 *   P2: every key code our input produces, Plat_KeyChar against Windows' character of the same physical key, under
 *       the US and the Spanish keyboard layout (loaded for this program only and unloaded again if it was not
 *       there before), with the same-code lookup the game used before for comparison, and
 *       GetKeyState against Plat_KeyState for Shift / Ctrl / Alt held and Caps / Num Lock toggled.
 * Build (x86 developer prompt, from the repo root, after the Release build):
 *   cl /nologo /O2 /I src /I build\_deps\sdl3-src\include tools\verify\plat_native.c src\platform\plat_system.c
 *      src\platform\input_sdl3.c build\_deps\sdl3-build\Release\SDL3-static.lib user32.lib gdi32.lib shell32.lib
 *      advapi32.lib ole32.lib oleaut32.lib imm32.lib version.lib winmm.lib setupapi.lib cfgmgr32.lib
 *      /Fe:plat_native.exe
 * Prints every difference and the counts; exit code 0 when everything the game uses matches.
 */
#include <stdio.h>
#include <string.h>
#include <windows.h>

#include <SDL3/SDL.h>

#include "platform/plat.h"
#include "platform/plat_sdl3.h"

/* input_sdl3.c's other services want these; the test never opens the input */
SDL_Window *PlatSdl_Window(void *native)
{
    (void)native;
    return NULL;
}
void PlatSdl_Release(void)
{
}
int PlatSdl_Video(void)
{
    return 0;
}

static int g_fail, g_checks;

static void Check(int ok, const char *what)
{
    g_checks++;
    if (!ok) {
        g_fail++;
        printf("DIFF %s\n", what);
    }
}

/* ---- P1 ------------------------------------------------------------------------------------------------- */

static char g_logged[512];

static void SDLCALL Capture(void *userdata, int category, SDL_LogPriority priority, const char *message)
{
    (void)userdata, (void)category, (void)priority;
    SDL_strlcpy(g_logged, message, sizeof(g_logged));
}

static void *__cdecl FakeRenderer(const char *name)
{
    return strcmp(name, "Render") == 0 ? (void *)&FakeRenderer : NULL;
}

static void P1(void)
{
    char a[MAX_PATH], b[MAX_PATH], line[600], saved[MAX_PATH];
    DWORD na, nb;
    HMODULE k32;
    void *lib, *bi;
    DWORD t0, t1;
    uint32_t s0, s1;
    char *slash;

    /* current folder: here, then a drive's root, then a buffer too small */
    na = GetCurrentDirectoryA(sizeof(a), a);
    nb = Plat_CurrentDir(b, sizeof(b));
    sprintf(line, "current folder: Windows '%s' (%lu), new '%s' (%lu)", a, (unsigned long)na, b, (unsigned long)nb);
    Check(na == nb && strcmp(a, b) == 0, line);
    printf("  %s\n", line);
    strcpy(saved, a);
    SetCurrentDirectoryA("C:\\");
    na = GetCurrentDirectoryA(sizeof(a), a);
    nb = Plat_CurrentDir(b, sizeof(b));
    sprintf(line, "drive root: Windows '%s' (%lu), new '%s' (%lu)", a, (unsigned long)na, b, (unsigned long)nb);
    Check(na == nb && strcmp(a, b) == 0, line);
    printf("  %s\n", line);
    SetCurrentDirectoryA(saved);
    na = GetCurrentDirectoryA(4, a);
    nb = Plat_CurrentDir(b, 4);
    sprintf(line, "too small a buffer: Windows %lu, new %lu", (unsigned long)na, (unsigned long)nb);
    Check(na == nb, line);

    /* the program's folder: Windows' full path without its file name, against the new one without "HG" */
    na = GetModuleFileNameA(NULL, a, sizeof(a));
    slash = strrchr(a, '\\');
    if (slash != NULL)
        slash[1] = 0;
    nb = Plat_ProgramPath(b, sizeof(b));
    sprintf(line, "program folder: Windows '%s', new '%s'", a, b);
    Check(nb > 2 && strcmp(b + nb - 2, "HG") == 0 && strncmp(a, b, nb - 2) == 0 && strlen(a) == nb - 2, line);
    printf("  %s\n", line);

    /* libraries: a system library, a function in it, a missing function, a missing library, free */
    k32 = GetModuleHandleA("kernel32.dll");
    lib = Plat_LoadLibrary("kernel32.dll");
    Check(lib == (void *)k32, "kernel32.dll: the same module");
    Check(Plat_LibraryFunction(lib, "GetTickCount") == (void *)GetProcAddress(k32, "GetTickCount"),
          "GetTickCount: the same address");
    Check(Plat_LibraryFunction(lib, "NoSuchFunctionHere") == NULL, "a missing function: NULL");
    Plat_FreeLibrary(lib);
    Check(Plat_LoadLibrary("no_such_library_hg.dll") == NULL, "a missing library: NULL");
    lib = Plat_LoadLibrary("winmm.dll");
    Check(lib != NULL && Plat_LibraryFunction(lib, "timeGetTime") != NULL, "winmm.dll and timeGetTime");
    Plat_FreeLibrary(lib);
    /* the built-in table: by file name, letter case aside, through a path */
    Plat_AddBuiltinLibrary("HGVK.DLL", FakeRenderer);
    bi = Plat_LoadLibrary("plugin\\hgvk.dll");
    Check(bi != NULL && Plat_LibraryFunction(bi, "Render") == (void *)&FakeRenderer, "built-in renderer by name");
    Check(Plat_LibraryFunction(bi, "Other") == NULL, "built-in: a missing function");
    Plat_FreeLibrary(bi);

    /* the coarse clock over a 1 s sleep */
    t0 = GetTickCount();
    s0 = Plat_TicksMs();
    Sleep(1000);
    t1 = GetTickCount();
    s1 = Plat_TicksMs();
    sprintf(line, "clock over Sleep(1000): GetTickCount %lu ms, new %u ms", (unsigned long)(t1 - t0), s1 - s0);
    Check((int)(s1 - s0) - (int)(t1 - t0) <= 20 && (int)(t1 - t0) - (int)(s1 - s0) <= 20, line);
    printf("  %s\n", line);

    /* debug text: the line end the game's texts carry is dropped (SDL adds its own) */
    SDL_SetLogOutputFunction(Capture, NULL);
    Plat_DebugText("Error loading TS_SFRCE.dll\n");
    Check(strcmp(g_logged, "Error loading TS_SFRCE.dll") == 0, "debug text with a line end");
    Plat_DebugText("plain");
    Check(strcmp(g_logged, "plain") == 0, "debug text without one");
}

/* ---- P2 ------------------------------------------------------------------------------------------------- */

/* the key codes input_sdl3.c produces for character keys (the only ones that reach the lookup) */
static int Produced(int vk)
{
    static const int extra[] = {0x08, 0x09, 0x0D, 0x1B, 0x20, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF, 0xC0, 0xDB, 0xDC,
                                0xDD, 0xDE, 0xE2, 0x6A, 0x6B, 0x6D, 0x6E, 0x6F};
    size_t i;

    if ((vk >= 0x30 && vk <= 0x39) || (vk >= 0x41 && vk <= 0x5A) || (vk >= 0x60 && vk <= 0x69))
        return 1;
    for (i = 0; i < sizeof(extra) / sizeof(extra[0]); i++) {
        if (extra[i] == vk)
            return 1;
    }
    return 0;
}

/*
 * Two comparisons per layout, for every key code our input produces:
 *  - "same code": MapVirtualKeyExA on that code (what the game got before this round: input_sdl3.c sends the US
 *    position's code, which Windows then reads with this layout's meaning);
 *  - "physical key" (the target): the character of the physical key the code stands for, as the original game got
 *    it with Windows' own key codes: US code -> scancode (US layout) -> this layout's code -> its character.
 */
static void KeysUnder(const char *id, const char *name, HKL us)
{
    HKL before[64], hkl;
    int n = GetKeyboardLayoutList(64, before), i, was = 0, vk;
    int diffSame = 0, diffPhysical = 0, unused = 0, deadKeys = 0;

    hkl = LoadKeyboardLayoutA(id, KLF_NOTELLSHELL);
    for (i = 0; i < n; i++)
        was |= before[i] == hkl;
    ActivateKeyboardLayout(hkl, 0);            /* this thread only */
    SDL_Init(SDL_INIT_VIDEO);                  /* SDL builds its key map from the active layout */
    for (vk = 0; vk < 256; vk++) {
        UINT w = MapVirtualKeyExA((UINT)vk, 2, hkl);
        UINT sc = MapVirtualKeyExA((UINT)vk, 0, us);
        UINT vkHere = sc != 0 ? MapVirtualKeyExA(sc, 1, hkl) : (UINT)vk;
        UINT phys;
        uint32_t c = Plat_KeyChar((uint32_t)vk);
        char line[200];

        /* keypad codes have no layout-dependent position: Windows maps them by code */
        if (vk >= 0x60 && vk <= 0x6f)
            vkHere = (UINT)vk;
        phys = MapVirtualKeyExA(vkHere, 2, hkl);
        if (!Produced(vk)) {
            unused += (w & 0xff) != (c & 0xff);
            continue;
        }
        if (phys & 0x80000000u)
            deadKeys++;
        /* the game keeps the low byte (int8_t cast in Input_QueueKey) */
        if ((w & 0xff) != (c & 0xff)) {
            diffSame++;
            printf("    %s key 0x%02x: same code 0x%02x, new 0x%02x, physical key 0x%02x\n", name, vk, w & 0xff,
                   c & 0xff, phys & 0xff);
        }
        sprintf(line, "%s key 0x%02x: physical key 0x%08x, new 0x%02x", name, vk, phys, c);
        if ((phys & 0xff) != (c & 0xff))
            diffPhysical++;
        Check((phys & 0xff) == (c & 0xff), line);
    }
    printf("  %s: of the key codes our input produces, %d differ from the physical key (target), %d from the "
           "same-code lookup (before this round); %d dead keys; %d differences among codes our input never "
           "produces\n", name, diffPhysical, diffSame, deadKeys, unused);
    SDL_Quit();
    if (!was)
        UnloadKeyboardLayout(hkl);
}

static void KeyStates(void)
{
    static const struct {
        int vk;
        SDL_Keymod mod;
        const char *what;
    } cases[] = {
        {0, SDL_KMOD_NONE, "nothing held"},
        {VK_SHIFT, SDL_KMOD_LSHIFT, "Shift held"},
        {VK_CONTROL, SDL_KMOD_LCTRL, "Ctrl held"},
        {VK_MENU, SDL_KMOD_LALT, "Alt held"},
    };
    static const int asked[] = {VK_SHIFT, VK_CONTROL, VK_MENU};
    BYTE state[256], saved[256];
    size_t c, k;

    GetKeyboardState(saved);
    for (c = 0; c < sizeof(cases) / sizeof(cases[0]); c++) {
        memset(state, 0, sizeof(state));
        if (cases[c].vk != 0)
            state[cases[c].vk] = 0x80;
        SetKeyboardState(state);
        SDL_SetModState(cases[c].mod);
        for (k = 0; k < 3; k++) {
            short w = GetKeyState(asked[k]);
            short n = Plat_KeyState(asked[k]);
            char line[160];

            sprintf(line, "%s: key 0x%02x Windows 0x%04x (game's 0x1000 test %d), new 0x%04x (%d)", cases[c].what,
                    asked[k], (unsigned short)w, (w & 0x1000) != 0, (unsigned short)n, (n & 0x1000) != 0);
            Check(w == n, line);
            if (c < 2 && k == 0)
                printf("  %s\n", line);
        }
    }
    /* the locks: toggled */
    memset(state, 0, sizeof(state));
    state[VK_CAPITAL] = 0x01;
    state[VK_NUMLOCK] = 0x01;
    SetKeyboardState(state);
    SDL_SetModState(SDL_KMOD_CAPS | SDL_KMOD_NUM);
    Check(GetKeyState(VK_CAPITAL) == Plat_KeyState(VK_CAPITAL), "Caps Lock toggled");
    Check(GetKeyState(VK_NUMLOCK) == Plat_KeyState(VK_NUMLOCK), "Num Lock toggled");
    SetKeyboardState(saved);
}

int main(void)
{
    printf("P1:\n");
    P1();
    printf("P2:\n");
    {
        HKL us = LoadKeyboardLayoutA("00000409", KLF_NOTELLSHELL);

        KeysUnder("00000409", "US", us);
        KeysUnder("0000040A", "Spanish", us);
    }
    SDL_Init(SDL_INIT_VIDEO);
    KeyStates();
    SDL_Quit();
    printf("%d checks, %d differences\n", g_checks, g_fail);
    return g_fail != 0;
}
