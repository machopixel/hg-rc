#include "w2i.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x100d16e4
void *__cdecl Sim_100d16e4(void *owner, void *chain)
{
    uint32_t first = (uint32_t)(uintptr_t)chain;
    uint32_t last = (uint32_t)(uintptr_t)chain;
    uint32_t node;

    while (U32(first, 4) != 0)
        first = U32(first, 4);
    while (U32(last, 8) != 0)
        last = U32(last, 8);
    for (node = first; node != 0; node = U32(node, 8))
        U32(node, 0x14) = (uint32_t)(uintptr_t)owner;
    U32(first, 4) = U32(owner, 0x10);
    U32(owner, 0x10) = last;
    if (U32(owner, 0xc) == 0)
        U32(owner, 0xc) = first;
    else
        U32(U32(first, 4), 8) = first;
    return (void *)(uintptr_t)last;
}

// FUNCTION: HGSIM 0x100d1834
void *__cdecl Sim_100d1834(void *owner, void *chain)
{
    uint32_t first = (uint32_t)(uintptr_t)chain;
    uint32_t last = (uint32_t)(uintptr_t)chain;
    uint32_t node;

    while (U32(first, 4) != 0)
        first = U32(first, 4);
    while (U32(last, 8) != 0)
        last = U32(last, 8);
    for (node = first; node != 0; node = U32(node, 8))
        U32(node, 0x14) = U32(owner, 0x14);
    U32(last, 8) = (uint32_t)(uintptr_t)owner;
    U32(first, 4) = U32(owner, 4);
    U32(owner, 4) = last;
    if (U32(first, 4) != 0)
        U32(U32(first, 4), 8) = first;
    else if (U32(owner, 0x14) != 0)
        U32(U32(owner, 0x14), 0xc) = first;
    return (void *)(uintptr_t)first;
}

// FUNCTION: HGSIM 0x100d339e
void __cdecl Sim_100d339e(void *obj, int *out1, int *out2)
{
    int32_t x = (int32_t)((uint32_t)((int32_t)(U32(obj, 0x2c) + U32(obj, 0x30)) / 2) + U32(obj, 0x4c));
    int32_t y = (int32_t)((uint32_t)((int32_t)(U32(obj, 0x34) + U32(obj, 0x38)) / 2) + U32(obj, 0x50));

    *out1 = x;
    *out2 = y;
}
