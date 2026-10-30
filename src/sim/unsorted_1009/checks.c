#include "fns.h"
#include "../simmem.h"
#include "../unsorted_1003/fns.h"
#include "../unsorted_100d/fns.h"

// FUNCTION: HGSIM 0x100988b9
int __cdecl Sim_100988b9(uint32_t unused)
{
    return Sim_10033291(0x100) != 0 || Sim_10033291(0x200) != 0;
}

// FUNCTION: HGSIM 0x10098966
int __cdecl Sim_10098966(uint32_t unused)
{
    return Sim_10033291(0x800) != 0 || *SIM_PTR(uint32_t, 0x1017D1B0) != 0;
}

// FUNCTION: HGSIM 0x100989dd
int __cdecl Sim_100989dd(uint32_t unused)
{
    return Sim_100d2c6f() != 0 || Sim_100332d2() != 0;
}
