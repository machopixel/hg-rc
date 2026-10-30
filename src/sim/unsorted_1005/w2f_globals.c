#include "w2f.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x100570e7
int __cdecl Sim_100570e7(void)
{
    return 2;
}

// FUNCTION: HGSIM 0x10057a51
int __cdecl Sim_10057a51(void)
{
    return 0;
}

// FUNCTION: HGSIM 0x10058549
void __cdecl Sim_10058549(int v)
{
    if (v < 1)
        v = 0;
    else if (v > 0x80)
        v = 0x80;
    GS32(0x10143fbc) = v;
}

// FUNCTION: HGSIM 0x100585c4
int __cdecl Sim_100585c4(int v)
{
    if (v == -1)
        return GS32(0x10143fd8);
    GS32(0x10143fd8) = v;
    return v;
}

// FUNCTION: HGSIM 0x10058716
void __cdecl Sim_10058716(uint32_t a, uint32_t b, uint32_t c, uint32_t d)
{
    G32(0x109b16c4) = a;
    G32(0x109b16c8) = b;
    G32(0x109b16c0) = c;
    G32(0x109afdcc) = d;
}

// FUNCTION: HGSIM 0x1005873d
void __cdecl Sim_1005873d(uint32_t a)
{
    G32(0x101668b4) = a;
}
