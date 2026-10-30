#include "w2l.h"
#include "fns.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x10047fe0
void __cdecl Sim_10047fe0(void)
{
    int i;

    for (i = 0; i < 0x100; i++)
        *SIM_PTR(uint8_t, 0x109b9920u + (uint32_t)i) = (uint8_t)i;
    *SIM_PTR(uint8_t, 0x109b992e) = 9;
}

// FUNCTION: HGSIM 0x1004b804
void __cdecl Sim_1004b804(const uint8_t *in, uint8_t *out)
{
    const uint8_t *table = SIM_PTR(uint8_t, 0x109b9080u + (G32(0x1013b98c) << 6));

    out[0] = table[in[0]];
    out[1] = table[in[1]];
    out[2] = table[in[2]];
}

// FUNCTION: HGSIM 0x1004c318
uint32_t __cdecl Sim_1004c318(uint32_t word)
{
    return word & 0xffff;
}

// FUNCTION: HGSIM 0x1004c325
int __cdecl Sim_1004c325(uint32_t word)
{
    int value = (int)Sim_1004c318(word);

    if (value <= 0x3b6)
        return value;
    return (value - 0x3b6) / 0x32 * 10 + 0x3b6;
}

// FUNCTION: HGSIM 0x1004c3dd
int __cdecl Sim_1004c3dd(uint32_t word)
{
    int value = (int)Sim_1004c318(word);

    if (value < 0x258)
        return 0;
    if (value < 0x2ee)
        return 1;
    if (value < 0x3b6)
        return 2;
    return 3;
}

// FUNCTION: HGSIM 0x1004c426
int __cdecl Sim_1004c426(uint32_t word)
{
    int value = (int)Sim_1004c318(word);
    uint32_t base = *SIM_PTR(u32a, 0x10124ec4u + (uint32_t)Sim_1004c30d((int)word) * 0x98u);

    if (value < 0x190)
        return (int)(base + 1u);
    if (value < 0x258)
        return (int)base;
    if (value < 0x2ee)
        return (int)(base - 1u);
    if (value < 0x3b6)
        return (int)(base - 2u);
    return (int)(base - 3u);
}

// FUNCTION: HGSIM 0x1004c95b
int __cdecl Sim_1004c95b(void)
{
    return 1;
}

// FUNCTION: HGSIM 0x1004ce50
int __cdecl Sim_1004ce50(int index)
{
    return (int)*SIM_PTR(u32a, 0x101116b2u + (uint32_t)index * 0xaa);
}

// FUNCTION: HGSIM 0x1004d26d
uint32_t __cdecl Sim_1004d26d(uint32_t word)
{
    return word & 0xffff;
}
