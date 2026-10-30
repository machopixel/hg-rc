#include "w4u.h"
#include "../win/w4u_api.h"

// FUNCTION: HGSIM 0x1000e62e
void __cdecl Sim_1000e62e(void)
{
    uint32_t i;

    if (G32(0x1015d0a4) == 0)
        return;
    for (i = 0; i < U32(G32(0x1015d0a4), 4); i++) {
        if (U32(G32(0x1015d0a4), 0x54 + 4 * i) != 0)
            U32(G32(0x1015d0a4), 0x54 + 4 * i) = 0;
        W4u_HeapFree(U32(G32(0x1015d0a4), 0x94 + 8 * i));
    }
    W4u_HeapFree(G32(0x1015d0a4));
    G32(0x1015d0a4) = 0;
}
