#include "w2h.h"
#include "../unsorted_1003/w2h.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x1004b7b9
int __cdecl Sim_1004b7b9(void)
{
    return Sim_1003830e(0, 0x100, GBYTES(0x109b8c00));
}

// FUNCTION: HGSIM 0x1004de34
void __cdecl Sim_1004de34(int32_t index, int32_t r, int32_t g, int32_t b)
{
    int32_t limit = (int32_t)((1u << (G32(0x1015f22c) & 31)) - 1u);
    uint8_t *dst = BYTES(G32(0x1015f360) + (uint32_t)index * 3);

    *dst = (uint8_t)((int32_t)((uint32_t)r * 0xff) / limit);
    dst++;
    *dst = (uint8_t)((int32_t)((uint32_t)g * 0xff) / limit);
    dst++;
    *dst = (uint8_t)((int32_t)((uint32_t)b * 0xff) / limit);
}

// FUNCTION: HGSIM 0x1004ecb8
void __cdecl Sim_1004ecb8(void)
{
    int32_t count = S32(G32(0x109e3da4), 0xc);
    int32_t i;

    if (G32(0x1013b9a8) != 0)
        return;
    for (i = 0; i < count; i++) {
        uint8_t rgb[3];

        Sim_1003830e(i, 1, rgb);
        Sim_1004de34(i, rgb[0], rgb[1], rgb[2]);
    }
    G32(0x1013b9a8) = 1;
}
