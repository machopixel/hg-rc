#include "w2i.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x100071f6
void __cdecl Sim_100071f6(void)
{
}

// FUNCTION: HGSIM 0x1000942e
void __cdecl Sim_1000942e(void *obj, int16_t value)
{
    U16(obj, 0x24) = (uint16_t)value;
}

// FUNCTION: HGSIM 0x10009558
void __cdecl Sim_10009558(void *obj, int index, uint32_t *a, uint32_t *b, uint32_t *c)
{
    uint32_t block = U32(obj, 0x30);
    uint32_t rec;

    if (block == 0)
        return;
    rec = block + (uint32_t)index * 0x58u + 0x18u;
    if (a != 0)
        *a = U32(rec, 0xc);
    if (b != 0)
        *b = U32(rec, 0x10);
    if (c != 0)
        *c = U32(rec, 0x14);
}
