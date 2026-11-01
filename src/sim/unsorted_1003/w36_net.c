#include "w36.h"
#include "fns.h"
#include "../unsorted_1002/w3o.h"
#include "../unsorted_1004/w3p.h"
#include "../unsorted_1006/w8w.h"
#include "../unsorted_1006/w9b.h"
#include "../unsorted_1007/w2e.h"
#include "../unsorted_1009/w14.h"
#include "../unsorted_100a/w5b.h"
#include "../unsorted_100a/w5x.h"
#include "../unsorted_100a/w5y.h"
#include "../unsorted_100f/w8z.h"
#include "../util/access.h"
#include "../util/w3o.h"

/* Network round (2026-11-01): 1.2's network functions that were still original code in 1.0's form. */

#define UNIT_AT(i) G32(0x109e8b40 + 4 * (uint32_t)(i))
#define SELF G32(0x1017b8c8)
#define LAG GFL(0x1015d088)                     /* the smoothed message lag (1.2 0x107cb42c) */
#define SCALE16 GFL(0x10111528)                 /* 1 / 65536 */

// FUNCTION: HGSIM 0x1003f3f8
// FUNCTION: HGSIM12 0x10032cbe
void __cdecl Sim_1003f3f8(const uint8_t *msg, int who, int size)
{
    /* the message text: 1.2's 80-byte buffers take "'%s' %s '%s'." with two player names (44-byte fields) and
       a text, which can run past 80 bytes; a larger buffer here (the text shown is the same) */
    char text[0x100];
    uint32_t idx, unit, gear, view;
    float lag, x, y, z, ground;
    double drop;
    int32_t killer;
    uint16_t keep;

    (void)who;      /* 1.2 takes the unit from the message */
#ifndef NETDA_MUTATE /* the checker's negative check only: no guard */
    if (g_w34NetOver != 0)
        return;
#endif
    idx = msg[2];
    lag = (float)(int32_t)(G32(0x1015dc78) - U32(msg, 10));
    if ((int32_t)(G32(0x1015dc78) - U32(msg, 10)) < 0)
        lag = 0.0f;
    if (LAG == 0.0f)
        LAG = lag;
    else
        LAG = (float)((double)LAG * 0.8 + (double)lag * 0.2);
    if ((double)GS32(0x1013ac9c) < (double)LAG)
        LAG = (float)GS32(0x1013ac9c);
    if (size != 0x42 || UNIT_AT(idx) == 0 || SELF == idx || (int32_t)idx >= GS32(0x1015dc64))
        return;
    /* (1.0 also returned here for a unit of state 3 run by the local player; 1.2 does not) */
    g_net10Heard[idx] = G32(0x1015dc7c);
    if (S32(msg, 10) < (int32_t)g_net10Db20[idx])
        return;
    g_net10Db20[idx] = U32(msg, 10);
    unit = UNIT_AT(idx);
    gear = U32(unit, 0x28);
    /* 1.2: a unit whose second part is gone (its value under 1e-7) is marked */
    if (FL(U32(gear, 0x88) + 0x3c, 8) < 1e-07f && g_net10Mark[idx] == 0)     /* 1.2's float at 0x100eb700 */
        g_net10Mark[idx] = 1;
    S32(unit, 0xf0) = S16(msg, 3);
    if ((S16(msg, 3) & 0xf00) == 0x400 && (U16(msg, 7) & 0x400) != 0) {
        uint32_t k = U32(unit, 0xf0) & 0xff;

        U16(GBYTES(0x109e9ba0), k * 0x50) |= 0x20;
        U16(GBYTES(0x109e9ba6), k * 0x50) |= (uint16_t)(1u << (U32(unit, 8) & 0x1f));
    }
    U32(unit, 0x150) = U16(msg, 5);
    U16(unit, 0x1c) |= 1;
    if (idx == Sim_1003db56((int16_t)*SIM_PTR(u16a, 0x1015ecd8))) {
        G32(0x1015ec60) = 1;
        if (G32(0x1015ecd0) == 2)
            G32(0x1015dc78) = U32(msg, 10);
    }
    if ((msg[9] & 0x40) != 0)
        U32(unit, 0x15c) = 1;
    FL(unit, 0x58) = (float)S32(msg, 0xe);
    FL(unit, 0x5c) = (float)S32(msg, 0x12);
    FL(unit, 0x60) = (float)S32(msg, 0x16);
    FL(unit, 0x64) = (float)((double)S32(msg, 0x1a) * (double)SCALE16);
    FL(unit, 0x68) = (float)((double)S32(msg, 0x1e) * (double)SCALE16);
    FL(unit, 0x6c) = (float)((double)S32(msg, 0x22) * (double)SCALE16);
    FL(gear, 0x520) = (float)S32(msg, 0x26);
    FL(gear, 0x588) = (float)S32(msg, 0x2a);
    x = (float)((double)FL(gear, 0x548) * (double)LAG + (double)FL(unit, 0x58));
    y = (float)((double)FL(gear, 0x54c) * (double)LAG + (double)FL(unit, 0x5c));
    z = (float)((double)FL(gear, 0x550) * (double)LAG + (double)FL(unit, 0x60));
    ground = (float)Sim_1002d961(x, y, z);
    drop = ((double)y - (double)FL(gear, 0x520)) - (double)ground;
    if (drop < (double)GFL(0x1011152c) && U32(unit, 0x15c) == 0) {
        FL(unit, 0x58) = x;
        FL(unit, 0x5c) = y;
        FL(unit, 0x60) = z;
    }
    FL(gear, 0x2c) = (float)((double)S32(msg, 0x2e) * (double)SCALE16);
    FL(gear, 0x28) = (float)((double)S32(msg, 0x2e) * (double)SCALE16);
    FL(gear, 0x7c) = (float)((double)S32(msg, 0x32) * (double)SCALE16);
    FL(gear, 0x78) = (float)((double)S32(msg, 0x32) * (double)SCALE16);
    FL(gear, 0x38) = (float)((double)S32(msg, 0x36) * (double)SCALE16);
    FL(gear, 8) = (float)((double)S32(msg, 0x3a) * (double)SCALE16);
    FL(gear, 0x18) = (float)((double)S32(msg, 0x3e) * (double)SCALE16);
    view = U32(U32(gear, 0), 0x54);
    U32(view, 0) = U32(gear, 0x18);
    U32(gear, 0x4f0) = U16(msg, 7) & 0xf;
    if (U32(gear, 0xc19) == 0 && (msg[9] & 3) != 0) {
        U32(gear, 0xc15) = msg[9] & 3;
        U32(gear, 0xc1d) = 0;
        U32(gear, 0xc19) = 1;
    }
    if ((msg[9] & 4) != 0 && U32(gear, 0xc1d) == 0) {
        U8(view, 0x55) = 1;
        U32(gear, 0xc1d) = 1;
        U32(gear, 0xc19) = 0;
    }
    if ((U16(msg, 7) & 0x800) == 0)
        U16(gear, 0x560) &= 0xff7f;
    else
        U16(gear, 0x560) |= 0x80;
    keep = U16(gear, 0x562) & 0xfff0;
    U16(gear, 0x562) = (uint16_t)(U16(msg, 7) >> 0xc);
    U16(gear, 0x562) |= keep;
    if ((U16(msg, 7) & 0x100) == 0) {
        U32(gear, 0xc31) = 0;
        U8(view, 0x1e) = 0;
    } else if (U32(gear, 0xc31) == 0) {
        U8(view, 0x1e) = 1;
        U32(gear, 0xc31) = 1;
    } else {
        U8(view, 0x1e) = 0;
    }
    if ((U16(msg, 7) & 0x20) != 0 && U32(gear, 0xc21) == 0) {
        U8(view, 0x54) = 1;
        U32(gear, 0xc21) = 1;
        U32(gear, 0xc25) = 0;
    }
    if ((U16(msg, 7) & 0x40) != 0 && U32(gear, 0xc25) == 0) {
        U8(view, 0x54) = 0;
        U32(gear, 0xc21) = 0;
        U32(gear, 0xc25) = 1;
    }
    if ((msg[9] & 8) != 0)
        U8(view, 0x57) = 1;
    if ((msg[9] & 0x10) != 0)
        U8(view, 0x58) = 1;
    if ((msg[9] & 0x20) != 0) {
        U8(view, 0x58) = 0;
        U8(view, 0x57) = 0;
    }
    U8(view, 0x59) = (U16(msg, 7) & 0x10) != 0;
    if (U32(gear, 0x4f0) == 4 || U32(gear, 0x4f0) == 5) {
        killer = S16(unit, 0x150);
        if (g_net10Mark[U32(unit, 4)] != 1) {
            /* (both test (flags | 0x80000000) != 0, always true) */
            Sim_1006abf6((uint32_t)killer, BYTES(gear), 2);
            Score_RecordKill(killer, BYTES(gear));
#ifdef NETDA_MUTATE8 /* the checker's negative check only: 1.0's 8 players */
            if (killer < 0 || killer > 7 || killer >= GS32(0x1015dc64))
#else
            if (killer < 0 || killer > 9 || killer >= GS32(0x1015dc64))
#endif
                Crt_SPrintf(text, SIM_PTR(const char, 0x1013ad3c), (const char *)(uintptr_t)(unit + 0x124),
                            (const char *)(uintptr_t)G32(0x101c4a04));
            else if (killer == S32(unit, 4))
                Crt_SPrintf(text, SIM_PTR(const char, 0x1013ad24), (const char *)(uintptr_t)(unit + 0x124),
                            (const char *)(uintptr_t)G32(0x101c4a04));
            else
                Crt_SPrintf(text, SIM_PTR(const char, 0x1013ad2c), (const char *)(uintptr_t)(unit + 0x124),
                            (const char *)(uintptr_t)G32(0x101c49f0), (const char *)(uintptr_t)(UNIT_AT(killer) + 0x124));
            Sim_1007df6c(text, (const char *)(uintptr_t)G32(0x109b98c0), 0x712, 0x50);
            g_net10Mark[U32(unit, 4)] = 1;
        }
    } else if (g_net10Mark[U32(unit, 4)] == 1) {
        uint32_t back;

        Sim_1009d200((int32_t)U32(unit, 4), 0, 0, 0);
        back = UNIT_AT(U32(unit, 4));
        if (back != 0) {
            Sim_100aa39f(BYTES(U32(back, 0x48)));
            Sim_100aa454(BYTES(U32(back, 0x48)));
            Sim_100aa799(BYTES(U32(back, 0x48)));
            U16(back, 0x1c) &= 0xb7ff;
            Crt_SPrintf(text, SIM_PTR(const char, 0x1013ad44), (const char *)(uintptr_t)(unit + 0x124),
                        (const char *)(uintptr_t)G32(0x101c4a14));
            Sim_1007df6c(text, (const char *)(uintptr_t)G32(0x109b98c0), 0x712, 0x50);
            g_net10Mark[U32(unit, 4)] = 0;
        }
    }
}
