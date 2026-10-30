#include "w5x.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x100c6e87
void __cdecl Sim_100c6e87(void *p)
{
    if (U32(p, 0) != 0)
        SIM_CALL_100fd760(U32(p, 0));
    SIM_CALL_100fefe0(p);
}

// FUNCTION: HGSIM 0x100c81b5
void __cdecl Sim_100c81b5(void *obj)
{
    if (S16(obj, 4) != 0) {
        U32(obj, 0xf8) = G32(0x109e89d8);
        SIM_CALL_1002c8cb(U32(obj, 0xec), 0, 0, 0);
    }
}

// FUNCTION: HGSIM 0x100cbe68
void __cdecl Sim_100cbe68(uint32_t value)
{
    SIM_CALL_100ca670(PTRTAB(G32(0x1017b8c8)), value, 0x40008);
}
