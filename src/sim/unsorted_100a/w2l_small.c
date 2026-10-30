#include "w2l.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x100a78c3
void __cdecl Sim_100a78c3(uint32_t a, uint32_t b)
{
    G32(0x1017d15c) = a;
    G32(0x1017d160) = b;
}

// FUNCTION: HGSIM 0x100a9012
void __cdecl Sim_100a9012(void)
{
}

// FUNCTION: HGSIM 0x100aa7f1
uint32_t __cdecl Sim_100aa7f1(const void *p)
{
    return U32(p, 4);
}
