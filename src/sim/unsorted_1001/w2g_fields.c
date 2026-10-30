#include "w2g.h"
#include "../simmem.h"

#define U16(p, off) (*(uint16_t *)((uint8_t *)(p) + (off)))
#define U32(p, off) (*(uint32_t *)((uint8_t *)(p) + (off)))
#define PTR(v) ((uint8_t *)(uintptr_t)(v))

// FUNCTION: HGSIM 0x10010145
void __cdecl Sim_10010145(const void *a, const void *b)
{
    uint32_t *p;

    if (a == 0 || b == 0)
        return;
    p = (uint32_t *)PTR(U32(b, 8));
    if (p == 0)
        return;
    *p = *SIM_PTR(uint32_t, 0x10149A90);
}

// FUNCTION: HGSIM 0x10013e20
int __cdecl Sim_10013e20(void *obj, int unused)
{
    (void)unused;
    U16(obj, 0x198) = 0;
    return 1;
}

// FUNCTION: HGSIM 0x10016c73
uint32_t __cdecl Sim_10016c73(int a, int unused, uint32_t c)
{
    uint32_t base = (uint32_t)a * 0x4FC3Au + 0x10473800u;
    int32_t row = (int16_t)*SIM_PTR(uint16_t, 0x104727A0u + (uint32_t)a * 4u);
    uint32_t addr = base + (uint32_t)row * 0x13Fu + 0x3Au + c * 2u + 0xEFu;

    (void)unused;
    return (c & 0xFFFF0000u) | *SIM_PTR(uint16_t, addr);
}

// FUNCTION: HGSIM 0x1001c618
int __cdecl Sim_1001c618(uint32_t v)
{
    /* The original is a switch on (v & 0xff) - 4 through a 22-entry byte table. */
    switch (v & 0xFF) {
    case 4: case 5: case 6: case 7: case 8:
    case 13: case 17: case 21: case 23: case 24: case 25:
        return 1;
    }
    return 0;
}
