#include "w2e.h"
#include "../unsorted_100f/w2e.h"
#include "../util/access.h"

#define NEG_16384 0xc6800000u
#define POS_16384 0x46800000u

// FUNCTION: HGSIM 0x100d7fc0
void __cdecl Sim_100d7fc0(void)
{
    static const uint32_t pair_values[16] = {
        0xfe, 0xfd, 0xfc, 0xfb, 0xfa, 0xf9, 0xf8, 0xf7, 0xef, 0xee, 0xed, 0xec, 0xeb, 0xea, 0xe9, 0xe8,
    };
    static const uint32_t third[3] = {NEG_16384, 0, POS_16384};
    int n, i, k;

    for (k = 0; k < 16; k++) {
        G32(0x10180980 + (uint32_t)k * 8) = pair_values[k];
        G32(0x10180984 + (uint32_t)k * 8) = pair_values[k];
    }
    for (n = 0; n < 9; n++) {
        uint8_t *block = GBYTES(0x10180a00) + (uint32_t)n * 0x80cu;

        U32(block, 0x754) = 0;
        U32(block, 0x750) = 0x2d;
        U32(block, 0xc) = third[n % 3];
        U32(block, 0x10) = 0x461c4000;
        U32(block, 0x14) = third[n / 3];
        U32(block, 0xe4) = 0x469c4000;
        U32(block, 0xe8) = 0;
        U32(block, 0xec) = 0x469c4000;
        U32(block, 0xf4) = 0;
        U32(block, 0xf8) = 0;
        for (i = 0; i < 0x2d; i++) {
            uint8_t *entry = block + 0xfc + (uint32_t)i * 0x24u;

            U32(block, 0x758 + (uint32_t)i * 4) = (uint32_t)i;
            U32(entry, 4) = 0xFFFFFFFFu;
            U32(entry, 0) = (uint32_t)i;
            U32(entry, 0x20) = (uint32_t)(uintptr_t)block;
        }
        Mem_Move(block + 0x18, GBYTES(0x10180980), 0x80);
    }
}
