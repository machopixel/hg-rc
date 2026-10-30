#include "w9h.h"
#include "../render/w2n.h"
#include "../util/access.h"
#include "../unsorted_1004/fns.h"
#include "../unsorted_1005/w3r.h"
#include "../unsorted_100b/w2f.h"

// FUNCTION: HGSIM 0x100e1398
void __cdecl Sim_100e1398(void)
{
    int32_t i, k;
    uint32_t rec, block;

    for (i = 0; i < GS32(0x1015dc64); i++) {
        Sim_1005b98c(i);
        k = Sim_10052be7(i);
        if (k == 0)
            U16(GBYTES(0x1016541a), 0) += 1;
        else if (k == 1)
            U16(GBYTES(0x1016541e), 0) += 1;
        else if (k == 2)
            U16(GBYTES(0x1016541c), 0) += 1;
    }
    for (i = 0; i < GS32(0x1015dc68); i++) {
        k = Sim_10052c02(i);
        if (k == 0)
            U16(GBYTES(0x10165420), 0) += 1;
        else if (k == 1)
            U16(GBYTES(0x10165424), 0) += 1;
        else if (k == 2)
            U16(GBYTES(0x10165422), 0) += 1;
        rec = Sim_100b5386((int)G32(0x109e9ba8 + (uint32_t)i * 0x50));
        if (rec != 0 && U32(rec, 0x8c) != 0) {
            block = U32(rec, 0x8c);
            G32(0x109e9bb4 + (uint32_t)i * 0x50) = (uint32_t)Render_Ftol((double)*(float *)(BYTES(block) + 0x50));
        }
    }
}

// FUNCTION: HGSIM 0x100e6b29
void __cdecl Sim_100e6b29(const uint32_t *src)
{
    volatile uint32_t *dst = SIM_PTR(volatile uint32_t, 0x101534c0);
    int i;

    for (i = 0; i < 0x40; i++)
        dst[i] = src[i];
}
