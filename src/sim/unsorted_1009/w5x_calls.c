#include "w5x.h"
#include "../unsorted_1003/fns.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x10098811
void __cdecl Sim_10098811(int unused)
{
    (void)unused;
    SIM_CALL_10090d15(-1);
}

// FUNCTION: HGSIM 0x100989a1
void __cdecl Sim_100989a1(int unused, int value)
{
    (void)unused;
    Sim_100332a3(0x800, value);
    SIM_CALL_100a96d8(value);
    G32(0x1014c9fc) = (uint32_t)value;
    U32(G32(0x1015d1b8), 0x18) = (uint32_t)value;
}
