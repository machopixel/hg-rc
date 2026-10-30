#include "fns.h"
#include "../simmem.h"

// FUNCTION: HGSIM 0x1003217c
void __cdecl Sim_1003217c(void)
{
    *SIM_PTR(uint32_t, 0x1015DD6C) = 1;
}

/* Accepts 0..4, returns 2 for anything else. */
// FUNCTION: HGSIM 0x10034855
int __cdecl Sim_10034855(int value)
{
    if (value < 0 || value > 4)
        return 2;
    *SIM_PTR(int32_t, 0x1015E4F8) = value;
    return 0;
}

// FUNCTION: HGSIM 0x1003a52b
void __cdecl Sim_1003a52b(uint32_t value)
{
    *(uint32_t *)(*SIM_PTR(unsigned char *, 0x1015D0A4) + 0x10) = value;
}
