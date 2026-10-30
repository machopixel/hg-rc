#include "w26.h"
#include "../simmem.h"
#include "../util/access.h"
#include "../util/util.h"
#include "../util/w3o.h"
#include "w10.h"
#include "w5a.h"
#include "w7l.h"
#include "w8t.h"
#include "../unsorted_1001/w6d.h"
#include "../unsorted_1001/w8t.h"
#include "../unsorted_1004/fns.h"
#include "../unsorted_1006/w10.h"
#include "../unsorted_1008/w8w.h"
#include "../unsorted_100d/w5c.h"

typedef double(__cdecl *W26bFmodFn)(double x, double y);

#define W26B_SHOT(i) (SIM_PTR(uint8_t, 0x109a3f80) + (uint32_t)(i) * 0x156u)
#define W26B_WEAPON(kind) (SIM_PTR(uint8_t, 0x10145508) + (uint32_t)(kind) * 0xd6u)

/* The muzzle segment of Sim_100d6f16 / Sim_1002dda9 (0x38 bytes, as in the original frame): +0xc.. the end point,
   +0x30 the hit's distance. */
typedef struct {
    uint32_t w[14];
} W26Segment;

/* a hit counter of the statistics block by the hit unit's side (0, 1, 2) */
static void W26b_Count(int side, uint32_t a0, uint32_t a1, uint32_t a2)
{
    if (side == 0)
        U16(SIM_PTR(uint8_t, a0), 0)++;
    else if (side == 1)
        U16(SIM_PTR(uint8_t, a1), 0)++;
    else if (side == 2)
        U16(SIM_PTR(uint8_t, a2), 0)++;
}

// FUNCTION: HGSIM 0x10021e8c
void __cdecl Sim_10021e8c(int idx)
{
    uint8_t *shot = W26B_SHOT(idx);
    W26Segment seg;
    uint32_t node[3], hit = 0, flags = 0, target, rec, unit, x, y, z;
    float dx, dy, dz, angle;
    int found, i;

    if (U32(shot, 0x48) == 0)
        return;
    if (S32(shot, 0x34) <= 0) {
        U32(shot, 0x48) = 0;
        return;
    }
    if (S32(W26B_WEAPON(U32(shot, 0x8e)), 0x10) == 6 || S32(W26B_WEAPON(U32(shot, 0x8e)), 0x10) == 4)
        U32(shot, 0x48) = 0;
    unit = PTRTAB(U32(shot, 0x50));
    Sim_100a9cfb((void *)(uintptr_t)U32(shot, 0x3c), &node[0], &node[1], &node[2]);
    Sim_100d6f16(BYTES(unit), &seg);
    found = Sim_1002dda9(&seg, &hit, (int)U32(shot, 0x50));
    x = seg.w[3];
    U32(shot, 0x18) = x;
    y = seg.w[4];
    U32(shot, 0x1c) = y;
    z = seg.w[5];
    U32(shot, 0x20) = z;
    dx = (float)(((double)Float_OfBits(x) - (double)Float_OfBits(node[0])) / (double)GFL(0x10111220));
    dy = (float)(((double)Float_OfBits(y) - (double)Float_OfBits(node[1])) / (double)GFL(0x10111220));
    dz = (float)(((double)Float_OfBits(z) - (double)Float_OfBits(node[2])) / (double)GFL(0x10111220));
    for (i = 0; i < 0x10; i++) {
        FL(shot, 0x92 + i * 0xc) = (float)((double)i * (double)dx + (double)Float_OfBits(node[0]));
        FL(shot, 0x96 + i * 0xc) = (float)((double)i * (double)dy + (double)Float_OfBits(node[1]));
        FL(shot, 0x9a + i * 0xc) = (float)((double)i * (double)dz + (double)Float_OfBits(node[2]));
    }
    if (found != 0 && hit != 0) {
        /* beyond the weapon's range the original returns here, without the effect */
        if ((double)Float_OfBits(seg.w[12]) > (double)FL(W26B_WEAPON(U32(shot, 0x8e)), 0x50))
            return;
        flags = U16(hit, 2);
        G32(0x10134898) = U32(shot, 0x50);
        if ((flags & 0x100) != 0) {
            target = U16(hit, 0x24);
            if ((U32(W26B_WEAPON(U32(shot, 0x8e)), 0x86) & 8) != 0 && Sim_1009c929(100) < 10) {
                rec = U32(PTRTAB(target), 0x28);
                if (S32(rec, 0x4f0) == 2 && U32(rec, 0xbbf) == 0 && (U16(rec, 0x560) & 0x20) == 0) {
                    if (target == G32(0x1017b8c8))
                        Sim_100203cf(0x2a, -1);
                    else
                        Sim_10015630(BYTES(U32(rec, 0)), 0xa, 0, 1);
                    U32(rec, 0xc3d) = 0x21f;
                }
            }
            U32(PTRTAB(target), 0x150) &= 0x7fffffffu;
            U32(PTRTAB(target), 0x150) = (uint32_t)(int32_t)*SIM_PTR(int16_t, 0x10134898) & 0xffffu;
            U32(shot, 8) |= 0x100;
            if ((U32(shot, 0x7e) & 0x40) != 0)
                U32(shot, 8) |= 0x1000;
            /* a facing angle the original computes and never uses */
            angle = (float)((double)FL(PTRTAB(target), 0x68) - Sim_1008486e(FL(shot, 0x18), FL(shot, 0x20)));
            (void)SIM_FN(W26bFmodFn, 0x100fcb30)((double)angle, 360.0);
            if (G32(0x10134898) == G32(0x1017b8c8))
                W26b_Count(Sim_10052be7((int)target), 0x101653f9, 0x101653f5, 0x101653f7);
            if (GS32(0x10134898) >= 0 && U32(PTRTAB(G32(0x10134898)), 8) == U32(PTRTAB(G32(0x1017b8c8)), 8))
                W26b_Count(Sim_10052be7((int)target), 0x1016540e, 0x10165412, 0x10165410);
            if (target == G32(0x1017b8c8))
                U16(SIM_PTR(uint8_t, 0x101653fb), 0)++;
            if (U32(PTRTAB(target), 8) == U32(PTRTAB(G32(0x1017b8c8)), 8))
                U16(SIM_PTR(uint8_t, 0x10165414), 0)++;
            if (G32(0x1015ecd0) == 0 ||
                (U32(PTRTAB(target), 0x14) == 3 && U32(PTRTAB(target), 0xc) == G32(0x1017b8c8)) ||
                (target == G32(0x1017b8c8) && G32(0x10134898) != G32(0x1017b8c8))) {
                rec = U32(PTRTAB(target), 0x28);
                FL(rec, 0x4e0) = (float)((double)FL(rec, 0x4e0) + (double)FL(shot, 0x5c));
                Sim_1006c161((int32_t)U32(shot, 0x50), BYTES(rec), 1, shot, (int32_t)U16(hit, 0x26));
            }
            if (target == G32(0x1017b8c8))
                U32(shot, 8) |= 0x4000;
            Sim_10015afd((int)U32(shot, 0x50), target);
        } else if ((flags & 0x200) != 0) {
            U32(shot, 8) |= 0x200;
            Sim_10025aee(U32(shot, 0x50), (const void *)(uintptr_t)hit, (float)(double)U32(shot, 0x70), x, y, z);
        } else if ((flags & 0x400) != 0) {
            U32(shot, 8) |= 0x200;
        } else {
            U32(shot, 8) |= 0x400;
        }
    }
    if (found != 0 && (flags & 0x100) != 0) {
        /* deliberate fix (DECISIONS.md round 26): the camera point is the hit point; the original passes [ebp-0x8c],
           [ebp-0x94], [ebp-8], never written */
#ifdef W26B_MUTATE /* the checker's negative check only */
        Sim_100246b3(U32(shot, 0x50), (uint8_t *)(uintptr_t)hit, (int32_t)U32(shot, 8), Float_OfBits(x),
                     Float_OfBits(y), Float_OfBits(z), y, x, z, 0, 0, 0);
#else
        Sim_100246b3(U32(shot, 0x50), (uint8_t *)(uintptr_t)hit, (int32_t)U32(shot, 8), Float_OfBits(x),
                     Float_OfBits(y), Float_OfBits(z), x, y, z, 0, 0, 0);
#endif
    }
}
