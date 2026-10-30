#include "w8w.h"
#include "w2f.h"
#include "w7n.h"
#include "../unsorted_1000/w5a.h"
#include "../unsorted_1001/w5c.h"
#include "../unsorted_1002/w3o.h"
#include "../unsorted_1002/w6d.h"
#include "../unsorted_1003/w7m.h"
#include "../unsorted_1007/w2g.h"
#include "../unsorted_1008/w3o.h"
#include "../unsorted_100a/w2f.h"
#include "../unsorted_100a/w2l.h"
#include "../unsorted_100a/w5z.h"
#include "../unsorted_100a/w7o.h"
#include "../unsorted_100c/w5z.h"
#include "../unsorted_100c/w6h.h"
#include "../unsorted_100f/w2e.h"
#include "../util/util.h"
#include "../util/w3o.h"

// FUNCTION: HGSIM 0x1006754e
int __cdecl Sim_1006754e(uint8_t *obj, float *x, float *y, float *z, uint32_t *hit)
{
    int32_t list = 0;
    uint32_t cells, node;
    uint8_t *found;
    float dist, limit, dx, dy, dz, length;
    float bx, by, bz;
    int32_t index;
    double scale;

    Sim_100881dd(*x, *z);
    while ((cells = G32(0x1036f2a0 + 4 * (uint32_t)list)) != 0) {
        list++;
        *hit = 0;
        for (node = U32(cells, 0x18); node != 0; node = U32(node, 0x18)) {
            if (U32(node, 0x34) != 6 || (U16(node, 2) & 0x100) != 0)
                continue;
            found = Sim_10067356(BYTES(node), FL(obj, 0x53c), &dist, &limit, *x, *y, *z, &dx, &dy, &dz);
            if (found == 0 || (double)dist >= (double)limit)
                continue;
            if ((U16(found, 2) & 0xf0) == 0x50) {
                uint32_t block = U32(found, 0x28);

                if (Sim_10084a94(FL(obj, 0x548), FL(obj, 0x54c), FL(obj, 0x550)) > (double)GFL(0x1012b600)) {
                    index = Sim_10028b90(block, 1);
                    if (index >= 0) {
                        Sim_1002977e(index, (float)((double)FL(obj, 0x548) * (double)GFL(0x1012b604)),
                                     (float)((double)FL(obj, 0x54c) * (double)GFL(0x1012b604)),
                                     (float)((double)FL(obj, 0x550) * (double)GFL(0x1012b604)));
                        /* the original subtracts the camera position from this point and never reads it */
                        Sim_100a9cfb(BYTES(block), (uint32_t *)&bx, (uint32_t *)&by, (uint32_t *)&bz);
                    }
                }
                return 0;
            }
            if ((double)dist > (double)GFL(0x1012b5f8)) {
                GFL(0x1015dd58) = (float)((double)dx / (double)dist);
                GFL(0x1015dd5c) = (float)((double)dy / (double)dist);
                GFL(0x1015dd60) = (float)((double)dz / (double)dist);
            } else {
                G32(0x1015dd5c) = 0;
                G32(0x1015dd58) = G32(0x1015dd5c);
                G32(0x1015dd60) = 0x3f800000u;
            }
            *x = (float)((double)GFL(0x1015dd58) * (double)limit + (double)FL(found, 0x44));
            *y = (float)((double)GFL(0x1015dd5c) * (double)limit + (double)FL(found, 0x48));
            *z = (float)((double)GFL(0x1015dd60) * (double)limit + (double)FL(found, 0x4c));
            scale = Sim_10084a94(FL(obj, 0x554), FL(obj, 0x558), FL(obj, 0x55c)) / (double)GFL(0x1012b608);
            length = (float)scale;
            if (scale > (double)GFL(0x1012b60c))
                length = (float)((double)length / (double)GFL(0x1012b610));
            else
                length = (float)((double)length / (double)GFL(0x1012b604));
            FL(obj, 0x548) = (float)((double)GFL(0x1015dd58) * (double)length);
            FL(obj, 0x54c) = (float)((double)GFL(0x1015dd5c) * (double)length);
            FL(obj, 0x550) = (float)((double)GFL(0x1015dd60) * (double)length);
            *hit = (uint32_t)(uintptr_t)found;
            return 1;
        }
    }
    return 0;
}

/* The 0x54-byte entries at 0x109a8650: float +0, word flags +0x14, word bits +0x16. */
#define ENTRY(k) (0x109a8650u + (uint32_t)(k) * 0x54)

// FUNCTION: HGSIM 0x10069c90
void __cdecl Sim_10069c90(uint8_t *s)
{
    uint32_t obj, k, first;

    if (U32(s, 0x50c) == 0 || U32(s, 0xc35) == 0)
        return;
    if (U32(s, 0x50c) == 1) {
        obj = U32(s, 0);
        if (!(U32(obj, 0x118) & 0x100) || (U32(obj, 0x118) & 0x1000) != 0) {
            Sim_100cbdb8(U32(s, 0), 0, 0);
            k = U32(U32(s, 0), 0x118) & 0xff;
            first = k;
            while (*SIM_PTR(s16a, ENTRY(k) + 0x14) & 0x20) {
                Sim_100cbdb8(U32(s, 0), 1, 0);
                k = U32(U32(s, 0), 0x118) & 0xff;
                if (k == first) {
                    U32(U32(s, 0), 0x118) |= 0x1000;
                    return;
                }
            }
        }
        if (U32(U32(s, 0), 0x118) & 0x100) {
            k = U32(U32(s, 0), 0x118) & 0xff;
            first = k;
            if (!((double)GFL(0x1015e4c4) >= (double)*SIM_PTR(f32o, ENTRY(k))) &&
                U32(U32(s, 0), 4) == G32(0x1017b8c8)) {
                if (!(*SIM_PTR(s16a, ENTRY(k) + 0x14) & 0x20)) {
                    *SIM_PTR(u16a, ENTRY(k) + 0x14) |= 0x20;
                    *SIM_PTR(u16a, ENTRY(k) + 0x16) |= (uint16_t)(1u << (U32(U32(s, 0), 8) & 31));
                    if (*SIM_PTR(s16a, ENTRY(k) + 0x14) & 0x40)
                        Sim_1007cbb4();
                    Sim_1000ebfb((const char *)GBYTES(0x101452d4), 0xfffff63cu, 0, 5, 0x50);
                }
                Sim_100cbdb8(U32(s, 0), 1, 0);
                if ((int32_t)(U32(U32(s, 0), 0x118) & 0xff) < (int32_t)first) {
                    U32(s, 0x50c) = 0;
                    U32(U32(s, 0), 0x118) |= 0x1000;
                    U32(U32(U32(s, 0), 0x54), 8) = 0;
                    U8(U32(U32(s, 0), 0x54), 0x25) = 1;
                    return;
                }
            }
        }
    }
    if (W8W6_CALL_100c2ae5(U32(s, 0)) == 0) {
        Sim_100cb8ea(BYTES(U32(s, 0)));
        Sim_100144e9(BYTES(U32(s, 0)));
        obj = U32(s, 0);
        if (U32(obj, 0x21c) == 1) {
            U32(U32(obj, 0x54), 8) = U32(obj, 0x218);
            U32(U32(s, 0), 0x21c) = 0;
        }
        U32(U32(s, 0), 0x218) = U32(U32(U32(s, 0), 0x54), 8);
    } else {
        U32(U32(s, 0), 0x21c) = 1;
    }
}

// FUNCTION: HGSIM 0x10069fd2
void __cdecl Sim_10069fd2(uint32_t a, uint8_t *s)
{
    int32_t slot, i, k;
    uint32_t entry, part, target;

    if (G32(0x10179ce0) == 0)
        return;
    if (U32(s, 0x8c) != 0) {
        for (slot = 0; slot < 0x11; slot++) {
            entry = U32(s, 0x88) + (uint32_t)slot * 0x3c;
            if (S16(entry, 0x36) & 0x2000)
                continue;
            for (i = 0; i < (int32_t)S16(entry, 0x34); i++) {
                if (U16(entry, 0xc + 2 * i) <= 10000)
                    continue;
                part = U32(s, 0x8c);
                for (k = 0; k < (int32_t)U32(s, 0x51c); k++, part += 0x24) {
                    if ((int32_t)S16(part, 8) != (int32_t)U16(entry, 0xc + 2 * i))
                        continue;
                    target = U32(s, 0x84) + (uint32_t)((int32_t)S16(part, 6) * 0x128);
                    if (target == 0 || (int32_t)U32(target, 0x20) <= 0)
                        continue;
                    W8W6_CALL_1006b52f(a, s, slot + 1, i, 0);
                    return;
                }
            }
        }
    }
    W8W6_CALL_1006a12d(a, s);
}

/* Raw float compare as in the original: the dword moves keep the bits, the FCOM decides. */
static uint32_t Lower(uint32_t mine, uint32_t other)
{
    return !(Float_OfBits(mine) >= Float_OfBits(other)) ? mine : other;
}

static uint32_t Higher(uint32_t mine, uint32_t other)
{
    return Float_OfBits(mine) > Float_OfBits(other) ? mine : other;
}

// FUNCTION: HGSIM 0x1006d5d0
int __cdecl Sim_1006d5d0(uint8_t *node, float *minX, float *maxX, float *minY, float *maxY, float *minZ, float *maxZ)
{
    float box[6], child[6], next[6];
    int haveChild = 0, haveNext = 0;
    uint32_t b0, b1, b2, b3, b4, b5;

    if (node == 0 || minX == 0 || maxX == 0 || minY == 0 || maxY == 0 || minZ == 0 || maxZ == 0)
        return 0;
    if (Sim_100aa7f1(node) != 0)
        haveChild = Sim_1006d5d0(BYTES(U32(node, 4)), &child[0], &child[1], &child[2], &child[3], &child[4],
                                 &child[5]);
    if (Sim_100aa7fc(node) != 0)
        haveNext = Sim_1006d5d0(BYTES(U32(node, 8)), &next[0], &next[1], &next[2], &next[3], &next[4], &next[5]);
    Sim_1002fd4c(BYTES(U32(node, 0x8c)), &box[0], &box[1], &box[2], &box[3], &box[4], &box[5]);
    box[0] = (float)((double)box[0] + (double)FL(node, 0x30));
    box[1] = (float)((double)box[1] + (double)FL(node, 0x30));
    box[2] = (float)((double)box[2] + (double)FL(node, 0x34));
    box[3] = (float)((double)box[3] + (double)FL(node, 0x34));
    box[4] = (float)((double)box[4] + (double)FL(node, 0x38));
    box[5] = (float)((double)box[5] + (double)FL(node, 0x38));
    b0 = Bits_OfFloat(box[0]);
    b1 = Bits_OfFloat(box[1]);
    b2 = Bits_OfFloat(box[2]);
    b3 = Bits_OfFloat(box[3]);
    b4 = Bits_OfFloat(box[4]);
    b5 = Bits_OfFloat(box[5]);
    if (haveChild) {
        b0 = Lower(b0, Bits_OfFloat(child[0]));
        b1 = Higher(b1, Bits_OfFloat(child[1]));
        b2 = Lower(b2, Bits_OfFloat(child[2]));
        b3 = Higher(b3, Bits_OfFloat(child[3]));
        b4 = Lower(b4, Bits_OfFloat(child[4]));
        b5 = Higher(b5, Bits_OfFloat(child[5]));
    }
    if (haveNext) {
        b0 = Lower(b0, Bits_OfFloat(next[0]));
        b1 = Higher(b1, Bits_OfFloat(next[1]));
        b2 = Lower(b2, Bits_OfFloat(next[2]));
        b3 = Higher(b3, Bits_OfFloat(next[3]));
        b4 = Lower(b4, Bits_OfFloat(next[4]));
        b5 = Higher(b5, Bits_OfFloat(next[5]));
    }
    *(u32a *)minX = b0;
    *(u32a *)maxX = b1;
    *(u32a *)minY = b2;
    *(u32a *)maxY = b3;
    *(u32a *)minZ = b4;
    *(u32a *)maxZ = b5;
    return 1;
}

// FUNCTION: HGSIM 0x1006d911
void __cdecl Sim_1006d911(uint8_t *obj)
{
    uint8_t *node = BYTES(U32(obj, 0x48));
    float box[6];

    Sim_100a9d47(node, 0, 0, 0);
    Sim_100aa0da(node, 0, 0, 0, 1);
    Sim_100aa64a(node);
    if (Sim_1006d5d0(node, &box[0], &box[1], &box[2], &box[3], &box[4], &box[5]) != 0) {
        FL(obj, 0x284) = (float)((double)box[0] - (double)GFL(0x1012b6a0));
        FL(obj, 0x288) = (float)((double)box[1] + (double)GFL(0x1012b6a0));
        FL(obj, 0x28c) = (float)((double)box[2] - (double)GFL(0x1012b6a0));
        FL(obj, 0x290) = (float)((double)box[3] + (double)GFL(0x1012b6a0));
        FL(obj, 0x294) = (float)((double)box[4] - (double)GFL(0x1012b6a0));
        FL(obj, 0x298) = (float)((double)box[5] + (double)GFL(0x1012b6a0));
    }
    Sim_100aa0da(node, U32(obj, 0x64), U32(obj, 0x68), U32(obj, 0x6c), 1);
    Sim_100a9d47(node, U32(obj, 0x58), U32(obj, 0x5c), U32(obj, 0x60));
    Sim_100aa64a(node);
}
