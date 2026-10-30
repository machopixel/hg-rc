#include "w2j.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x10034877
void __cdecl Sim_10034877(uint32_t value)
{
    G32(0x1015e4fc) = value;
}

// FUNCTION: HGSIM 0x10034884
void __cdecl Sim_10034884(uint32_t value)
{
    G32(0x10139e50) = value;
}

// FUNCTION: HGSIM 0x10035589
uint32_t __cdecl Sim_10035589(uint32_t unused)
{
    return G32(0x109e3d80);
}

// FUNCTION: HGSIM 0x1003b449
void __cdecl Sim_1003b449(int index, uint32_t value)
{
    U32(SIM_PTR(uint8_t, 0x109b9e20u + (uint32_t)index * 0x7eu), 0) = value;
}
