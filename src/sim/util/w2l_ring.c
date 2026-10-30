#include "w2l.h"
#include "access.h"

// FUNCTION: HGSIM 0x1009c9a5
int __cdecl Sim_1009c9a5(int m)
{
    int remainder = SIM_PTR(s32a, 0x101cffc0)[GS32(0x1017cd40)] % m;

    GS32(0x1017cd40) = (int32_t)((uint32_t)GS32(0x1017cd40) + 1u) % 0x7f;
    return remainder;
}
