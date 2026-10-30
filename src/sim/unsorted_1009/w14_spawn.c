#include "w14.h"
#include "w9c.h"
#include "../simmem.h"
#include "../util/access.h"
#include "../util/util.h"
#include "../util/w3o.h"
#include "../view/w3r.h"
#include "../unsorted_1002/w3o.h"
#include "../unsorted_1002/w3q.h"
#include "../unsorted_1005/w14.h"
#include "../unsorted_1005/w8v.h"
#include "../unsorted_1005/w9b.h"
#include "../unsorted_1006/w10.h"
#include "../unsorted_1007/w2i.h"
#include "../unsorted_1007/w2j.h"
#include "../unsorted_100a/w3t.h"
#include "../unsorted_100a/w5b.h"
#include "../unsorted_100a/w5y.h"
#include "../unsorted_100a/w5z.h"
#include "../unsorted_100b/w4w.h"

#define W14S_UNIT(rec) BYTES(U32(rec, 0))
#define W14S_NODE(rec) ((void *)(uintptr_t)U32(W14S_UNIT(rec), 0x48))
#define W14S_FAR 0x7f7fc99eu /* 3.4e38 */

/* The ground offset of a foot node: its height, or 3.4e38 without the node (or with the foot's damage bit). */
static uint32_t W14S_Foot(uint8_t *rec, uint32_t off, uint16_t bit)
{
    uint32_t x, y, z;

    if (U32(rec, off) == 0 || (U16(rec, 0x562) & bit) != 0)
        return W14S_FAR;
    Sim_100a9cfb((void *)(uintptr_t)U32(rec, off), &x, &y, &z);
    return y;
}

// FUNCTION: HGSIM 0x1009d200
int __cdecl Sim_1009d200(int32_t index, int32_t force, uint32_t unused3, uint32_t unused4)
{
    static const uint8_t cleared[] = {0x1e, 0x1f, 0x20, 0x21, 0x22, 0x26, 0x27, 0x2f, 0x30, 0x59,
                                      0x31, 0x46, 0x56, 0x5b, 0x55, 0x59};
    uint8_t *rec = 0, *zone;
    uint16_t word;
    uint32_t design, i, low;
    int32_t pick, tries = 0, done = 0, g;
    /* Set by every pass of the start-point loop. Its first test exits only with a zone count <= 0, which cannot
     * happen: the zone loader 0x100daea4 makes the count (size - 0x18) / 0xc unsigned and below 9, and a count of 0
     * divides by zero in Sim_1009c929 before (DECISIONS.md round 14). */
    float x = 0.0f, y = 0.0f, z = 0.0f, d;
    uint32_t heading = 0;
    union {
        float f;
        uint32_t u;
    } px, pz, foot0, foot1;

    (void)unused3;
    (void)unused4;
    if (index >= 0 && index < 0x19)
        rec = BYTES(U32(PTRTAB(index), 0x28));
    if (rec == 0)
        return 0;
    word = U16(PTRTAB(index), 0x1c);
    if (force == 0 && (word & 2) == 0)
        return 0;
    U32(rec, 0x2c) = 0;
    U32(rec, 0x28) = 0;
    U32(rec, 0xc) = 0;
    U32(rec, 8) = 0;
    U32(rec, 0x3c) = 0;
    U32(rec, 0x38) = 0;
    U32(rec, 0x1c) = 0;
    U32(rec, 0x18) = 0;
    U32(BYTES(U32(W14S_UNIT(rec), 0x54)), 8) = 0;
    U8(BYTES(U32(W14S_UNIT(rec), 0x54)), 0x25) = 1;
    for (i = 0; i < sizeof(cleared); i++)
        U8(BYTES(U32(W14S_UNIT(rec), 0x54)), cleared[i]) = 0;
    U32(rec, 0x520) = U32(rec, 0x588);
    GS32(0x1014c99c) = index;
    Sim_100b35b0();
    design = Sim_100599a0((const char *)rec + 0xbeb);
    U32(W14S_UNIT(rec), 0x48) =
        Sim_1009d90e((void *)(uintptr_t)G32(0x1017cf78 + (uint32_t)index * 4), 0, index);
    Sim_10072e35(rec);
    Sim_1005cd4a(rec, BYTES(design));
    Sim_100aa39f(W14S_NODE(rec));
    Sim_100aa799(W14S_NODE(rec));
    U32(W14S_UNIT(rec), 0x24) = 0xffffffffu;
    U32(W14S_UNIT(rec), 0xf0) = 0x1000;
    U32(W14S_UNIT(rec), 0x164) = 0;
    word = (uint16_t)((word & 0xbff8) | 0x10);
    U16(PTRTAB(index), 0x1c) = word;
    if (G32(0x1015ec58) != 0)
        Sim_1007caab(index);
    Sim_100500ab(W14S_UNIT(rec));
    Sim_1006da46(W14S_UNIT(rec));
    zone = SIM_PTR(uint8_t, 0x109b8420 + U32(W14S_UNIT(rec), 8) * 0x74);
    pick = Sim_1009c929(S32(zone, 0x70));
    if (pick == S32(W14S_UNIT(rec), 0x280))
        pick++;
    while (done == 0 && tries < S32(zone, 0x70)) {
        tries++;
        x = FL(zone, 0x10 + pick * 4);
        z = FL(zone, 0x30 + pick * 4);
        heading = U32(zone, 0x50 + pick * 4);
        Mat3x4_TransformPoint(SIM_PTR(const float, 0x109b7d7c + U32(W14S_UNIT(rec), 8) * 0x6c), &x, &y, &z);
        done = 1;
        for (g = 0; g < GS32(0x1015dc64); g++) {
            uint8_t *other = BYTES(PTRTAB(g));

            if (!(FL(other, 0x58) < x) && !(FL(other, 0x58) > x) &&
                !(FL(other, 0x5c) < FL(W14S_UNIT(rec), 0x5c)) && !(FL(other, 0x5c) > FL(W14S_UNIT(rec), 0x5c)) &&
                !(FL(other, 0x60) < z) && !(FL(other, 0x60) > z)) {
                done = 0;
                tries++;
                pick++;
                if (pick == S32(zone, 0x70))
                    pick = 0;
                break;
            }
        }
    }
    S32(W14S_UNIT(rec), 0x280) = pick;
    px.f = x;
    pz.f = z;
    Sim_100a9d47(W14S_NODE(rec), px.u, U32(W14S_UNIT(rec), 0x5c), pz.u);
    Sim_100aa0da(W14S_NODE(rec), 0, heading, 0, 0);
    Sim_100aa799(W14S_NODE(rec));
    FL(W14S_UNIT(rec), 0x58) = x;
    FL(W14S_UNIT(rec), 0x60) = z;
    U32(W14S_UNIT(rec), 0x68) = heading;
    U16(rec, 0x562) = 0;
    if (index == GS32(0x1017b8c8)) {
        Sim_1002c113();
        G32(0x10179cd4) = 0;
        G32(0x10179d04) = 0;
        G32(0x10179ca0) = 0;
        G32(0x10179ca4) = 0;
        G32(0x10179ce0) = 0;
    }
    foot0.u = W14S_Foot(rec, 0x4c0, 0x2000);
    foot1.u = W14S_Foot(rec, 0x4c4, 0x4000);
    if (!(foot0.f >= foot1.f))
        foot1.u = foot0.u;
    low = foot1.u == W14S_FAR ? 0 : foot1.u;
    FL(W14S_UNIT(rec), 0x7c) =
        (float)Sim_1002d961(FL(W14S_UNIT(rec), 0x58), FL(W14S_UNIT(rec), 0x5c), FL(W14S_UNIT(rec), 0x60));
    foot1.u = low;
    d = (float)((double)FL(W14S_UNIT(rec), 0x7c) - (double)foot1.f);
    Sim_100a9d7c(W14S_NODE(rec), 0.0f, d, 0.0f);
    Sim_100aa799(W14S_NODE(rec));
    Sim_100a9cfb(W14S_NODE(rec), (uint32_t *)(void *)(W14S_UNIT(rec) + 0x58),
                 (uint32_t *)(void *)(W14S_UNIT(rec) + 0x5c), (uint32_t *)(void *)(W14S_UNIT(rec) + 0x60));
    GS32(0x1014c99c) = -1;
    *GBYTES(0x1015d1b0 + U32(W14S_UNIT(rec), 4)) = 0;
    return 1;
}
