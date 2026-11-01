#include "w28.h"
#include "../../platform/plat.h"
#include "../util/access.h"
#include "../unsorted_1005/w2g.h"
#include "../unsorted_1008/w4u.h"
#include "../unsorted_1008/w5y.h"

#define W28_WINDOW() ((void *)(uintptr_t)G32(0x1017b8f0))
#define W28_DRIVER() G32(0x1015e688)        /* the display driver's table */
#define W28_FITS() (GS32(0x1015e9c4) <= 0x280 && GS32(0x1015e9c8) <= 0x1e0 && G32(0x1017b940) == 0)

typedef void(__cdecl *W28VoidFn)(void);
typedef void(__cdecl *W28PaletteFn)(uint32_t first, uint32_t count, void *colours, uint32_t now);
typedef int32_t(__cdecl *W28IntFn)(int32_t);

int g_w28FocusPaused;

static void W28_SetPalette(void)
{
    if (W28_DRIVER() != 0)
        ((W28PaletteFn)(uintptr_t)U32(W28_DRIVER(), 0x14))(0, 0x100, GBYTES(0x1015e6a0), 1);
}

// FUNCTION: HGSIM 0x100867a2
int32_t __stdcall Sim_100867a2(uint32_t window, uint32_t message, uint32_t wParam, uint32_t lParam)
{
    void *win = (void *)(uintptr_t)window;
    uint32_t t;

    if (message >= 0x100 && message <= 0x108)
        return 0;
    switch (message) {
    case 0x0f:
        if (G32(0x1017b924) != 0 && G32(0x1015e9bc) == 2) {
            ((W28VoidFn)(uintptr_t)U32(G32(0x1015e68c), 0x18))();
            Plat_WindowPainted(win);
            return 0;
        }
        break;
    case 0x02:
        Plat_RequestQuit();
        return 0;
    case 0x1c:
        if (G32(0x1017b8b8) != 0)
            break;
        G32(0x1017b928) = wParam;
        G32(0x1017b920) = G32(0x1017b928);
        if (G32(0x1017b920) == 1) {
            /* 1.2 (0x10071397): the game resumes (0x10075070 = Sim_1008b900). Fix: only after its own pause */
#ifdef W28F_MUTATE /* the checker's negative check only */
            if (1) {
#else
            if (g_w28FocusPaused) {
#endif
                g_w28FocusPaused = 0;
                Sim_1008b900();
            }
            Sim_10057f53();
            if (W28_FITS()) {
                if (G32(0x1015e9c0) == 1) {
                    SIM_FN(W28VoidFn, 0x10037f71)();
                    G32(0x1015e9c0) = 0;
                    Plat_WindowShow(W28_WINDOW(), PLAT_SHOW_RESTORE);
                } else if (W28_DRIVER() == 0 || U32(W28_DRIVER(), 0) != 1) {
                    Plat_WindowShow(W28_WINDOW(), PLAT_SHOW_RESTORE);
                } else {
                    Plat_WindowShow(W28_WINDOW(), PLAT_SHOW_NOACTIVATE);
                }
            } else {
                Plat_WindowShow(W28_WINDOW(), PLAT_SHOW_RESTORE);
            }
            if (G32(0x1017b934) != 0)
                ((W28VoidFn)(uintptr_t)G32(0x109e3d5c))();
        } else {
            /* 1.2: the game pauses (0x1007504e = Sim_1008b8de). Fix: not while the clock is already held (timer
             * flag 0x200: the HUD menu, the hold in Sim_1002a983), whose owner resumes it; never in a network game */
            if ((G32(0x1015317c) & 0x200) == 0 && G32(0x1015ecd0) == 0) {
                g_w28FocusPaused = 1;
                Sim_1008b8de();
            }
            if (G32(0x1017b934) != 0)
                ((W28VoidFn)(uintptr_t)G32(0x109e3d60))();
            if (W28_FITS()) {
                if (W28_DRIVER() != 0 && U32(W28_DRIVER(), 0) == 0 && G32(0x1015e9c0) == 0) {
                    Plat_WindowShow(W28_WINDOW(), PLAT_SHOW_MINIMIZE);
                    G32(0x1015e9c0) = 1;
                    SIM_FN(W28VoidFn, 0x10037f71)();
                } else if (W28_DRIVER() == 0 || U32(W28_DRIVER(), 0) != 1) {
                    Plat_WindowShow(W28_WINDOW(), PLAT_SHOW_MINIMIZE);
                } else {
                    Plat_WindowShow(W28_WINDOW(), PLAT_SHOW_HIDE);
                }
            } else {
                Plat_WindowShow(W28_WINDOW(), PLAT_SHOW_MINIMIZE);
            }
        }
        W28_SetPalette();
        return 0;
    case 0x10:
        G32(0x1017b8b8) = 1;
        G32(0x1017b8bc) = G32(0x1017b8bc) + 2;
        return 0;
    case 0xa0:
        if (G32(0x1017b93c) == 0 && G32(0x1015e9bc) == 2) {
            Plat_CursorVisible(1);
            G32(0x1017b93c) = 1;
        }
        break;
    case 0x46:
        if (G32(0x1017b938) == 0)
            break;
        t = Plat_EventTime();
        if (t > G32(0x1015dbc8) && (G32(0x1015dbc8) > G32(0x1015da2c) || t < G32(0x1015da2c))) {
            G32(0x1017b938) = 0;
        } else {
            U32(lParam, 8) = G32(0x109e3780);
            U32(lParam, 0xc) = G32(0x109e3784);
            U32(lParam, 0x10) = G32(0x109e3788);
#ifdef W28W_MUTATE /* the checker's negative check only */
            U32(lParam, 0x14) = G32(0x109e3788);
#else
            U32(lParam, 0x14) = G32(0x109e378c);
#endif
        }
        return 0;
    case 0x200:
        if (G32(0x1017b93c) != 0 &&
            (G32(0x1017b92c) == 0 || SIM_FN(W28IntFn, 0x100766ca)(5) == 1)) {
            Plat_CursorVisible(0);
            G32(0x1017b93c) = 0;
        }
        return 0;
    case 0x30f:
        if (W28_DRIVER() != 0) {
            W28_SetPalette();
            return 1;
        }
        return 0;
    default:
        break;
    }
    return Plat_WindowDefault(win, message, wParam, lParam);
}
