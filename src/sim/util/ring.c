#include "util.h"
#include "access.h"

// FUNCTION: HGSIM 0x1009c929
int __cdecl Sim_1009c929(int m)
{
    int remainder = (int)SIM_PTR(s32a, 0x101cffc0)[GS32(0x1017cd38)] % m;

    GS32(0x1017cd38) = GS32(0x1017cd38) + 1;
    if (GS32(0x1017cd38) == 0x7f)
        GS32(0x1017cd38) = 0;
    return remainder;
}

// FUNCTION: HGSIM 0x1009c969
uint32_t __cdecl Sim_1009c969(void)
{
    uint32_t value = SIM_PTR(u32a, 0x101cfce0)[GS32(0x1017cd3c)];

    GS32(0x1017cd3c) = GS32(0x1017cd3c) + 1;
    if (GS32(0x1017cd3c) == 0x7f)
        GS32(0x1017cd3c) = 0;
    return value;
}
