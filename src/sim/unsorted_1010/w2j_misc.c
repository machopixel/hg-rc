#include "w2j.h"

// FUNCTION: HGSIM 0x10108710
uint32_t __cdecl Sim_10108710(uint32_t flags)
{
    uint32_t result = 0;
    uint32_t pair, low;

    if (flags & 1)
        result = 0x10;
    if (flags & 4)
        result |= 8;
    if (flags & 8)
        result |= 4;
    if (flags & 0x10)
        result |= 2;
    if (flags & 0x20)
        result |= 1;
    if (flags & 2)
        result |= 0x80000;
    pair = flags & 0xc00;
    if (pair == 0x400)
        result |= 0x100;
    else if (pair == 0x800)
        result |= 0x200;
    else if (pair == 0xc00)
        result |= 0x300;
    low = flags & 0x300;
    if (low == 0)
        result |= 0x20000;
    else if (low == 0x200)
        result |= 0x10000;
    if (flags & 0x1000)
        result |= 0x40000;
    return result;
}

// FUNCTION: HGSIM 0x101091f0
int __cdecl Sim_101091f0(void)
{
    return 0;
}
