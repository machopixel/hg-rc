#include "w2h.h"
#include "../unsorted_1001/w2h.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x100a774e
int __cdecl Sim_100a774e(void)
{
    return Sim_10015bc9();
}

// FUNCTION: HGSIM 0x100a88ff
uint32_t __cdecl Sim_100a88ff(void)
{
    uint32_t top = G32(0x1017d174) - 0x40;

    G32(0x1017d174) = top;
    U8(top, 0x29) = 0;
    if (G32(0x1017d174) - 0xc8 <= G32(0x1017d178))
        G32(0x101c9ea4) = 0;
    return top;
}

// FUNCTION: HGSIM 0x100aa270
void __cdecl Sim_100aa270(void *obj, const uint32_t *src)
{
    uint32_t *dst = (uint32_t *)((uint8_t *)obj + 0x7c);

    dst[0] = src[0];
    dst[1] = src[1];
    dst[2] = src[2];
    dst[3] = src[3];
    U32(obj, 0x94) = U32(obj, 0x94) & 0xffff7fffu;
    U32(obj, 0x94) = U32(obj, 0x94) | 1;
}
