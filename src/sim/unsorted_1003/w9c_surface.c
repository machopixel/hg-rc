#include "w9c.h"
#include "../unsorted_100f/w2e.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x10035aec
void __cdecl Sim_10035aec(void)
{
    uint8_t *base = BYTES(U32(G32(0x1015e698), 0));
    int32_t i;

    for (i = 0; i < S32(G32(0x1015e698), 8) / 4; i++) {
        uint32_t row = (U32(G32(0x1015e698), 4) + 1) * (uint32_t)i;

        Mem_Move(base + row + 0x14, base + row, 10);
    }
}
