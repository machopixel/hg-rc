#include "w2h.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x10007020
void __cdecl Sim_10007020(void)
{
    int group;
    int i;

    for (group = 0; group < 0x200; group++) {
        for (i = 0; i < 0x20; i++) {
            uint8_t *rec = GBYTES(0x10a51ac0 + (uint32_t)group * 0x300 + (uint32_t)i * 0x18);

            U8(rec, 0) = 0;
            U16(rec, 0x10) = 0;
            U32(rec, 0x14) = 0;
        }
    }
    for (i = 0; i < 0x200; i++) {
        uint8_t *rec = GBYTES(0x10ac1b00 + (uint32_t)i * 0xe);

        U16(rec, 0) = 0xffff;
        U16(rec, 2) = 0;
        U16(rec, 4) = 0;
        U16(rec, 6) = 0;
        U16(rec, 8) = 0xfffe;
        U32(rec, 0xa) = 0xffffffff;
    }
    G32(0x1012da30) = 0;
    G32(0x1015cfe0) = 0;
    G32(0x1015cfe4) = 0;
    G32(0x1015cfe8) = 0;
}
