#include "w9e.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x1001dd66
void __cdecl Sim_1001dd66(void)
{
    uint8_t *obj = BYTES(PTRTAB(G32(0x1017b8c8)));

    if (obj != 0)
        (void)*(volatile u32a *)(obj + 0x48); /* the original loads it into a dead local */
}
