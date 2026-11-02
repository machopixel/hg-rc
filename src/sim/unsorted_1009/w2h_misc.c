#include "w2h.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x10098a80
void __cdecl Sim_10098a80(void)
{
    G32(0x1014bb94) = G32(0x101c47b0);
    G32(0x1014bbac) = G32(0x101c4750);
    G32(0x1014bbc0) = G32(0x101c4f64);
    G32(0x1014bbd4) = G32(0x101c4db8);
    G32(0x1014bbe8) = G32(0x101c510c);
    G32(0x1014bbfc) = G32(0x101c4fd8);
    G32(0x1014b9e8) = G32(0x101c5124);
    G32(0x1014b9ec) = G32(0x101c4698);
}

// FUNCTION: HGSIM 0x10098e54
int32_t __cdecl Sim_10098e54(int32_t value)
{
    return value;
}
