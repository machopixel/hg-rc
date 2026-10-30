#include "w9c.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x1003b986
void __cdecl Sim_1003b986(uint32_t key)
{
    int32_t i;

    for (i = 0; i < GS32(0x1015ea80); i++) {
        uint8_t *e = GBYTES(0x109b9e20) + (uint32_t)i * 0x7eu;

        if (U32(e, 0x60) == key) {
            int k;

            for (k = 0; k < 4; k++)
                U32(e, 0x50 + 4 * k) = 0;
            U32(e, 4) = 0xffffffffu;
            U16(e, 0x7c) = 0;
            return;
        }
    }
}
