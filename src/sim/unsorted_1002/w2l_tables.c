#include "w2l.h"
#include "../util/access.h"

#define ENTRY(i) SIM_PTR(uint8_t, 0x109eeb04u + (uint32_t)(i) * 0x30)

// FUNCTION: HGSIM 0x10020456
void __cdecl Sim_10020456(int number, int value)
{
    int32_t n = (int32_t)((uint32_t)number - 1u);

    if (value == 0xffff)
        value = 0;
    else if (value < 9)
        value = value + 1;
    else
        value = value - 1;
    if (n < 0 || n >= 5 || value < 0 || value >= 0x14 || n == 3)
        return;
    *SIM_PTR(s32a, 0x10133700u + (uint32_t)n * 8) = value;
}

// FUNCTION: HGSIM 0x100272a0
uint32_t __cdecl Sim_100272a0(int index)
{
    if (index < 0 || index >= 10)
        return 0;
    return *SIM_PTR(u32a, 0x109f1484u + (uint32_t)index * 8);
}

// FUNCTION: HGSIM 0x10028973
void __cdecl Sim_10028973(void *p, uint32_t value)
{
    U32(p, 0xc) = value;
}

// FUNCTION: HGSIM 0x10028b50
int __cdecl Sim_10028b50(void)
{
    int i;

    for (i = 0; i < 0x80 && U32(ENTRY(i), 0) != 0; i++)
        ;
    return i == 0x80;
}

// FUNCTION: HGSIM 0x1002997e
int __cdecl Sim_1002997e(int key)
{
    int i;

    for (i = 0; i < 0x80; i++) {
        if (S32(ENTRY(i), 0) == key)
            return i;
    }
    return -1;
}
