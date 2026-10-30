#include "w2j.h"
#include "../util/access.h"
#include "../unsorted_1002/w2j.h"

// FUNCTION: HGSIM 0x100089aa
uint32_t __cdecl Sim_100089aa(uint32_t value)
{
    uint32_t old = G32(0x1015d09c);

    G32(0x1015d09c) = value;
    return old;
}

// FUNCTION: HGSIM 0x100091f7
void *__cdecl Sim_100091f7(void *obj, unsigned short value, uint32_t offset)
{
    uint32_t block = U32(obj, 0x30);
    uint32_t entry;

    if (block == 0)
        return 0;
    entry = block + U32(block, 8) + (uint32_t)((int32_t)S16(block, 6) * 0x28);
    U32(entry, 4) = (offset - entry) & 0xffff;
    U32(entry, 0x24) = (uint32_t)(uintptr_t)obj;
    U16(entry, 0) = value;
    U16(entry, 2) = 0;
    U32(entry, 8) = 0;
    U16(block, 6) = (uint16_t)(U16(block, 6) + 1);
    return BYTES(entry);
}

// FUNCTION: HGSIM 0x1000943e
uint32_t __cdecl Sim_1000943e(const void *obj)
{
    return U16(obj, 2);
}

// FUNCTION: HGSIM 0x1000f029
void __cdecl Sim_1000f029(void *obj)
{
    if (obj == 0 || U32(obj, 8) == 0)
        return;
    U32(G32(0x1015d0a4), U32(obj, 4) * 4 + 0x54) = 0;
    U32(G32(0x1015d0a4), U32(obj, 4) * 4 + 0x2d4) = 0;
    U32(obj, 4) = 0xffffffffu;
    Sim_1002828e((int)S16(obj, 0x1c), (const void *)(uintptr_t)G32(0x1014ceb4));
    U32(obj, 8) = 0;
}
