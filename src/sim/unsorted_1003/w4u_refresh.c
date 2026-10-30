#include "w4u.h"
#include "w3p.h"
#include "../core/core.h"
#include "../win/w4u_api.h"

typedef int(__stdcall *W4uQueryCounterFn)(void *counter);
typedef int(__stdcall *W4uGetWindowRectFn)(void *window, void *rect);
typedef void(__cdecl *W4uLog1Fn)(const void *format);
typedef void(__cdecl *W4uLog7Fn)(const void *format, uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t e,
                                 uint32_t f);
typedef void(__cdecl *W4uMethod0Fn)(void);
typedef uint32_t(__cdecl *W4uMethod3Fn)(uint32_t a, uint32_t b, uint32_t c);
typedef void(__cdecl *W4uMethod4Fn)(uint32_t a, uint32_t b, uint32_t c, uint32_t d);

#define W4U_IAT_QUERYPERFORMANCECOUNTER 0x10ac55e8u
#define W4U_IAT_GETWINDOWRECT 0x10ac5764u

#define CURRENT G32(0x1015e68c)
#define ACTIVE G32(0x1015e694)
#define STEP G32(0x1015e9cc)

// FUNCTION: HGSIM 0x10037cab
void __cdecl Sim_10037cab(void)
{
    uint32_t now[2];

    if (S32(CURRENT, 0) > GS32(0x1013a2c4)) {
        ACTIVE = 0;
        return;
    }
    if (STEP == 1) {
        if (SIM_API(W4uQueryCounterFn, W4U_IAT_QUERYPERFORMANCECOUNTER)(GBYTES(0x1015e680)) == 0) {
            ACTIVE = 0;
            return;
        }
        STEP = STEP + 1;
        return;
    }
    if (STEP != 5) {
        STEP = STEP + 1;
        return;
    }
    if (SIM_API(W4uQueryCounterFn, W4U_IAT_QUERYPERFORMANCECOUNTER)(now) == 0) {
        ACTIVE = 0;
        return;
    }
    /* The original tests whether the high words are equal, but computes the same difference either way. */
    U32(CURRENT, 0xc) = now[0] - G32(0x1015e680);
    SIM_FN(W4uLog7Fn, 0x10083495)(GBYTES(0x1013a2ec), U32(CURRENT, 0), G32(0x1015e680), G32(0x1015e684), now[0],
                                  now[1], U32(CURRENT, 0xc));
    if (S32(CURRENT, 0) == GS32(0x1013a2c4)) {
        ACTIVE = 0;
    } else {
        while (S32(CURRENT, 0) < GS32(0x1013a2c4)) {
            (*(W4uMethod0Fn *)(BYTES(CURRENT) + 0x14))();
            CURRENT = SIM_PTR(u32a, 0x1013a2ac)[U32(CURRENT, 0)];
            if (U32(CURRENT, 8) != 0
                && (*(W4uMethod3Fn *)(BYTES(CURRENT) + 0x10))(G32(0x1015e698), G32(0x109e3d14),
                                                               G32(0x109e3d18)) == 0) {
                STEP = 0;
                break;
            }
            W4U_LOG(0x1013a320, U32(CURRENT, 0));
            U32(CURRENT, 8) = 0;
            if (S32(CURRENT, 0) == GS32(0x1013a2c4))
                ACTIVE = 0;
        }
    }
    if (ACTIVE == 0)
        Sim_10037e8f();
}

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
        SIM_API(W4uGetWindowRectFn, W4U_IAT_GETWINDOWRECT)(*SIM_PTR(void *, 0x1017b8f0), GBYTES(0x109e3780));
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
