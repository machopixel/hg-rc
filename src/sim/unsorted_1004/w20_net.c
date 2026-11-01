#include "w20.h"
#include "fns.h"
#include "w3p.h"
#include "w35.h"
#include "../util/access.h"
#include "../unsorted_1001/w8t.h"
#include "../unsorted_1003/fns.h"
#include "../unsorted_1003/w6e.h"
#include "../unsorted_1007/w2e.h"
#include "../unsorted_1007/w3p.h"
#include "../unsorted_1009/w14.h"
#include "../unsorted_100a/w5b.h"
#include "../unsorted_100a/w5x.h"
#include "../unsorted_100a/w5y.h"
#include "../unsorted_100f/w2e.h"
#include "../unsorted_100f/w8z.h"

#ifdef NETRG_MUTATE /* the checker's negative check only: 1.0's "RG" below */
void __cdecl Sim_10040af8(uint8_t *msg, int who)
{
    uint8_t id = U8(msg, 2);
    uint32_t gear, own = G32(0x1017b8c8);

    if (G32(0x1015ecd0) == 1) {
        int32_t index;

        if (id == own)
            return;
        gear = PTRTAB(id);
        if (U32(gear, 0x14) == 3) {
            if (S32(gear, 0xc) > 0 && PTRTAB(U32(gear, 0xc)) != 0) {
                id = U8(gear, 0xc);
            } else {
                U32(gear, 0x164) = 0;
                Sim_10040564((int)U32(gear, 4));
                return;
            }
        }
        if (U8(U32(gear, 0x168), 0) != 0) {
            U8(msg, 2) = (uint8_t)(U8(msg, 2) + 8);
            U8(U32(gear, 0x168), 0) = (uint8_t)(U8(U32(gear, 0x168), 0) - 1);
            U32(gear, 0x164) = 1;
        }
        Mem_Move((void *)(uintptr_t)G32(0x1015eca8), msg, 3);
        index = Sim_1003dbc0(id);
        Net_DpSend_1007e950(((uint32_t)index & 0xffff0000u) | U16(U32(G32(0x1015d004), 0x16), (uint32_t)index * 2u),
                            (void *)(uintptr_t)G32(0x1015eca8), 3, 1);
        return;
    }
    if (id == own || id == own + 8) {
        if (G32(0x1015f200) == 0)
            return;
        if (id >= 8) {
            G32(0x1014cb1c) = 0;
            Sim_1003cbb3(0, 0);
            Sim_1009d200((int32_t)own, 0, 0, 0);
            Sim_100448cd(0);
            U8(GBYTES(0x1015fdba), 0) = 1;
            U8(GBYTES(0x1015fdbd), 0) = 1;
            U8(GBYTES(0x1015fdd0), 0) = 1;
            G32(0x1015d530) = 0;
            G32(0x1015f1d4) = 0;
            G32(0x1015f208) = 0;
            G32(0x1015f1d0) = 0;
            G32(0x1015f1e4) = 0;
            G32(0x1015f200) = 0;
            /* (the original clears +0x164 of a never-set gear pointer here: see w20.h) */
            G32(0x1015f1fc) = 0;
            G32(0x1015ef10) = 0;
        } else {
            U8(G32(0x1017b8c4), 8) = 0;
            /* (the original clears +0x164 of a never-set gear pointer here: see w20.h) */
            G32(0x1015f200) = 0;
            G32(0x1015f1d0) = 0xffffffffu;
            Sim_1007df6c(SIM_PTR(const char, 0x1013ad7c), (const char *)(uintptr_t)G32(0x109b98c0), 0x58610, 0x64);
            G32(0x1015f208) = 1;
            G32(0x1015ef10) = 1;
            gear = PTRTAB(G32(0x1017b8c8));
            if (U8(G32(0x1017b8c4), 0xa) != 0)
                U8(U32(gear, 0x168), 0) = 0;
        }
        return;
    }
    if (id < 8) {
        gear = PTRTAB(id);
        U8(U32(gear, 0x168), 0) = 0;
        U32(gear, 0x164) = 0;
        return;
    }
    gear = PTRTAB(id - 8);
    if (U32(gear, 0x14) != 3)
        return;
    {
        char text[0x50];

        Sim_1009d200(S32(gear, 4), 0, 0, 0);
        Sim_10011093((void *)(uintptr_t)gear);
        Sim_100aa39f((void *)(uintptr_t)U32(gear, 0x48));
        Sim_100aa454((void *)(uintptr_t)U32(gear, 0x48));
        Sim_100aa799((void *)(uintptr_t)U32(gear, 0x48));
        U16(gear, 0x1c) = (uint16_t)(U16(gear, 0x1c) & 0xb7ffu);
        Crt_SPrintf(text, SIM_PTR(const char, 0x1013adcc), (const char *)(uintptr_t)(gear + 0x124));
        Sim_1007df6c(text, (const char *)(uintptr_t)G32(0x109b98c0), 0x712, 0x50);
        g_net10Mark[U32(gear, 4)] = 0;
        Sim_10040564((int)U32(gear, 4));
    }
}
#else
/* 1.2 (0x1003474e, rewritten): "RG", a player asks the host for a regeneration (unit < 10) or a revival (unit + 10).
   The host answers the sender (1.0: the unit's player), adding 10 when the unit has lives left, which it takes; no
   longer a state-3 unit's controller. The asker, for the host's answer about itself: out of lives (the "Out of
   regenerations" text) or revived; about another: a state-3 unit loses its lives, or comes back ("resurrected").
   1.2 checks the unit and its owner where 1.0 did not, clears the right unit's +0x164 (1.0 a never-set pointer's),
   and calls Sim12_10089c60 before the own respawn. */
// FUNCTION: HGSIM 0x10040af8
// FUNCTION: HGSIM12 0x1003474e
void __cdecl Sim_10040af8(uint8_t *msg, int who)
{
    uint32_t id = U8(msg, 2);
    uint32_t own = G32(0x1017b8c8);
    uint32_t unit;

#ifndef NETG_MUTATE /* the checker's negative check only: no guard */
    if (g_w34NetOver != 0)
        return;
#endif
    if (G32(0x1015ecd0) == 1) {
        int32_t index;

        if (id == own)
            return;
        unit = PTRTAB(id);
        if (U8(U32(unit, 0x168), 0) != 0) {
            U8(msg, 2) = (uint8_t)(U8(msg, 2) + 10);
            U8(U32(unit, 0x168), 0) = (uint8_t)(U8(U32(unit, 0x168), 0) - 1);
            U32(unit, 0x164) = 1;
        }
        Mem_Move((void *)(uintptr_t)G32(0x1015eca8), msg, 3);
        index = Sim_1003dbc0((uint32_t)who);
        Net_DpSend_1007e950(((uint32_t)index & 0xffff0000u) | U16(U32(G32(0x1015d004), 0x16), (uint32_t)index * 2u),
                            (void *)(uintptr_t)G32(0x1015eca8), 3, 1);
        return;
    }
    if (who != (int)Sim_1003db56((int16_t)*SIM_PTR(u16a, 0x1015ecd8)))
        return;
    if (id == own || id == own + 10) {
        if (G32(0x1015f200) == 0)
            return;
        if (id < 10) {
            unit = PTRTAB(id);
            if (unit == 0 || U32(unit, 4) != own)
                return;
            U32(unit, 0x164) = 0;
            G32(0x1015f200) = 0;
            G32(0x1015f1d0) = 0xffffffffu;
            Sim_1007df6c(SIM_PTR(const char, 0x1013ad7c), (const char *)(uintptr_t)G32(0x109b98c0), 0x58610, 100);
            G32(0x1015f208) = 1;
            G32(0x1015ef10) = 1;
            U8(U32(unit, 0x168), 0) = 0;
        } else {
            unit = PTRTAB(id - 10);
            if (unit == 0 || U32(unit, 4) != own)
                return;
            G32(0x1014cb1c) = 0;
            Sim_1003cbb3(0, 0);
            Sim12_10089c60();
            Sim_1009d200((int32_t)own, 0, 0, 0);
            Sim_100448cd(0);
            U8(GBYTES(0x1015fdba), 0) = 1;
            U8(GBYTES(0x1015fdbd), 0) = 1;
            U8(GBYTES(0x1015fdd0), 0) = 1;
            G32(0x1015d530) = 0;
            G32(0x1015f1d4) = 0;
            G32(0x1015f208) = 0;
            G32(0x1015f1d0) = 0;
            G32(0x1015f1e4) = 0;
            G32(0x1015f200) = 0;
            U32(unit, 0x164) = 0;
            G32(0x1015f1fc) = 0;
            G32(0x1015ef10) = 0;
        }
        return;
    }
    if (id < 10) {
        unit = PTRTAB(id);
        if (unit != 0 && U32(unit, 0x14) == 3) {
            U8(U32(unit, 0x168), 0) = 0;
            U32(unit, 0x164) = 0;
        }
        return;
    }
    unit = PTRTAB(id - 10);
    if (unit == 0 || U32(unit, 0x14) != 3 || U32(unit, 0x164) == 0)
        return;
    {
        char text[0x50];

        Sim_1009d200(S32(unit, 4), 0, 0, 0);
        Sim_100aa39f((void *)(uintptr_t)U32(unit, 0x48));
        Sim_100aa454((void *)(uintptr_t)U32(unit, 0x48));
        Sim_100aa799((void *)(uintptr_t)U32(unit, 0x48));
        U16(unit, 0x1c) = (uint16_t)(U16(unit, 0x1c) & 0xb7ffu);
        Crt_SPrintf(text, SIM_PTR(const char, 0x1013adcc), (const char *)(uintptr_t)(unit + 0x124));
        Sim_1007df6c(text, (const char *)(uintptr_t)G32(0x109b98c0), 0x712, 0x50);
        g_net10Mark[U32(unit, 4)] = 0;
        Sim_10040564((int)U32(unit, 4));
    }
}
#endif
