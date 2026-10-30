#include "w9e.h"
#include "w3o.h"
#include "w4v.h"
#include "../unsorted_1000/w2l.h"
#include "../unsorted_100a/w2g.h"
#include "../unsorted_100a/w5x.h"
#include "../unsorted_100a/w5y.h"
#include "../unsorted_100a/w5z.h"
#include "../unsorted_100a/w7o.h"
#include "../unsorted_100f/w2e.h"
#include "../util/access.h"

#define LINK(i) (GBYTES(0x101a2100) + (uint32_t)(i) * 0x88)

// FUNCTION: HGSIM 0x100b43ce
int __cdecl Sim_100b43ce(int index)
{
    uint8_t *rec = LINK(index);
    uint8_t *target, *obj, *node = 0;
    uint8_t matrix[0x30], other[0x30];
    uint32_t px = 0, py = 0, pz = 0;
    int next;

    if (S32(rec, 0x24) == -1)
        return -1;
    target = GBYTES(0x109e9ba0) + (uint32_t)S32(rec, 0x24) * 0x50;
    U32(rec, 0x18) |= 0x200;
    U32(rec, 0x18) |= 0x800;
    U16(target, 0) |= 4;
    U32(target, 8) = 0xffffffffu;
    Sim_100b42aa(index);
    obj = BYTES(U32(rec, 0x28));
    if (obj != 0) {
        node = BYTES(U32(rec, 0x2c));
        if (node != 0) {
            Mem_Move(matrix, Sim_100aa863(node), 0x30);
            Mem_Move(other, Sim_100aa807(node), 0x30);
            Sim_100a9ccf(node, (uint32_t)(uintptr_t)&px, (uint32_t)(uintptr_t)&py, (uint32_t)(uintptr_t)&pz);
        }
        Sim_10009787(obj);
        U32(rec, 0x28) = 0;
        U32(rec, 0x2c) = 0;
    }
    next = S32(rec, 0x20);
    if (next == -1)
        return next;
    rec = LINK(next);
    U32(rec, 0x18) &= ~0x800u;
    if (W9E_CALL_100b48de(next, U32(rec, 0x10)) == 0) {
        U32(rec, 0x18) &= 0x800u;
        return -1;
    }
    node = BYTES(U32(rec, 0x2c));
    if (node != 0) {
        Sim_100aa87a(node, matrix);
        Sim_100aa812(node, matrix);
        Sim_100aa0da(node, px, py, pz, 0);
    }
    Sim_100b42d5(next);
    if (node != 0)
        Sim_100aa64a(node);
    return next;
}
