#include "w2f.h"
#include "../simmem.h"
#include "../util/access.h"
#include "../unsorted_1005/w2f.h"

// FUNCTION: HGSIM 0x10098849
int __cdecl Sim_10098849(int unused)
{
    (void)unused;
    /* through the original entry: hooks_view.c forces the option on there */
    return SIM_FN(int(__cdecl *)(int), 0x100585c4)(-1);
}

// FUNCTION: HGSIM 0x1009885e
int __cdecl Sim_1009885e(int unused, int b)
{
    (void)unused;
    SIM_FN(int(__cdecl *)(int), 0x100585c4)(b);
    U32(G32(0x1015d1b8), 0x2c) = (uint32_t)b;
    return b;
}

// FUNCTION: HGSIM 0x1009b2eb
int __cdecl Sim_1009b2eb(void)
{
    return G32(0x1015d1ac) == 4;
}
