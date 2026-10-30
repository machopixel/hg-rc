#include "w9c.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x100b5bcb
void __cdecl Sim_100b5bcb(uint32_t unused, uint32_t value)
{
    (void)unused;
    G32(0x1015084c) = 1;
    U32(G32(0x1015d1b8), 0x24) = value;
}
