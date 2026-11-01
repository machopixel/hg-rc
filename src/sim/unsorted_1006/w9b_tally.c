#include "w9b.h"
#include "../unsorted_1000/w2e.h"
#include "../unsorted_1001/w2l.h"
#include "../unsorted_1001/w3s.h"
#include "../unsorted_1001/w7l.h"
#include "../unsorted_1002/w5a.h"
#include "../unsorted_1002/w8t.h"
#include "../unsorted_1003/w8u.h"
#include "../unsorted_1004/fns.h"
#include "../unsorted_1004/w3p.h"
#include "../unsorted_1005/w5b.h"
#include "../unsorted_1007/w2e.h"
#include "../unsorted_1007/w2f.h"
#include "../unsorted_100a/w5x.h"
#include "../unsorted_100f/w2e.h"
#include "../unsorted_100f/w8z.h"
#include "../util/access.h"
#include "../util/util.h"

#define SELF G32(0x1017b8c8)
#define TEXT(addr) SIM_PTR(const char, addr)
#define TEXT_AT(addr) ((const char *)(uintptr_t)G32(addr))
#define GW16(addr) (*SIM_PTR(u16a, addr))

// FUNCTION: HGSIM 0x100654a1
void __cdecl FlashText_Start(uint32_t which)
{
    const char *src;

    if (G32(0x10179ce0) == 0)
        return;
    G32(0x101658c0) = 0x389;
    G32(0x101658c4) = 0;
    G32(0x101658c8) = 0x5a;
    G32(0x101658cc) = (uint32_t)(GS32(0x101658c0) / GS32(0x101658c8));
    G32(0x101658d0) = which;
    G32(0x101658d4) = 1;
    switch (which) {
    case 0:    src = TEXT_AT(0x101c4eb0); break;
    case 1:    src = TEXT_AT(0x101c4e7c); break;
    case 2:    src = TEXT_AT(0x101c4e94); break;
    case 3:    src = TEXT_AT(0x101c4e58); break;
    case 4:    src = TEXT_AT(0x101c4e64); break;
    case 0xa:  src = TEXT_AT(0x101c4e40); break;
    case 0x14: src = TEXT_AT(0x101c4e48); break;
    case 0x15: src = TEXT_AT(0x101c4560); break;
    default:   src = TEXT(0x10176958); break;
    }
    Str_Copy(SIM_PTR(char, 0x101658d8), src);
}

/* The entry of the block, read again at every use like the original does. */
#define ENTRY BYTES(U32(block, 0))

static void W9b_Inc16(uint32_t addr)
{
    GW16(addr) = (uint16_t)(GW16(addr) + 1);
}

/*
 * The team tally of a kill of unit class `cls` by team slot `slot` (0 or 1); `full` (the unit is of class 1) also
 * counts it in the totals.
 */
static void W9b_TallyTeam(int team, int slot, uint32_t cls, int full)
{
    switch (team) {
    case 0:
        if (full)
            W9b_Inc16(0x10165404);
        W9b_Inc16(0x101654c0 + (uint32_t)slot * 0x16 + cls * 2);
        break;
    case 1:
        if (full) {
            W9b_Inc16(0x10165400);
            W9b_Inc16(0x10165526 + (uint32_t)slot * 2);
        }
        W9b_Inc16(0x10165468 + (uint32_t)slot * 0x16 + cls * 2);
        break;
    case 2:
        if (full)
            W9b_Inc16(0x10165402);
        W9b_Inc16(0x10165494 + (uint32_t)slot * 0x16 + cls * 2);
        break;
    default:
        break;
    }
}

/* one more kill of `victim` by `killer` in 1.2's kill table (rows of 0x14, round 33) */
static void W9b_Kill(int killer, uint32_t victim)
{
    u16a *w = (u16a *)(g_w34Kills + (uint32_t)killer * 0x14u + victim * 2u);

    *w = (uint16_t)(*w + 1);
}

// FUNCTION: HGSIM 0x1006a12d
// FUNCTION: HGSIM12 0x10059e9d
void __cdecl Score_RecordKill(int killer, uint8_t *block)
{
    int slot = -1;
    int is_self = (U32(ENTRY, 4) == SELF);
    int team1, team2;
    int i;
    int team;
    char text[0x50];

    if (is_self && U8(G32(0x1017b8c4), 1) != 0 && U32(block, 0x4f0) != 7)
        return;
    team1 = Sim_10015c61(1);
    team2 = Sim_10015c61(2);
    if (G32(0x1015ec58) != 0) {
        if (is_self) {
            for (i = 0; i < 0x77; i++)
                SIM_PTR(u32a, 0x109e8480)[i] =
                    (uint32_t)(uintptr_t)(GBYTES(0x101362e0) + (U32(block, 0x613) + G32(0x101c64f0) * 2) * 0x770 +
                                          (uint32_t)i * 16);
            Sim_1000ecb0(TEXT(0x101452e0));
        }
        /* (1.0 also marked the unit in 0x1015d1b0 here; 1.2 does not) */
    }
    if ((S16(ENTRY, 0x1c) & 2) != 0 || (S16(ENTRY, 0x1c) & 4) != 0)
        return;
    if (is_self)
        W9b_Inc16(0x101653fe);
    if (G32(0x1017e678) != 0 && (U16(G32(0x1017e678), 2) & 0x100) != 0 &&
        U16(G32(0x1017e678), 0x24) == U32(ENTRY, 4))
        G32(0x1017e678) = 0;
    U32(ENTRY, 0x150) |= (uint32_t)(int16_t)killer & 0xffff;
    /* 1.2: in a network game, a kind-3 unit of the local player's killed by player 0..9: counted in 1.2's kill table
       (row killer, column the unit) and sent ("AK") */
    if (G32(0x1015ec58) != 0 && U32(ENTRY, 0x14) == 3 && U32(ENTRY, 0xc) == SELF && killer >= 0 && killer < 10) {
#ifdef W34_MUTATE /* the checker's negative check only: the wrong column */
        W9b_Kill(killer, SELF);
#else
        W9b_Kill(killer, U32(ENTRY, 4));
#endif
        Sim12_100352b7((uint8_t)SELF, (int)U32(ENTRY, 4));
    }
    if (U32(ENTRY, 8) == U32(PTRTAB(SELF), 8)) {
        if (U32(ENTRY, 4) == (uint32_t)team1)
            slot = 0;
        else if (U32(ENTRY, 4) == (uint32_t)team2)
            slot = 1;
        if (U32(block, 0x4f0) == 5) {
            W9b_Inc16(0x10165418);
        } else {
            W9b_Inc16(0x10165416);
            if (slot >= 0 && slot < 2)
                W9b_Inc16(0x10165522 + (uint32_t)slot * 2);
        }
        if (U32(ENTRY, 4) == SELF && killer >= 0 && killer < 10 && G32(0x1015ec58) != 0) {
            W9b_Kill(killer, SELF);
            Sim_100413c7();
        }
    }
    if (U32(block, 0xbdf) - U32(U32(PTRTAB(SELF), 0x28), 0xbe3) >= 6)
        G32(0x10165512) = 1;
    if ((uint32_t)killer == SELF) {
        uint32_t cls = U32(block, 0xbdb);

        if (U32(ENTRY, 0) == 4)
            cls = 6;
        if (U32(block, 0x613) == 1)
            W9b_Inc16(0x101654f6 + cls * 2);
        else
            W9b_Inc16(0x10165504 + cls * 2);
    }
    Sim_1007b02d();
    if (U32(block, 0x4f0) == 5) {
        if (G32(0x1015ebbc) != 0) {
            if (U32(ENTRY, 4) == SELF)
                *GBYTES(0x101653fd) = 4;
            U32(block, 0x4f0) = 4;
        } else if (is_self) {
            *GBYTES(0x101653fd) = 2;
        }
    } else if (U32(ENTRY, 4) == SELF) {
        *GBYTES(0x101653fd) = 4;
    }
    if ((uint32_t)killer == SELF) {
        team = Sim_10052be7((int)U32(ENTRY, 4));
        if (U32(ENTRY, 0) == 1) {
            switch (team) {
            case 0:
                G32(0x10179cac) -= 1;
                W9b_Inc16(0x101653eb);
                W9b_Inc16(0x10165452 + U32(ENTRY, 0) * 2);
                break;
            case 1:
                G32(0x10179cac) += 1;
                W9b_Inc16(0x101653e7);
                W9b_Inc16(0x10165426 + U32(ENTRY, 0) * 2);
                break;
            case 2:
                G32(0x10179cac) += 1;
                W9b_Inc16(0x101653e9);
                W9b_Inc16(0x1016543c + U32(ENTRY, 0) * 2);
                break;
            default:
                break;
            }
        } else {
            switch (team) {
            case 0: W9b_Inc16(0x10165452 + U32(ENTRY, 0) * 2); break;
            case 1: W9b_Inc16(0x10165426 + U32(ENTRY, 0) * 2); break;
            case 2: W9b_Inc16(0x1016543c + U32(ENTRY, 0) * 2); break;
            default: break;
            }
        }
    }
    slot = -1;
    if (killer == team1)
        slot = 0;
    else if (killer == team2)
        slot = 1;
    if ((S16(ENTRY, 0x1c) & 0x1400) != 0 && slot >= 0 && slot < 2) {
        team = Sim_10052be7((int)U32(ENTRY, 4));
        W9b_TallyTeam(team, slot, U32(ENTRY, 0), U32(ENTRY, 0) == 1);
    }
    if (Sim_10052be7((int)U32(ENTRY, 4)) == 1) {
        switch (U32(ENTRY, 0)) {
        case 1:  Sim_100203cf(0x22, -1); break;
        case 3:  Sim_100203cf(0x23, -1); break;
        case 4:  Sim_100203cf(0x24, -1); break;
        case 5:
        case 8:  Sim_100203cf(0x25, -1); break;
        case 10: Sim_100203cf(0x26, -1); break;
        default: break;
        }
    }
    U16(ENTRY, 0x1c) = (uint16_t)(U16(ENTRY, 0x1c) | 6);
    U32(block, 0x4d8) = 0;
    if (G32(0x1015ec58) != 0 || U32(block, 0x4f0) != 5 || U32(ENTRY, 4) != SELF)
        U32(block, 0x4f0) = 4;
    Sim_1001150d(ENTRY);
    {
        uint8_t *entry = ENTRY;

        if (U32(entry, 4) == (uint32_t)Sim_10052aaf((int)U32(ENTRY, 8))) {
            int leader = Sim_10012843((int)U32(ENTRY, 8));

            if ((uint32_t)leader == SELF)
                Sim_1007df6c(TEXT_AT(0x101c42a8), TEXT_AT(0x109b98c0), 0x389, 0x50);
            Sim_100128dd((int)U32(ENTRY, 8), (int)U32(ENTRY, 4), (unsigned)leader);
            Crt_SPrintf(text, TEXT(0x101452ec), G32(0x1015dc78), U32(ENTRY, 8), leader);
            Sim_1005f9aa(text);
        }
    }
    if (G32(0x1015ec58) == 0 || (U32(ENTRY, 0x14) == 3 && U32(ENTRY, 0xc) == SELF)) {
        if (U32(ENTRY, 4) != SELF && (uint32_t)Sim_10052aaf((int)U32(ENTRY, 8)) == SELF)
            Sim_10020256(5, (int32_t)U32(ENTRY, 0x10));
    }
    Gear_SetLod(U32(ENTRY, 4), 1);
    Sim_100ac58e((int)U32(ENTRY, 0x154));
    U32(ENTRY, 0x154) = 0xffffffffu;
}
