#include "w2f.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x1007b02d
void __cdecl Sim_1007b02d(void)
{
    int32_t best = (int32_t)0xffff8001;
    uint32_t block;
    uint32_t sum = 0;
    int has2 = 0;
    int32_t i;

    block = 0x10473800 + G32(0x1013bb40) * 0x4fc3a;
    if (G32(0x1015ec58) == 0 || U8(G32(0x1017b8c4), 0xa) != 0)
        return;
    for (i = 0; i < S32(GBYTES(block), 0); i++) {
        uint32_t rec = 0x1047383a + G32(0x1013bb40) * 0x4fc3a + (uint32_t)i * 0x13f;

        sum += U8(GBYTES(rec), 0x74);
        if (U8(GBYTES(rec), 0x74) != 0 && U8(GBYTES(rec), 1) == 2)
            has2 = 1;
    }
    if (has2 && sum == 1) {
        int32_t pick = 0;
        int32_t row, col;

        for (row = 0; row < 8; row++) {
            int32_t score = 0;

            for (col = 0; col < 8; col++) {
                int32_t v = U16(GBYTES(0x1016552c), row * 16 + col * 2);

                if (row == col)
                    score -= v;
                else
                    score += v;
            }
            if (score > best) {
                best = score;
                pick = row;
            } else if (score == best) {
                pick = -2;
            }
        }
        GS32(0x101655ac) = pick;
    }
}

// FUNCTION: HGSIM 0x1007cbb9
int __cdecl Sim_1007cbb9(void)
{
    return 1;
}

// FUNCTION: HGSIM 0x1007f0b4
int __cdecl Sim_1007f0b4(void)
{
    U32(G32(0x1015e698), 0) = G32(0x1015e9a0);
    return 0;
}
