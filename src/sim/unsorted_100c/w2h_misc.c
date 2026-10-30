#include "w2h.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x100c6fb8
void __cdecl Sim_100c6fb8(void)
{
}

// FUNCTION: HGSIM 0x100ca96a
void __cdecl Sim_100ca96a(void)
{
    uint32_t obj = PTRTAB(G32(0x1017b8c8));

    U32(obj, 0xf0) = U32(obj, 0xf0) | 0x1000;
}

// FUNCTION: HGSIM 0x100cc96c
int __cdecl Sim_100cc96c(void)
{
    return 0x18;
}
