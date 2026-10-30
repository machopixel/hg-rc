#include "fns.h"
#include "../simmem.h"

// FUNCTION: HGSIM 0x100d5c9b
int __cdecl Sim_100d5c9b(int value)
{
    return value == 0x1b || value == 0x2a || value == 0x29;
}

// FUNCTION: HGSIM 0x100dcbe0
uint32_t __cdecl Sim_100dcbe0(uint32_t flags)
{
    if (flags & 0x730)
        flags |= 0x730;
    if (flags & 3)
        flags |= 3;
    return flags;
}

// FUNCTION: HGSIM 0x100d2c6f
int __cdecl Sim_100d2c6f(void)
{
    return *SIM_PTR(int32_t, 0x10152418) == 1;
}

// FUNCTION: HGSIM 0x100d1df9
int __cdecl Sim_100d1df9(void)
{
    return 0;
}
