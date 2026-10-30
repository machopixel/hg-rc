#include "w4v.h"
#include "../util/access.h"
#include "../win/w4a.h"
#include "../win/w4v.h"
#include "../view/view.h"
#include "../util/util.h"
#include "../unsorted_100a/w2g.h"
#include "../unsorted_100a/w3t.h"
#include "../unsorted_100d/w2i.h"
#include "../unsorted_1004/w2l.h"
#include "../unsorted_100f/w2e.h"

#define PTR(v) ((void *)(uintptr_t)(v))

// FUNCTION: HGSIM 0x10034b30
int __cdecl Sim_10034b30(void)
{
    uint32_t rec;

    G32(0x109e3da4) = (uint32_t)(uintptr_t)Heap_AllocZero(0x18);
    if (G32(0x109e3da4) == 0)
        return 0;
    rec = G32(0x109e3da4);
    U32(rec, 0) = G32(0x1017b918);
    U32(G32(0x109e3da4), 4) = G32(0x1017b91c);
    U32(G32(0x109e3da4), 8) = 1;
    U32(G32(0x109e3da4), 0xc) = 0x100;
    U32(G32(0x109e3da4), 0x10) = 1;
    U32(G32(0x109e3da4), 0x14) = 0;
    G32(0x109e3de4) = G32(0x1017b918) * G32(0x1017b91c);
    G32(0x109e3de8) = G32(0x1017b918);
    G32(0x109e3da8) = G32(0x1017b91c);
    G32(0x109e3e14) = G32(0x1017b918) - 1u;
    G32(0x109e3de0) = G32(0x1017b91c) - 1u;
    GS32(0x109e3e18) = GS32(0x109e3de8) / 2;
    GS32(0x109e3e1c) = GS32(0x109e3da8) / 2;
    return 1;
}

// FUNCTION: HGSIM 0x10034c11
int __cdecl Sim_10034c11(void)
{
    int result = 0;
    int i;

    if (Sim_10034b30() != 0) {
        Sim_100ab89a((const int32_t *)PTR(G32(0x109e3da4)));
        Camera_SetAspect((const int32_t *)PTR(G32(0x109e3da4)));
        G32(0x109e3e00) = (uint32_t)(uintptr_t)GBYTES(0x109e3e20);
        G32(0x109e3e04) = 0;
        G32(0x109e3e08) = 0;
        G32(0x109e3e0c) = U32(G32(0x109e3da4), 0) - 1u;
        G32(0x109e3e10) = U32(G32(0x109e3da4), 4) - 1u;
        for (i = 0; i < 5; i++)
            G32(0x1015e518 + 4 * i) = G32(0x109e3e00 + 4 * i);
        result = 1;
        Sim_100d3142((const uint32_t *)GBYTES(0x109e3e00));
        Sim_10047fe0();
        G32(0x1015e538) = 1;
    }
    return result;
}

// FUNCTION: HGSIM 0x1003730b
int __cdecl Sim_1003730b(uint32_t unused, void *owner)
{
    uint8_t *rec;
    uint32_t first, second, third;
    uint8_t *entry;
    int i;

    (void)unused;
    U32(owner, 0x28) = 0;
    rec = Heap_AllocRaw((uint32_t)Sim_10072ff2());
    if (rec == 0)
        return 0;
    first = (uint32_t)(uintptr_t)rec + 0xc51;
    second = first + 0x940;
    third = second + 0x3fc;
    U32(rec, 0x84) = first;
    U32(rec, 0x88) = second;
    U32(rec, 0x8c) = third;
    U32(rec, 0) = (uint32_t)(uintptr_t)owner;
    for (i = 0; i < 10; i++)
        U32(rec, 0x498 + 4 * i) = 0;
    U32(owner, 0x28) = (uint32_t)(uintptr_t)rec;
    U32(owner, 0x2c) = 0xc51;
    entry = PTR(U32(rec, 0x84));
    for (i = 0; i < 8; i++) {
        U32(entry, 0) = 0xffffffffu;
        U32(entry, 4) = 0xffffffffu;
        U32(entry, 8) = 0xffffffffu;
        U32(entry, 0x10) = 0;
        U32(entry, 0x14) = 0;
        U32(entry, 0x20) = 0xffffffffu;
        entry += 0x128;
    }
    return 1;
}

// FUNCTION: HGSIM 0x10038fac
void __cdecl Sim_10038fac(uint32_t value, uint32_t key, const void *rec, const void *src)
{
    uint32_t node;
    uint32_t a, b, c;
    int n = 0;
    int i;

    for (node = G32(0x109e3760); node != 0; node = U32(node, 0x690)) {
        if (U32(node, 0) == key)
            return;
    }
    node = (uint32_t)(uintptr_t)Heap_AllocRaw(0x698);
    U32(node, 0x690) = G32(0x109e3760);
    U32(node, 0x694) = 0;
    U32(node, 0) = key;
    U32(node, 4) = U32(U32(node, 0), 4);
    U32(node, 8) = value;
    Sim_100a9cfb(PTR(key), &a, &b, &c);
    U32(node, 0x680) = a;
    U32(node, 0x684) = b;
    U32(node, 0x688) = c;
    U32(node, 0x68c) = U32(rec, 0x5ff);
    Mem_Move(PTR(node + 0xc), src, 0x128);
    for (i = S32(src, 0x108); i < S32(src, 0x38); i++) {
        uint32_t record = U32(rec, 0x8c) + U32(src, 0x3c + 4 * i) * 0x24;

        Mem_Move(PTR(node + (uint32_t)n * 0x24 + 0x134), PTR(record), 0x24);
        n++;
        U16(U32(rec, 0x8c) + U32(src, 0x3c + 4 * i) * 0x24, 2) = 0;
    }
    U32(G32(0x109e3760), 0x694) = node;
    G32(0x109e3760) = node;
}
