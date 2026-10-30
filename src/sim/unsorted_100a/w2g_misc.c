#include "w2g.h"
#include "../simmem.h"
#include "../unsorted_100f/w2g.h"

#define G32(addr) (*SIM_PTR(uint32_t, addr))

// FUNCTION: HGSIM 0x100a8943
uint32_t __cdecl Sim_100a8943(void)
{
    uint32_t old = G32(0x1017D178);

    G32(0x1017D178) = old + 0x10;
    if (old + 0x10 + 0xC8 >= G32(0x1017D174))
        G32(0x101C9EA4) = 0;
    return old;
}

// FUNCTION: HGSIM 0x100aa807
uint8_t *__cdecl Sim_100aa807(uint8_t *p)
{
    return p + 0xC;
}

// FUNCTION: HGSIM 0x100ab89a
void __cdecl Sim_100ab89a(const int32_t *point)
{
    int32_t best = 7;
    int32_t i;

    for (i = 0; i < 3; i++) {
        int32_t dx = (int32_t)(*SIM_PTR(uint32_t, 0x1014D1B8u + (uint32_t)i * 8u) - ((uint32_t)point[0] - 1u));
        int32_t dy = (int32_t)(*SIM_PTR(uint32_t, 0x1014D1BCu + (uint32_t)i * 8u) - ((uint32_t)point[1] - 1u));
        int32_t sum = (int32_t)((uint32_t)Math_AbsInt_100fd100(dx) + (uint32_t)Math_AbsInt_100fd100(dy));

        if (sum < best) {
            best = sum;
            G32(0x101C64F0) = (uint32_t)i;
        }
    }
}

// FUNCTION: HGSIM 0x100ac5da
void __cdecl Sim_100ac5da(int value)
{
    if (value < 1)
        value = 0;
    else if (value > 0xF)
        value = 0xF;
    G32(0x1017D1E0) = (uint32_t)value;
}

// FUNCTION: HGSIM 0x100ac603
int __cdecl Sim_100ac603(int value)
{
    if (value == -1)
        return (int)G32(0x1014D1D8);
    G32(0x1014D1D8) = (uint32_t)value;
    return value;
}
