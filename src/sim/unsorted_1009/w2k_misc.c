#include "w2k.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x100987e6
uint32_t __cdecl Sim_100987e6(uint32_t unused)
{
    return U32(G32(0x1015D1B8), 0x64);
}

// FUNCTION: HGSIM 0x10098e39
void __cdecl Sim_10098e39(void)
{
}

// FUNCTION: HGSIM 0x10098e3e
int __cdecl Sim_10098e3e(void)
{
    return -1;
}

// FUNCTION: HGSIM 0x1009ca28
int __cdecl Sim_1009ca28(void)
{
    return 0x400;
}

// FUNCTION: HGSIM 0x1009e5b9
uint32_t __cdecl Sim_1009e5b9(void)
{
    return G32(0x1017D060);
}
