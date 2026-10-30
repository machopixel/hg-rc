#include "w5c.h"
#include "../unsorted_1003/fns.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x100988fc
void __cdecl Sim_100988fc(uint32_t unused, uint32_t value)
{
    (void)unused;
    Sim_100332a3(0x100, (int)value);
    Sim_100332a3(0x200, (int)value);
    Sim_100332a3(0x800, (int)value);
    SIM_CALL_100a96d8(value);
    G32(0x1014c9fc) = value;
    U32(G32(0x1015d1b8), 0x14) = value;
    U32(G32(0x1015d1b8), 0x18) = value;
}

// FUNCTION: HGSIM 0x1009964b
uint32_t __cdecl Sim_1009964b(void)
{
    if (G32(0x1017cc3c) == 0)
        SIM_CALL_10099510();
    return G32(0x1017cc3c);
}
