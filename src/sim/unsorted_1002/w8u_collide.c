#include "w8u.h"
#include "w3q.h"
#include "w7l.h"
#include "../unsorted_1000/w3o.h"
#include "../unsorted_1009/w2e.h"
#include "../unsorted_1009/w3o.h"
#include "../util/w3o.h"

/*
 * The sphere test of one node: when its word +0 lacks bit 0x800 and Sim_10009eb3 gives a distance below *dist
 * (or a NaN), Sim_1002e569 checks the node's faces on a copy of the segment; a hit nearer than *dist (by
 * Sim_1009cbde of the cut segment) becomes the new best: *dist, the segment, and the three dwords 0x1015dd40..48.
 */
static void W8u_TestNode(uint8_t *node, uint8_t *seg, uint8_t *work, uint32_t *dist, uint32_t *hit, uint32_t *a,
                         uint32_t *b, uint32_t *c)
{
    float d, len;

    if ((U16(node, 0) & 0x800) != 0)
        return;
    d = Sim_10009eb3(node, seg);
    if (!((double)d >= (double)Float_OfBits(*dist))) {
        Sim_1009cfbd(work, seg);
        if (Sim_1002e569(node, work, d) != 0) {
            len = Sim_1009cbde(work);
            if (!((double)len >= (double)Float_OfBits(*dist))) {
                *dist = Bits_OfFloat(len);
                Sim_1009cfbd(seg, work);
                *hit = (uint32_t)(uintptr_t)node;
                *a = G32(0x1015dd40);
                *b = G32(0x1015dd44);
                *c = G32(0x1015dd48);
            }
        }
    }
}

// FUNCTION: HGSIM 0x1002dfbb
uint32_t __cdecl Sim_1002dfbb(void *node, const void *seg, void *outSeg, uint32_t *dist, uint32_t *outA,
                              uint32_t *outB, uint32_t *outC)
{
    uint8_t best[0x38], work[0x38], sub[0x38];
    uint32_t hit = 0, a = 0x41200000, b = 0, c = 0;
    uint8_t *list;
    uint8_t *child;

    if (node == 0 || seg == 0 || dist == 0 || outA == 0 || outB == 0 || outC == 0)
        return 0;
    list = BYTES(U32(node, 0x28));
    Sim_1009cfbd(best, seg);
    if (list == 0 || U32(list, 4) == 0) {
        W8u_TestNode(node, best, work, dist, &hit, &a, &b, &c);
    } else {
        float d;

        Sim_1009cfbd(work, seg);
        d = Sim_10009d7e(node, best);
        if (!((double)d >= (double)Float_OfBits(*dist))) {
            W8u_TestNode(node, best, work, dist, &hit, &a, &b, &c);
            for (child = BYTES(U32(list, 4)); child != 0; child = BYTES(U32(child, 8))) {
                uint32_t sd, sa, sb, sc, r;

                Sim_1009cfbd(work, best);
                sd = *dist;
                r = Sim_1002dfbb((void *)(uintptr_t)U32(child, 0x8c), work, sub, &sd, &sa, &sb, &sc);
                if (r != 0 && !((double)Float_OfBits(sd) >= (double)Float_OfBits(*dist))) {
                    *dist = sd;
                    Sim_1009cfbd(best, sub);
                    hit = r;
                    a = sa;
                    b = sb;
                    c = sc;
                }
            }
        }
    }
    if (hit != 0) {
        Sim_1009cfbd(outSeg, best);
        *outA = a;
        *outB = b;
        *outC = c;
    }
    return hit;
}

// FUNCTION: HGSIM 0x1002e91e
int __cdecl Sim_1002e91e(void *face, void *points, uint32_t x, uint32_t y, uint32_t z)
{
    float nx, ny, nz;

    if (U16(face, 2) < 3)
        return 0;
    nx = (float)W8U_FABS_1002((double)FL(face, 0x18));
    ny = (float)W8U_FABS_1002((double)FL(face, 0x1c));
    nz = (float)W8U_FABS_1002((double)FL(face, 0x20));
    if ((double)ny > (double)nx && (double)ny > (double)nz)
        return Sim_1002e9f5(face, points, Float_OfBits(x), Float_OfBits(z));
    if ((double)nx > (double)nz)
        return W8U_CALL_1002f021(face, points, y, z);
    return Sim_1002ed0b(face, points, Float_OfBits(x), Float_OfBits(y));
}
