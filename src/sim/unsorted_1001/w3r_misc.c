#include "w3r.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x10019670
void __cdecl Sim_10019670(void)
{
    uint32_t gear = U32(PTRTAB(GS32(0x1017b8c8)), 0x28);
    int i;

    for (i = 0; i < 8; i++) {
        uint32_t record = U32(gear, 0x84) + (uint32_t)i * 0x128u;

        (void)*(volatile u32a *)BYTES(record + 4);
    }
}
