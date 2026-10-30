#include "w2j.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x100905e8
void __cdecl Sim_100905e8(void)
{
    int i;

    for (i = 0; i < 0x10; i++)
        SIM_PTR(u32a, 0x10217bc0)[i] = (uint32_t)(i * 10 * i * i);
    for (i = 0; i < 0xf; i++)
        GBYTES(0x10215b00)[i] = (uint8_t)(0xfe - i);
    GBYTES(0x10215b00)[0xf] = 0xf0;
    for (i = 0; i < 0x10; i++)
        GBYTES(0x10215bb0)[i] = (uint8_t)((0xf - i) / 4);
    for (i = 0; i < 4; i++)
        GBYTES(0x1036f050)[i] = (uint8_t)(0x1f - i);
    for (i = 0; i < 4; i++)
        GBYTES(0x10215b18)[i] = (uint8_t)(0x17 - i);
    for (i = 0; i < 4; i++)
        GBYTES(0x10215bc0)[i] = (uint8_t)(0x27 - i);
    for (i = 0; i < 0x10; i += 2) {
        SIM_PTR(u32a, 0x10215b20)[i] = (uint32_t)(0x5f - i / 2);
        SIM_PTR(u32a, 0x10215b24)[i] = (uint32_t)(0x5f - i / 2);
    }
    for (i = 0; i < 0x10; i += 2) {
        SIM_PTR(u32a, 0x10215b60)[i] = (uint32_t)(i + 0x78);
        SIM_PTR(u32a, 0x10215b64)[i] = (uint32_t)(i + 0x78);
    }
    for (i = 0; i < 0x10; i++)
        GBYTES(0x10215bd0)[i] = (uint8_t)(0xf7 - i / 2);
    for (i = 0; i < 0x10; i++)
        GBYTES(0x10217b20)[i] = (uint8_t)((0xf - i) / 4);
    for (i = 0; i < 0x10; i++)
        GBYTES(0x101d09c0)[i] = (uint8_t)(0x1f - i / 2);
    for (i = 0; i < 0x10; i++)
        GBYTES(0x10217b90)[i] = (uint8_t)(i + 0xf0);
    for (i = 0; i < 0xf; i++)
        GBYTES(0x10215af0)[i] = (uint8_t)(0xfe - i);
    GBYTES(0x10215af0)[i] = 0xf0;
    for (i = 0; i < 8; i++)
        GBYTES(0x10215b10)[i] = (uint8_t)(0xde - i);
}
