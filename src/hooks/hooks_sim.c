/*
 * Replacements and taps for functions inside HGSIM.DLL (and, later,
 * HGSHELL.DLL and the renderer plugins). Each entry lists the original
 * prologue so that only the known build of the game is patched.
 */
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "hook.h"
#include "../sim/simmem.h"
#include "../launcher/debug.h"
#include "../sim/platform/w28.h"

/* Appends a line to trace.log, so a crash right after still leaves the trail. */
static void TraceLog(const char *fmt, ...)
{
    char buf[512];
    va_list ap;
    FILE *fp;

    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    fp = fopen("trace.log", "at");
    if (fp != NULL) {
        fputs(buf, fp);
        fclose(fp);
    }
}

static const char *SafeString(DWORD value)
{
    const char *text = (const char *)(UINT_PTR)value;

    if (text == NULL)
        return "(null)";
    if (IsBadStringPtrA(text, 200))
        return "(bad pointer)";
    return text;
}

/*
 * The CD-music startup. The original opens the CD audio device through MCI and
 * shows "Cannot aquire audio CD device" when that fails, which it does on any
 * PC without a music CD in the drive. It returns 0 when music is ready and 1
 * when it is not; callers then run without music. Report "not available"
 * without the message box. Music will later play from files.
 */
// FUNCTION: HGSIM 0x1009dfa0
static int HookedCdAudioInit(void)
{
    return 1;
}

/* SystemError(code, format, ...): the game's fatal error report. A tap on the original only in "noconv" runs;
 * otherwise the converted Sim_100bc280 (round 28) is hooked there and reports through TraceSystemError. */
// FUNCTION: HGSIM 0x100bc280
static void __cdecl TapSystemError(const DWORD *args)
{
    TraceLog("SystemError code=0x%02lx %s\n", (unsigned long)args[0], SafeString(args[1]));
}

static void __cdecl TraceSystemError(uint32_t code, const char *format)
{
    TraceLog("SystemError code=0x%02lx %s\n", (unsigned long)code, SafeString((DWORD)(uintptr_t)format));
}

/*
 * LoadPlugin(path): loads one renderer DLL and checks it. It gives up without
 * a word when the plugin is the wrong kind, was not asked for, or its
 * CheckFunc says the hardware is unusable, and leaves the function table
 * empty, which later crashes. Repeat the checks here to say which one failed.
 */
typedef int(__cdecl *PluginDescFn)(char *kind, int *id, char *extra);
typedef int(__cdecl *PluginCheckFn)(const char *arg);

#define SIM_WANTED_NAME_RVA 0x15E57C  /* plugin file name asked for, or "" */
#define SIM_WANTED_ID_RVA   0x15E5BC  /* plugin type asked for */
#define SIM_CHECK_ARG_RVA   0x15E59C  /* argument for CheckFunc, or "" */

// FUNCTION: HGSIM 0x1003650d
static void __cdecl TapLoadPlugin(const DWORD *args)
{
    const char *path = SafeString(args[0]);
    HMODULE module = LoadLibraryA(path);
    HMODULE sim = GetModuleHandleA("HGSIM.DLL");
    const char *wantedName = (const char *)sim + SIM_WANTED_NAME_RVA;
    const char *checkArg = (const char *)sim + SIM_CHECK_ARG_RVA;
    PluginDescFn desc;
    PluginCheckFn check;
    char kind[256] = "";
    int id = -1;
    int checkResult = -1;

    if (module == NULL) {
        TraceLog("LoadPlugin(%s): LoadLibrary failed, error %lu\n", path, GetLastError());
        return;
    }
    desc = (PluginDescFn)(void *)GetProcAddress(module, "GetFuncDesc");
    check = (PluginCheckFn)(void *)GetProcAddress(module, "CheckFunc");
    if (desc != NULL)
        desc(kind, &id, NULL);
    if (check != NULL)
        checkResult = check(checkArg[0] != '\0' ? checkArg : NULL);
    TraceLog("LoadPlugin(%s): kind=%s id=%d CheckFunc=%d; game wants id=%d name='%s'\n", path,
             kind, id, checkResult, *(const int *)((const char *)sim + SIM_WANTED_ID_RVA),
             wantedName);
}

/* Reads a plugin choice: kind ("RASTER" or "SOUND"), file name, index. */
// FUNCTION: HGSIM 0x1003695b
static void __cdecl TapPluginChoice(const DWORD *args)
{
    TraceLog("PluginChoice kind=%s name=%s index=%ld\n", SafeString(args[0]), SafeString(args[1]),
             (long)args[2]);
}

static const unsigned char kPrologueCdAudioInit[] = {0x55, 0x8B, 0xEC, 0x83, 0xEC, 0x70};
static const unsigned char kPrologueSystemError[] = {0x55, 0x8B, 0xEC, 0x83, 0xEC, 0x14};
static const unsigned char kPrologueLoadPlugin[] = {0x55, 0x8B, 0xEC, 0x81, 0xEC,
                                                    0x14, 0x01, 0x00, 0x00};
static const unsigned char kProloguePluginChoice[] = {0x55, 0x8B, 0xEC, 0x68,
                                                      0x14, 0xA2, 0x13, 0x10};

static const HookDesc g_hooks[] = {
    {.module = "HGSIM.DLL", .rva = 0x9DFA0, .name = "CdAudioInit",
     .expect = kPrologueCdAudioInit, .stolen = sizeof(kPrologueCdAudioInit),
     .replacement = (void *)HookedCdAudioInit},
    {.module = "HGSIM.DLL", .rva = 0xBC280, .name = "SystemError",
     .expect = kPrologueSystemError, .stolen = sizeof(kPrologueSystemError),
     .tap = TapSystemError},
    {.module = "HGSIM.DLL", .rva = 0x3650D, .name = "LoadPlugin",
     .expect = kPrologueLoadPlugin, .stolen = sizeof(kPrologueLoadPlugin),
     .tap = TapLoadPlugin},
    {.module = "HGSIM.DLL", .rva = 0x3695B, .name = "PluginChoice",
     .expect = kProloguePluginChoice, .stolen = sizeof(kProloguePluginChoice),
     .tap = TapPluginChoice},
};

void Hooks_OnModuleLoaded(const char *moduleName, HMODULE module)
{
    int count;

    if (getenv("HG_NO_HOOKS") != NULL)
        return;
    if (_stricmp(moduleName, "HGSIM.DLL") == 0 && !Hooks_ConfigWord("noconv")) {
        /* the converted SystemError, LoadPlugin and PluginChoice take the taps' place (conv_w28_all.inc) */
        HookDesc use[sizeof(g_hooks) / sizeof(g_hooks[0])];
        size_t i, n = 0;

        for (i = 0; i < sizeof(g_hooks) / sizeof(g_hooks[0]); i++)
            if (g_hooks[i].rva != 0xBC280 && g_hooks[i].rva != 0x3650D && g_hooks[i].rva != 0x3695B)
                use[n++] = g_hooks[i];
        g_w28ErrorTrace = TraceSystemError;
        g_w28Trace = TraceLog;
        count = Hook_InstallModule(module, moduleName, use, n);
    } else {
        count = Hook_InstallModule(module, moduleName, g_hooks, sizeof(g_hooks) / sizeof(g_hooks[0]));
    }
    DebugPrintf("%s: %d hook(s) installed\n", moduleName, count);
    if (_stricmp(moduleName, "HGSHELL.DLL") == 0)
        HooksDDraw_OnShellLoaded(module);
    if (_stricmp(moduleName, "HGSIM.DLL") == 0) {
        g_simBase = (unsigned char *)module;
        HooksView_OnSimLoaded(module);
        HooksMem_OnSimLoaded(module);
        HooksCore_OnSimLoaded(module);
        HooksConv_OnSimLoaded(module);
        HooksInput_OnSimLoaded(module);
        HooksAudio_OnSimLoaded(module);
        HooksUninit_OnSimLoaded(module);
        HooksHud_OnSimLoaded(module);
        HooksDispProbe_OnSimLoaded(module);
        HooksDamageLog_OnSimLoaded(module);
    }
}
