#include "w7m.h"
#include "../../platform/plat.h"
#include "w2e.h"
#include "w5b.h"
#include "w6e.h"
#include "../unsorted_100f/w2e.h"
#include "../unsorted_100f/w6i.h"

typedef uint32_t(__cdecl *W7mFuncDescFn)(char *kind, uint32_t *id, char *text);
typedef uint32_t(__cdecl *W7mCheckFn)(uint32_t arg);
typedef int32_t(__stdcall *W7mSetWindowLongFn)(uint32_t window, int32_t index, uint32_t value);
typedef int(__stdcall *W7mSetWindowPosFn)(uint32_t window, int32_t after, int32_t x, int32_t y, int32_t cx, int32_t cy,
                                          uint32_t flags);
typedef uint32_t(__stdcall *W7mTimeGetTimeFn)(void);
typedef int32_t(__stdcall *W7mShowCursorFn)(int show);
typedef int32_t(__stdcall *W7mGetSystemMetricsFn)(int index);
typedef uint32_t(__stdcall *W7mGetModuleHandleFn)(const char *name);

// FUNCTION: HGSIM 0x10039540
const char *__cdecl Sound_ErrorName(int32_t hr)
{
    uint32_t text;

    switch ((uint32_t)hr) {
    case 0x8878000au: text = 0x1013a55c; break;     /* DSERR_ALLOCATED */
    case 0x8878001eu: text = 0x1013a56c; break;     /* DSERR_CONTROLUNAVAIL */
    case 0x80070057u: text = 0x1013a584; break;     /* DSERR_INVALIDPARAM */
    case 0x88780032u: text = 0x1013a598; break;     /* DSERR_INVALIDCALL */
    case 0x80004005u: text = 0x1013a5ac; break;     /* DSERR_GENERIC */
    case 0x88780046u: text = 0x1013a5bc; break;     /* DSERR_PRIOLEVELNEEDED */
    case 0x8007000eu: text = 0x1013a5d4; break;     /* DSERR_OUTOFMEMORY */
    case 0x88780064u: text = 0x1013a5e8; break;     /* DSERR_BADFORMAT */
    case 0x80004001u: text = 0x1013a5f8; break;     /* DSERR_UNSUPPORTED */
    case 0x88780078u: text = 0x1013a60c; break;     /* DSERR_NODRIVER */
    case 0x88780082u: text = 0x1013a61c; break;     /* DSERR_ALREADYINITIALIZED */
    case 0x80040110u: text = 0x1013a638; break;     /* DSERR_NOAGGREGATION */
    case 0x88780096u: text = 0x1013a64c; break;     /* DSERR_BUFFERLOST */
    case 0x887800a0u: text = 0x1013a660; break;     /* DSERR_OTHERAPPHASPRIO */
    case 0x887800aau: text = 0x1013a678; break;     /* DSERR_UNINITIALIZED */
    default: text = 0x1013a68c; break;              /* "Unknown HRESULT" */
    }
    return SIM_PTR(const char, text);
}

// FUNCTION: HGSIM 0x100361d8
void __cdecl Sim_100361d8(char *name, uint8_t *list)
{
    char kind[0x100];
    uint32_t id;
    char text[0x110];
    void *module;
    W7mFuncDescFn desc;
    W7mCheckFn check;
    uint32_t ok;

    if (G32(0x1015e56c) != 0)
        return;
    /* LoadLibraryA / GetProcAddress / FreeLibrary: the platform's, which also knows the built-in renderer */
    module = Plat_LoadLibrary(name);
    if (module == 0)
        return;
    desc = (W7mFuncDescFn)Plat_LibraryFunction(module, SIM_PTR(const char, 0x1013a0b4));
    if (desc != 0) {
        ok = desc(kind, &id, text);
        if (Str_CompareNoCase(kind, SIM_PTR(const char, 0x1013a0c0)) == 0) {      /* "RASTER" */
            check = (W7mCheckFn)Plat_LibraryFunction(module, SIM_PTR(const char, 0x1013a0c8));
            if (check != 0)
                ok = check(0);
            if (ok != 0) {
                Str_Copy((char *)list + G32(0x1015e574) * 0x124, text);
                Path_StripDirectory(name);
                Str_Copy((char *)list + G32(0x1015e574) * 0x124 + 0x100, name);
                U32(list, G32(0x1015e574) * 0x124 + 0x120) = id;
                G32(0x1015e574) = G32(0x1015e574) + 1;
            }
        }
    }
    Plat_FreeLibrary(module);
}

// FUNCTION: HGSIM 0x100381d0
void __cdecl Sim_100381d0(const uint32_t *rec)
{
    void *window = (void *)(uintptr_t)G32(0x1017b8f0);
    int sw, sh;

    if (rec == 0)
        return;
    /* The original sets the window's style (SetWindowLongA: rec[2] with WS_VISIBLE, a type-2 class keeping its
       frame) and places a type-2 window (SetWindowPos to [0x109e3780..]). Now one SDL3 placement (DECISIONS.md,
       "Windows calls: the screen"): a type-2 class gets a framed window with that client rectangle, unless it
       covers the whole screen; the others (the renderer) borderless full screen, where the original's popup kept
       the window's desktop size. */
    Plat_ScreenSize(&sw, &sh);
    if (rec[1] == 2) {
        if (G32(0x1015e688) != 0 && U32(G32(0x1015e688), 0) == 0) {
            G32(0x1015da2c) = Plat_TimeMs();
            G32(0x1015dbc8) = G32(0x1015da2c) + 0xbb8;
            G32(0x1017b938) = 1;
        }
        Plat_WindowPlace(window, GS32(0x109e3788) >= sw && GS32(0x109e378c) >= sh, GS32(0x109e3780),
                         GS32(0x109e3784), GS32(0x109e3788), GS32(0x109e378c));
        if (G32(0x1017b92c) != 0 && W7M_CALL_100766ca(5) == 0)
            Plat_CursorVisible(1);
    } else {
        Plat_WindowPlace(window, 1, 0, 0, sw, sh);
        if (G32(0x1017b92c) != 0)
            Plat_CursorVisible(0);
    }
    if (sw <= GS32(0x109e3d14) && sh <= GS32(0x109e3d18))
        G32(0x1015e9bc) = 1;
    else
        G32(0x1015e9bc) = rec[1];
}

// FUNCTION: HGSIM 0x1003c5a0
uint32_t __cdecl Sim_1003c5a0(const char *name)
{
    G32(0x1015eb68) = SIM_API(W7mGetModuleHandleFn, 0x10ac55ec)(name);
    W7M_CALL_1003be30();
    return G32(0x1015eb68);
}

// FUNCTION: HGSIM 0x1003ca04
void __cdecl Sim_1003ca04(void)
{
    int32_t pick = 3;
    int32_t phase;
    int32_t i;

    if (S16(G32(0x109e88e0), 6) >= 1 && G32(0x109b9e08) == 1) {
        Sim_1003cbb3(0, 0);
        G32(0x109b9e08) = 0;
    }
    if (GS32(0x1015eb98) < 2) {
        G32(0x1015eb98) = G32(0x1015eb98) + 1;
        G32(0x109b9df8) = 0;
        return;
    }
    if (GS32(0x1015dc78) < GS32(0x109b9df8))
        return;
    G32(0x109b9df8) = G32(0x1015dc78) + 0x712;
    phase = (int32_t)((uint32_t)(GS32(0x1015dc78) / 0xb5) + G32(0x1013abd4)) % GS32(0x1013abc8);
    if (phase < GS32(0x1013abd0)) {
        G32(0x1015ebb8) = G32(0x1015ebb8) + 1;
        G32(0x1015ebb8) = (uint32_t)(GS32(0x1015ebb8) % GS32(0x1013abcc));
    }
    G32(0x1013abd0) = (uint32_t)phase;
    for (i = 0; i <= 3; i++) {
        if (GS32(0x1013abd0) >= SIM_PTR(s32a, 0x1015eba0)[i])
            pick = i;
    }
    Sim_1003cb21(pick);
}
