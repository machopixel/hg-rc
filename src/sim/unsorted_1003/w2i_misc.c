#include "w2i.h"
#include "../util/access.h"

#define ENTRY(i) (0x109B9E20u + (uint32_t)(i) * 0x7eu)

// FUNCTION: HGSIM 0x10033555
unsigned char *__cdecl Sim_10033555(unsigned char *entry)
{
    return entry != 0 ? entry + 4 : 0;
}

// FUNCTION: HGSIM 0x10035444
void __cdecl Sim_10035444(int v)
{
    if (v >= 0 && v < 11)
        GS32(0x1015E53C) = v;
    else
        GS32(0x1015E53C) = 0;
}

// FUNCTION: HGSIM 0x10037675
int __cdecl Sim_10037675(void)
{
    return 0;
}

// FUNCTION: HGSIM 0x1003a53b
uint32_t __cdecl Sim_1003a53b(void)
{
    return U32(G32(0x1015D0A4), 0x10);
}

// FUNCTION: HGSIM 0x1003b471
void __cdecl Sim_1003b471(int index, uint32_t value)
{
    G32(ENTRY(index) + 4) = value;
}

// FUNCTION: HGSIM 0x1003b4d0
uint32_t __cdecl Sim_1003b4d0(int index)
{
    uint32_t result = 0;

    if (index < GS32(0x1015EA80) && index >= 0)
        result = G32(ENTRY(index) + 0x50u + G32(ENTRY(index) + 4) * 4u);
    return result;
}
