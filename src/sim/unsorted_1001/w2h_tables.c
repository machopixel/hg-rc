#include "w2h.h"
#include "../unsorted_1005/w2h.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x10010063
int32_t __cdecl Sim_10010063(int32_t i)
{
    int32_t value = 7;

    if (i < 8)
        value = GS32(0x1012ff58 + (uint32_t)i * 4);
    return value;
}

// FUNCTION: HGSIM 0x10010088
uint32_t __cdecl Sim_10010088(int32_t ignored)
{
    (void)ignored;
    return Sim_10052632((int32_t)G32(0x1013bb40));
}

// FUNCTION: HGSIM 0x10013920
uint32_t __cdecl Sim_10013920(uint32_t a, int16_t b, int16_t c)
{
    (void)b;
    (void)c;
    return a & 0xffff0000u; /* the original clears ax and leaves the rest of eax */
}

// FUNCTION: HGSIM 0x10013e0a
int __cdecl Sim_10013e0a(int a, int b)
{
    (void)a;
    (void)b;
    return 1;
}

// FUNCTION: HGSIM 0x10015bc9
int __cdecl Sim_10015bc9(void)
{
    int i;

    G32(0x10a38a40) = 0;
    for (i = 0; i < 12; i++)
        G32(0x10a38a44 + (uint32_t)i * 4) = G32(0x101302e8 + (uint32_t)i * 4);
    return 1;
}

// FUNCTION: HGSIM 0x10019d96
int __cdecl Sim_10019d96(void)
{
    return 0x2c;
}

// FUNCTION: HGSIM 0x1001f4dd
int __cdecl Sim_1001f4dd(void)
{
    return 0;
}
