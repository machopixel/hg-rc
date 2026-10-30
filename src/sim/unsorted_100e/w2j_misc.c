#include "w2j.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x100e122a
void __cdecl Sim_100e122a(void)
{
}

// FUNCTION: HGSIM 0x100e2740
void __cdecl Sim_100e2740(uint32_t a, uint32_t b)
{
    G32(0x1017e830) = a;
    G32(0x1017e854) = b;
}
