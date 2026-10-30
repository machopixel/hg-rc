#include "w5b.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x100b77b3
void __cdecl Sim_100b77b3(void *block)
{
    if (block != 0 && U32(block, 4) != 0) {
        int i;

        for (i = 0; i < S32(block, 0); i++) {
            SIM_FREE(BYTES(U32(U32(block, 4), (uint32_t)i * 8u)));
            SIM_FREE(BYTES(U32(U32(block, 4), (uint32_t)i * 8u + 4u)));
        }
        SIM_FREE(BYTES(U32(block, 4)));
    }
    if (block != 0)
        SIM_FREE(block);
}
