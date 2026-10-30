#include "w2k.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x100d1648
void *__cdecl Sim_100d1648(void *owner, void *chain)
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
    U32(last, 8) = U32(owner, 0xC);
    U32(owner, 0xC) = first;
    if (U32(owner, 0x10) == 0)
        U32(owner, 0x10) = last;
    else
        U32(U32(last, 8), 4) = last;
    return (void *)(uintptr_t)first;
}

// FUNCTION: HGSIM 0x100d2c4f
int __cdecl Sim_100d2c4f(const void *obj)
{
    return (U16(obj, 0) & 0x1000) != 0;
}
