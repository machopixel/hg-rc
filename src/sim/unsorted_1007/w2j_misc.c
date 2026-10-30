#include "w2j.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x10072e35
void __cdecl Sim_10072e35(void *obj)
{
    uint32_t first = (uint32_t)(uintptr_t)obj + 0xc51;
    uint32_t second = first + 0x940;
    uint32_t third = second + 0x3fc;
    uint32_t p;
    int i;

    U32(obj, 0x84) = first;
    U32(obj, 0x88) = second;
    U32(obj, 0x8c) = third;
    p = U32(obj, 0x84);
    for (i = 0; i < 8; i++) {
        U32(p, 0) = 0xffffffffu;
        U32(p, 4) = 0xffffffffu;
        U32(p, 8) = 0xffffffffu;
        U32(p, 0x10) = 0;
        U32(p, 0x14) = 0;
        if (G32(0x1015ec58) == 0 || G32(0x1014c99c) != 0xffffffffu)
            U32(p, 0xc) = 0;
        U32(p, 0x18) = 0xffffffffu;
        U32(p, 0x1c) = 0;
        U32(p, 0x2c) = 0;
        U32(p, 0x30) = 0xffffffffu;
        U32(p, 0x34) = 0;
        U32(p, 0x104) = 0;
        U32(p, 0x20) = 0xffffffffu;
        U32(p, 0x38) = 0;
        p += 0x128;
    }
    p = U32(obj, 0x8c);
    for (i = 0; i < 0x24; i++) {
        U16(p, 0) = 0xffff;
        U16(p, 2) = 0;
        U16(p, 8) = 0;
        U16(p, 0xa) = 0;
        U32(p, 0xc) = 0;
        U32(p, 0x10) = 0;
        U32(p, 0x14) = 0;
        p += 0x24;
    }
}

// FUNCTION: HGSIM 0x100765f6
uint32_t __cdecl Sim_100765f6(const void *obj)
{
    uint32_t result = 0;

    if (S32(obj, 0xc) > 0)
        result = U32(U32(obj, 8), (uint32_t)S32(obj, 0xc) * 4u - 4u);
    return result;
}

// FUNCTION: HGSIM 0x10076667
void __cdecl Sim_10076667(void *obj)
{
    U32(obj, 0xc) = 0;
}
