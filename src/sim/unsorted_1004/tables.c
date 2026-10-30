#include "fns.h"
#include "../simmem.h"

// FUNCTION: HGSIM 0x1004d330
int __cdecl Sim_1004d330(int index)
{
    return *(int *)(SIM_PTR(unsigned char, 0x101116B2) + (uint32_t)index * 0xAA);
}

// FUNCTION: HGSIM 0x10052aaf
int __cdecl Sim_10052aaf(int index)
{
    int value = 0;

    if (index < 0x10)
        value = *(int *)(SIM_PTR(unsigned char, 0x109B7D40) + (uint32_t)index * 0x6C);
    return value;
}

// FUNCTION: HGSIM 0x10052be7
int __cdecl Sim_10052be7(int index)
{
    unsigned char *record = (unsigned char *)(uintptr_t)SIM_PTR(uint32_t, 0x109E8B40)[(uint32_t)index];

    return *(int *)(SIM_PTR(unsigned char, 0x109B7D54) + *(uint32_t *)(record + 8) * 0x6C);
}

// FUNCTION: HGSIM 0x10052c02
int __cdecl Sim_10052c02(int index)
{
    int entry = *(int *)(SIM_PTR(unsigned char, 0x109E9BB0) + (uint32_t)index * 0x50);

    if (entry < 0)
        return 2;
    return SIM_PTR(int, 0x109B8400)[entry];
}
