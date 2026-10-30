#include "w2k.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x10037306
void __cdecl Sim_10037306(void)
{
}

/* A shift count of the original is taken modulo 32 by the processor. */
static uint32_t Channel(uint32_t pixel, uint32_t shiftAddr, uint32_t bitsAddr)
{
    uint32_t shift = G32(shiftAddr);
    uint32_t bits = G32(bitsAddr);

    return ((pixel >> (shift & 31)) << ((8 - bits) & 31)) & 0xff;
}

// FUNCTION: HGSIM 0x10037680
uint32_t __cdecl Sim_10037680(uint32_t pixel)
{
    uint32_t c0 = Channel(pixel, 0x109E38A0, 0x109E3890);
    uint32_t c1 = Channel(pixel, 0x109E38A4, 0x109E3894);
    uint32_t c2 = Channel(pixel, 0x109E38A8, 0x109E3898);
    uint32_t c3 = Channel(pixel, 0x109E38AC, 0x109E389C);

    return (c0 << 24) | (c1 << 16) | (c2 << 8) | c3;
}

// FUNCTION: HGSIM 0x10037c9e
void __cdecl Sim_10037c9e(uint32_t value)
{
    G32(0x1017B8F0) = value;
}

// FUNCTION: HGSIM 0x1003d863
void __cdecl Sim_1003d863(void)
{
    G32(0x1013ABF0) = G32(0x1014CB3C);
    G32(0x1014CB3C) = (uint32_t)(uintptr_t)GBYTES(0x10003837);
    G32(0x1015EBF4) = 0;
}
