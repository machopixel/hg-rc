#include "w2g.h"
#include "../simmem.h"
#include "../unsorted_1003/w2g.h"

#define U16(p, off) (*(uint16_t *)((uint8_t *)(p) + (off)))
#define U32(p, off) (*(uint32_t *)((uint8_t *)(p) + (off)))
#define PTR(v) ((uint8_t *)(uintptr_t)(v))

// FUNCTION: HGSIM 0x100d1a60
int32_t __cdecl Sim_100d1a60(uint32_t code)
{
    switch (code) {
    case 0x2B:
        return *SIM_PTR(int32_t, 0x10144FA0);
    case 0x54:
        return *SIM_PTR(int32_t, 0x101452D0);
    case 0x58:
        return *SIM_PTR(int32_t, 0x1015F1D8);
    case 0xC4:
        return Sim_1003cba3(0);
    case 0xC5:
        return *SIM_PTR(int32_t, 0x1014CB1C) == 1;
    }
    return 0;
}

// FUNCTION: HGSIM 0x100d2c86
void __cdecl Sim_100d2c86(int unused, int value)
{
    (void)unused;
    *SIM_PTR(uint32_t, 0x10152418) = value != 0 ? 1 : 2;
}

// FUNCTION: HGSIM 0x100d3a67
int32_t __cdecl Sim_100d3a67(void)
{
    return *SIM_PTR(int32_t, 0x1017E65C);
}

// FUNCTION: HGSIM 0x100d635e
void __cdecl Sim_100d635e(uint8_t *obj, int index, int value)
{
    if (value >= 0 && value < 3) {
        uint8_t *entry = PTR(U32(obj, 0x84) + (uint32_t)index * 0x128u);

        U32(entry, 0xC) = (uint32_t)value;
    }
}

// FUNCTION: HGSIM 0x100d67af
void __cdecl Sim_100d67af(uint8_t *obj)
{
    int32_t index;
    int32_t value;

    U16(obj, 0x560) = (uint16_t)(U16(obj, 0x560) ^ 0x4000);
    index = (int32_t)U32(obj, 0x4FC);
    value = (int32_t)U32(PTR(U32(obj, 0x84) + (uint32_t)index * 0x128u), 0xC);
    index = index + 1;
    if (U32(PTR(U32(obj, 0x84) + (uint32_t)index * 0x128u), 0) == 0xFFFFFFFFu)
        index = 0;
    Sim_100d635e(obj, index, value);
}

// FUNCTION: HGSIM 0x100da428
void __cdecl Sim_100da428(void)
{
}
