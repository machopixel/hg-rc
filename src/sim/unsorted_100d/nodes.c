#include "fns.h"
#include "../simmem.h"

#define U32(p) (*(uint32_t *)(uintptr_t)(p))
#define NODE(p) ((uint8_t *)(uintptr_t)(p))

// FUNCTION: HGSIM 0x100d1562
void __cdecl List_Unlink(void *node)
{
    uint8_t *n = node;
    uint8_t *owner = NODE(U32(n + 0x14));

    if (owner == 0)
        return;
    if (U32(n + 8) != 0)
        U32(NODE(U32(n + 8)) + 4) = U32(n + 4);
    else
        U32(owner + 0x10) = U32(n + 4);
    if (U32(n + 4) != 0)
        U32(NODE(U32(n + 4)) + 8) = U32(n + 8);
    else
        U32(owner + 0xc) = U32(n + 8);
    U32(n + 8) = 0;
    U32(n + 4) = 0;
    U32(n + 0x14) = 0;
}

// FUNCTION: HGSIM 0x100d7ee0
void *__cdecl SlotPool_Take(void *owner)
{
    uint8_t *o = owner;
    uint8_t *entry = 0;

    if (o == 0 || (int32_t)U32(o + 0x754) >= 0x2d)
        return 0;
    entry = o + U32(o + U32(o + 0x754) * 4 + 0x758) * 0x24 + 0xfc;
    U32(o + 0x754) = U32(o + 0x754) + 1;
    return entry;
}

// FUNCTION: HGSIM 0x100d7f41
void __cdecl SlotPool_Release(void *owner, int index)
{
    uint8_t *o = owner;

    if (o == 0 || (int32_t)U32(o + 0x754) <= 0 || index < 0 || index >= (int32_t)U32(o + 0x750))
        return;
    U32(o + (uint32_t)index * 0x24 + 0xfc + 4) = 0xFFFFFFFFu;
    U32(o + 0x754) = U32(o + 0x754) - 1;
    U32(o + U32(o + 0x754) * 4 + 0x758) = (uint32_t)index;
}

// FUNCTION: HGSIM 0x100da020
void __cdecl Sim_100da020(void *block, uint32_t *out8, uint32_t *outC, uint32_t *payload)
{
    uint8_t *b = block;

    if (out8)
        *out8 = U32(b + 8);
    if (outC)
        *outC = U32(b + 0xc);
    if (payload) {
        int count = (int)((U32(b + 4) - 0x10) >> 2);
        int i;

        for (i = 0; i < count; i++)
            payload[i] = U32(b + (uint32_t)i * 4 + 0x10);
    }
}
