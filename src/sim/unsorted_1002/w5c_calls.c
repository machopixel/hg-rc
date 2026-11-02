#include "w5c.h"
#include "../unsorted_100f/w2e.h"
#include "../util/access.h"
#include "../crt/hg_winheap.h"

// FUNCTION: HGSIM 0x10029ab3
void __cdecl Sim_10029ab3(uint32_t key, uint32_t value)
{
    int i;
    int k;

    if (key == 0)
        return;
    for (i = 0; i < 0x80; i++) {
        if (G32(0x109f09e4 + (uint32_t)i * 0x14) == key) {
            for (k = 0; k < 5; k++)
                G32(0x109f09e0 + (uint32_t)i * 0x14 + 4u * (uint32_t)k) = G32(0x1015dc48 + 4u * (uint32_t)k);
            break;
        }
    }
    SIM_CALL_10028fb0(key, value);
}

// FUNCTION: HGSIM 0x1002cde1
int __cdecl Sim_1002cde1(uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t *out)
{
    uint32_t first;
    uint32_t ok = 0;
    uint32_t third;

    SIM_CALL_1002ce45(a, b, c, d, &first, &ok, &third);
    if (ok != 0) {
        *out = third;
        G32(0x1015dd48) = 0;
        G32(0x1015dd40) = G32(0x1015dd48);
        G32(0x1015dd44) = 0x3f800000;
    }
    return (int)ok;
}

// FUNCTION: HGSIM 0x1002fad6
void *__cdecl Sim_1002fad6(void *arg)
{
    uint8_t *block = (uint8_t *)Hg_HeapAlloc(G32(0x1017b8f8), 1, 0x78);

    if (block != 0) {
        Mem_Fill(block, 0, 0x78);
        SIM_CALL_1002fba5(arg, block, block + 4, block + 8, block + 0xc, block + 0x10, block + 0x14);
    }
    return block;
}

// FUNCTION: HGSIM 0x1002fb3d
void __cdecl Sim_1002fb3d(void *obj)
{
    if (obj == 0)
        return;
    if ((U16(obj, 0) & 0x200) != 0)
        return;
    if (U32(obj, 0x98) == 0)
        U32(obj, 0x98) = (uint32_t)(uintptr_t)Sim_1002fad6(obj);
    if (U32(obj, 0x98) == 0)
        return;
    U16(obj, 0) = (uint16_t)(U16(obj, 0) | 0x200);
}
