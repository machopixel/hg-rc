#include "w4u.h"
#include "../../platform/plat.h"
#include "w3p.h"
#include "../core/core.h"
#include "../win/w4u_api.h"

typedef void(__cdecl *W4uLog1Fn)(const void *format);
typedef void(__cdecl *W4uLog7Fn)(const void *format, uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t e,
                                 uint32_t f);
typedef void(__cdecl *W4uMethod0Fn)(void);
typedef uint32_t(__cdecl *W4uMethod3Fn)(uint32_t a, uint32_t b, uint32_t c);
typedef void(__cdecl *W4uMethod4Fn)(uint32_t a, uint32_t b, uint32_t c, uint32_t d);

/* The original's QueryPerformanceCounter: the platform's fine clock (SDL3), which does not fail. */
static void W4u_Counter(uint32_t *out)
{
    uint64_t now = Plat_CounterNow();

    out[0] = (uint32_t)now;
    out[1] = (uint32_t)(now >> 32);
}

#define CURRENT G32(0x1015e68c)
#define ACTIVE G32(0x1015e694)
#define STEP G32(0x1015e9cc)

// FUNCTION: HGSIM 0x10037f71
void __cdecl ToggleFullScreen(void)
{
    uint32_t target = 0;
    int i;

    if (ACTIVE != 0)
        return;
    if (G32(0x1015e9c0) == 0 && !(GS32(0x1015e9c4) > GS32(0x109e3d14) || GS32(0x1015e9c8) > GS32(0x109e3d18)))
        return;
    SIM_FN(W4uLog1Fn, 0x10083495)(GBYTES(0x1013a370));
    Timer_Pause(0x80, 1);
    if (U32(G32(0x1015e688), 0) == 3) {
        /* Deviation (Diego, 2026-11-02: "Only add Fullscreen or windowed"): with the renderer (display class 3,
           Vulkan) Alt+Enter switches the window between borderless full screen and a framed window, keeping the
           renderer. The original looked for a windowed display driver (the GDI one, mode 5) and moved the whole
           display to it, which cannot show the 3D. */
        void *window = *SIM_PTR(void *, 0x1017b8f0);
        int sw, sh, w, h;

        if (Plat_WindowIsFullscreen(window)) {
            Plat_ScreenSize(&sw, &sh);
            Plat_WindowedSize(&w, &h);
            Plat_WindowPlace(window, 0, (sw - w) / 2, (sh - h) / 2, w, h);
        } else {
            Plat_WindowPlace(window, 1, 0, 0, 0, 0);
        }
        if (G32(0x1017b92c) == 0) {
            SIM_FN(W4uLog1Fn, 0x10083495)(GBYTES(0x1013a498));
            Timer_Pause(0x80, 0);
        }
        return;
    }
    if (U32(G32(0x1015e688), 4) == 1) {
        for (i = 0; i < 7; i++) {
            uint32_t other;

            target = SIM_PTR(u32a, 0x1013a2a8)[i];
            other = SIM_PTR(u32a, 0x1013a298)[U32(target, 4)];
            if (U32(target, 8) != 0 && U32(other, 4) == 2)
                break;
        }
        if (i == 7) {
            SIM_FN(W4uLog1Fn, 0x10083490)(GBYTES(0x1013a398));
            if (G32(0x1017b92c) == 0) {
                SIM_FN(W4uLog1Fn, 0x10083495)(GBYTES(0x1013a3ec));
                Timer_Pause(0x80, 0);
            }
            return;
        }
    } else {
        if (G32(0x1015e690) == 0) {
            SIM_FN(W4uLog1Fn, 0x10083490)(GBYTES(0x1013a414));
            if (G32(0x1017b92c) == 0) {
                SIM_FN(W4uLog1Fn, 0x10083495)(GBYTES(0x1013a470));
                Timer_Pause(0x80, 0);
            }
            return;
        }
        target = G32(0x1015e690);
        /* the original's GetWindowRect: the client rectangle, as the window is placed by it (Plat_WindowPlace) */
        Plat_WindowRect(*SIM_PTR(void *, 0x1017b8f0), SIM_PTR(int32_t, 0x109e3780));
        G32(0x109e3788) = G32(0x109e3788) - G32(0x109e3780);
        G32(0x109e378c) = G32(0x109e378c) - G32(0x109e3784);
    }
    (*(W4uMethod0Fn *)(BYTES(CURRENT) + 0x14))();
    CURRENT = target;
    (*(W4uMethod3Fn *)(BYTES(CURRENT) + 0x10))(G32(0x1015e698), G32(0x109e3d14), G32(0x109e3d18));
    (*(W4uMethod4Fn *)(BYTES(G32(0x1015e688)) + 0x14))(0, 0x100, (uint32_t)(uintptr_t)GBYTES(0x1015e6a0), 1);
    G32(0x10179d68) = 1;
    if (G32(0x1017b92c) == 0) {
        SIM_FN(W4uLog1Fn, 0x10083495)(GBYTES(0x1013a498));
        Timer_Pause(0x80, 0);
    }
}
