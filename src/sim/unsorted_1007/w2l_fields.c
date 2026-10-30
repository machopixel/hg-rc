#include "w2l.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x10073027
uint32_t __cdecl Sim_10073027(const void *obj)
{
    return U32(U32(obj, 0x28), 0x508);
}

// FUNCTION: HGSIM 0x10076676
int __cdecl Sim_10076676(const void *p)
{
    return U32(p, 0xc) == 0;
}

// FUNCTION: HGSIM 0x1007dc1f
void __cdecl Sim_1007dc1f(void)
{
    G32(0x10179d38) = G32(0x101c49dc);
    G32(0x10179d3c) = G32(0x101c49e0);
    G32(0x10179d50) = G32(0x101c459c);
    G32(0x10179d54) = G32(0x101c45a0);
    G32(0x10179d40) = G32(0x101c5138);
    G32(0x10179d44) = G32(0x101c513c);
    G32(0x10179d48) = G32(0x101c5144);
    G32(0x10179d58) = G32(0x101c47dc);
    G32(0x10179d5c) = G32(0x101c47e0);
    G32(0x10179d60) = G32(0x101c47e8);
}
