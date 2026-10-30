#include "w2f.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x10010804
void __cdecl Sim_10010804(int n)
{
    if (n == 0)
        return;
    U32(GBYTES(0x1047383a + G32(0x1013bb40) * 0x4fc3a + (uint32_t)n * 0x13f), 2) = 0x10000;
}

// FUNCTION: HGSIM 0x10013902
uint32_t __cdecl Sim_10013902(uint32_t a, uint16_t b, uint16_t c)
{
    (void)b;
    (void)c;
    return a & 0xFFFF0000u;
}

// FUNCTION: HGSIM 0x10013e3c
int __cdecl Sim_10013e3c(void *obj, int unused)
{
    uint32_t v = (uint32_t)U16(obj, 0x1a6) | ((uint32_t)U16(obj, 0x1aa) << 16);

    (void)unused;
    U32(obj, 0x180) = v;
    if (U16(obj, 0x198) == 0)
        U16(obj, 0x198) = 1;
    return 1;
}

// FUNCTION: HGSIM 0x10016a4a
int __cdecl Sim_10016a4a(int v)
{
    if (v == 1)
        return 2;
    if (v == 2)
        return 1;
    return 0;
}

// FUNCTION: HGSIM 0x1001f4e4
int __cdecl Sim_1001f4e4(void)
{
    return 0;
}
