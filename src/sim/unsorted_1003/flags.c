#include "fns.h"
#include "../simmem.h"

#define BITS SIM_PTR(uint32_t, 0x1014CB38)
#define FLAG SIM_PTR(uint32_t, 0x1014CB34)

// FUNCTION: HGSIM 0x1003327f
void __cdecl Sim_1003327f(uint32_t mask)
{
    *BITS ^= mask;
}

// FUNCTION: HGSIM 0x10033291
int __cdecl Sim_10033291(uint32_t mask)
{
    return (*BITS & mask) == 0;
}

// FUNCTION: HGSIM 0x100332a3
void __cdecl Sim_100332a3(uint32_t mask, int on)
{
    if (on != 0)
        *BITS &= ~mask;
    else
        *BITS |= mask;
}

// FUNCTION: HGSIM 0x100332d2
int __cdecl Sim_100332d2(void)
{
    return *FLAG == 0;
}
