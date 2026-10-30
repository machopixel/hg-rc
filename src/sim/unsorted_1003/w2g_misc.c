#include "w2g.h"
#include "../simmem.h"

// FUNCTION: HGSIM 0x100332e9
void __cdecl Sim_100332e9(int unused, int value)
{
    (void)unused;
    *SIM_PTR(uint32_t, 0x1014CB34) = value == 0;
}

// FUNCTION: HGSIM 0x1003c860
void __cdecl Sim_1003c860(uint8_t *buf, uint32_t count, uint32_t seed)
{
    uint8_t k1 = (uint8_t)(seed * 0x29);
    uint8_t k2 = (uint8_t)(seed / 0x46 + 0x35);
    uint8_t n = (uint8_t)(seed % 0x46);

    while (count-- != 0) {
        uint8_t b = *buf;

        b = (uint8_t)(b - k2);
        b ^= k1;
        if (b & 1)
            b = (uint8_t)(b - 0x6c);
        *buf++ = b;
        k1 = (uint8_t)(k1 + 0x29);
        n = (uint8_t)(n + 1);
        if (n == 0x46) {
            n = 0;
            k2 = (uint8_t)(k2 + 1);
        }
    }
}

// FUNCTION: HGSIM 0x1003cba3
int32_t __cdecl Sim_1003cba3(int unused)
{
    (void)unused;
    return *SIM_PTR(int32_t, 0x109B9E08);
}
