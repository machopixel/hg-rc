#include "w2j.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x100d1780
void *__cdecl Sim_100d1780(void *a, void *b)
{
    uint32_t first = (uint32_t)(uintptr_t)b;
    uint32_t last = (uint32_t)(uintptr_t)b;
    uint32_t node;

    while (U32(first, 4) != 0)
        first = U32(first, 4);
    while (U32(last, 8) != 0)
        last = U32(last, 8);
    for (node = first; node != 0; node = U32(node, 8))
        U32(node, 0x14) = U32(a, 0x14);
    U32(first, 4) = (uint32_t)(uintptr_t)a;
    U32(last, 8) = U32(a, 8);
    U32(a, 8) = first;
    if (U32(last, 8) != 0)
        U32(U32(last, 8), 4) = last;
    else if (U32(a, 0x14) != 0)
        U32(U32(a, 0x14), 0x10) = last;
    return BYTES(last);
}

// FUNCTION: HGSIM 0x100d8ce7
void __cdecl Sim_100d8ce7(uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t e)
{
}

// FUNCTION: HGSIM 0x100d9fc0
void __cdecl Sim_100d9fc0(void)
{
}
