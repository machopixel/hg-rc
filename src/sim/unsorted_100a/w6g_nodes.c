#include "w6g.h"
#include "w2i.h"
#include "w2j.h"
#include "w5b.h"
#include "w5x.h"
#include "w5y.h"
#include "w5z.h"
#include "../util/util.h"
#include "../util/w3o.h"
#include "../view/view.h"
#include "../view/w2l.h"
#include "../win/w4v.h"
#include "../unsorted_1000/w2f.h"
#include "../unsorted_1000/w2k.h"
#include "../unsorted_1002/w3o.h"
#include "../unsorted_1003/w2l.h"
#include "../unsorted_1006/w5a.h"
#include "../unsorted_1008/w3s.h"
#include "../unsorted_100f/w2e.h"
#include "../render/w2n.h"

#define PTR(v) ((void *)(uintptr_t)(v))

// FUNCTION: HGSIM 0x100a8c24
void __cdecl Sim_100a8c24(void *obj)
{
    if (obj == 0)
        return;
    U32(obj, 0x4e0) = 0;
    (void)W6G_CALL_10047bcd(BYTES(obj) + 4);
    (void)W6G_CALL_10047bcd(BYTES(obj) + 0x14);
    if (U32(obj, 0x490) != 0)
        Sim_100aa0da(PTR(U32(obj, 0x490)), U32(obj, 0x1c), 0, 0, 0);
    if (U32(obj, 0x48c) != 0) {
        U32(U32(obj, 0), 0x70) = 0;
        U32(U32(obj, 0), 0x74) = U32(obj, 0xc);
        U32(U32(obj, 0), 0x78) = 0;
        Sim_100aa0da(PTR(U32(obj, 0x48c)), U32(U32(obj, 0), 0x70), U32(U32(obj, 0), 0x74), U32(U32(obj, 0), 0x78), 0);
    }
    Sim_100aa799(PTR(U32(U32(obj, 0), 0x48)));
}

// FUNCTION: HGSIM 0x100a96d8
void __cdecl Sim_100a96d8(int flag)
{
    if (G32(0x1017d1b8) == 0)
        return;
    if (flag != 0)
        Sim_100aa39f(PTR(G32(0x1017d1b8)));
    else
        Sim_100aa351(PTR(G32(0x1017d1b8)));
    Sim_100aa799(PTR(G32(0x1017d1b8)));
    G32(0x1017d1b0) = (uint32_t)flag;
}

/* The part of the set-up that Sim_100a9830 and Sim_100a9a1a share after linking the node. */
static void Node_InitFrom(uint8_t *node, uint8_t *parent)
{
    Mat3x4_SetIdentity((uint32_t *)(node + 0xc));
    Sim_100610ae(0.0f, 0.0f, 0.0f, 0.0f, (float *)(node + 0x3c));
    if (parent != 0) {
        Mem_Move(node + 0x4c, parent + 0x4c, 0x30);
        Mem_Move(node + 0x7c, parent + 0x7c, 0x10);
    } else {
        Mat3x4_SetIdentity((uint32_t *)(node + 0x4c));
        Sim_100610ae(0.0f, 0.0f, 0.0f, 0.0f, (float *)(node + 0x7c));
    }
    U32(node, 0x9c) = 0;
    U32(node, 0xa0) = 0;
    U32(node, 0x8c) = 0;
}

// FUNCTION: HGSIM 0x100a9830
void *__cdecl Sim_100a9830(void *parent, uint32_t flags)
{
    uint8_t *node;

    if ((flags & 2) != 0) {
        node = Heap_AllocRaw(0xa4);
        if (node == 0)
            return 0;
        U32(node, 0x94) = 2;
    } else if ((flags & 4) != 0) {
        node = SIM_API(W6gHeapAllocFn, 0x10ac5658)(G32(0x1017b8f8), 1, 0xa4);
        if (node == 0)
            return 0;
        U32(node, 0x94) = 4;
    } else {
        return 0;
    }
    U32(node, 0) = (uint32_t)(uintptr_t)parent;
    U32(node, 4) = 0;
    U32(node, 8) = 0;
    if (parent != 0) {
        if ((flags & 0x10) != 0) {
            uint8_t *last = Sim_100aabfd(parent);

            if (last != 0)
                U32(last, 8) = (uint32_t)(uintptr_t)node;
            else
                U32(parent, 4) = (uint32_t)(uintptr_t)node;
        } else {
            U32(node, 8) = U32(parent, 4);
            U32(parent, 4) = (uint32_t)(uintptr_t)node;
        }
    }
    Node_InitFrom(node, parent);
    U32(node, 0x94) |= 1;
    U32(node, 0x90) = 0;
    U32(node, 0x98) = G32(0x1014d1a8);
    G32(0x1014d1a8) = ((G32(0x1014d1a8) + 7) & 0x7f) + 0x40;
    return node;
}

// FUNCTION: HGSIM 0x100a9a1a
void *__cdecl Sim_100a9a1a(void *parent, void *node)
{
    U32(node, 0) = (uint32_t)(uintptr_t)parent;
    U32(node, 4) = 0;
    if (parent != 0) {
        U32(node, 8) = U32(parent, 4);
        U32(parent, 4) = (uint32_t)(uintptr_t)node;
    } else {
        U32(node, 8) = 0;
    }
    Node_InitFrom(node, parent);
    U32(node, 0x94) = 1;
    U32(node, 0x90) = 0;
    U32(node, 0x98) = G32(0x1014d1a8);
    G32(0x1014d1a8) = ((G32(0x1014d1a8) + 7) & 0x7f) + 0x40;
    return node;
}

// FUNCTION: HGSIM 0x100a9e1a
void __cdecl Sim_100a9e1a(void *node, const void *src)
{
    int i;

    for (i = 0; i < 9; i++)
        U32(node, 0xc + 4 * i) = U32(src, 4 * i);
    W6G_CALL_10060d2e(src, BYTES(node) + 0x3c);
    U32(node, 0x94) |= 1;
    U32(node, 0x94) |= 0x4000;
}

// FUNCTION: HGSIM 0x100aa8cb
void __cdecl Sim_100aa8cb(void *node)
{
    uint32_t self = (uint32_t)(uintptr_t)node;
    uint32_t parent = U32(node, 0);

    if (parent != 0) {
        if (U32(parent, 4) == self) {
            U32(parent, 4) = U32(node, 8);
        } else {
            uint32_t p;

            for (p = U32(parent, 4); p != 0; p = U32(p, 8)) {
                if (U32(p, 8) == self) {
                    U32(p, 8) = U32(node, 8);
                    break;
                }
            }
        }
    }
    U32(node, 0) = 0;
    U32(node, 8) = 0;
    Mem_Move(BYTES(node) + 0xc, BYTES(node) + 0x4c, 0x30);
    Mem_Move(BYTES(node) + 0x3c, BYTES(node) + 0x7c, 0x10);
    U32(node, 0x94) |= 0x4000;
    U32(node, 0x94) &= ~0x8000u;
    U32(node, 0x94) |= 1;
    if (U32(node, 0x8c) != 0) {
        U16(U32(node, 0x8c), 0x24) = 0xffff;
        Sim_1008a40a(PTR(U32(node, 0x8c)));
        Sim_1008a46e(PTR(U32(node, 0x8c)));
    }
    Sim_100aa799(node);
}

// FUNCTION: HGSIM 0x100aae7b
void __cdecl Sim_100aae7b(void *node, int level, uint32_t key)
{
    int current = (int)((Sim_100093ea(PTR(U32(node, 0x8c))) & 0xf0) >> 4);
    uint32_t child;

    if (level <= 0 || level >= 0x10 || level <= current)
        return;
    level <<= 4;
    if (U32(node, 0x8c) != 0 && U16(U32(node, 0x8c), 0x26) == key)
        Sim_100093be((uint16_t *)PTR(U32(node, 0x8c)), (uint32_t)level);
    for (child = U32(node, 4); child != 0; child = U32(child, 8))
        Sim_100aae7b(PTR(child), level >> 4, key);
}

// FUNCTION: HGSIM 0x100aaf32
void __cdecl Sim_100aaf32(void *node)
{
    if (node == 0 || U32(node, 0x8c) == 0)
        return;
    Sim_10009408(PTR(U32(node, 0x8c)), 0x50);
    Sim_100aa8cb(node);
    Sim_100aa351(node);
    Sim_100aa406(node);
    Sim_100aa799(node);
    Sim_1003b8e3((int)(uintptr_t)node);
}

// FUNCTION: HGSIM 0x100ab186
void __cdecl Sim_100ab186(void *a, void *b)
{
    uint32_t x, y, z;

    if (a == 0 || b == 0)
        return;
    W6G_CALL_100aaa08(b, a);
    Sim_100a9d47(b, 0, 0, 0);
    Sim_100a9ccf(a, (uint32_t)(uintptr_t)&x, (uint32_t)(uintptr_t)&y, (uint32_t)(uintptr_t)&z);
    Sim_100aa0da(b, x, y, z, 0);
}

// FUNCTION: HGSIM 0x100ad87d
void __cdecl Sim_100ad87d(void *ctx, void *node)
{
    uint32_t obj, mesh, child;
    int count, i;

    if (node == 0 || ctx == 0 || U32(node, 0x8c) == 0)
        return;
    obj = U32(node, 0x8c);
    mesh = U32(obj, 0x30);
    if (U32(obj, 0x9c) != U32(U32(obj, 0x30), 0x10))
        Sim_100aa307(PTR(obj));
    count = S16(mesh, 6);
    if ((U16(U32(node, 0x8c), 2) & 0x70) == 0) {
        for (i = 0; i < count; i++)
            Sim_100ad1f3(ctx, BYTES(mesh) + U32(mesh, 8) + (uint32_t)i * 0x28);
    }
    if (U32(node, 4) != 0) {
        for (child = U32(node, 4); child != 0; child = U32(child, 8))
            Sim_100ad87d(ctx, PTR(child));
    }
}
