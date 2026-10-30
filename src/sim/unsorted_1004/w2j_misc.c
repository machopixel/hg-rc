#include "w2j.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x100405e6
void __cdecl Sim_100405e6(const void *obj)
{
    uint32_t entry;

    if (G32(0x1017b8c8) == U8(obj, 4))
        return;
    entry = PTRTAB(U8(obj, 2));
    if (entry == 0)
        return;
    if (U32(entry, 0x14) != 3)
        return;
    U32(entry, 0xc) = U8(obj, 4);
    U32(entry, 0x18) = 0;
}

// FUNCTION: HGSIM 0x100449fc
int __cdecl Sim_100449fc(void *dst, const void *src)
{
    if (src == 0 || dst == 0)
        return 0;
    if (U32(src, 0x18) == 0)
        return 0;
    U32(dst, 0) = U32(src, 0);
    U32(dst, 4) = U32(src, 4);
    U32(dst, 8) = U32(src, 8);
    U32(dst, 0xc) = U32(src, 0xc);
    U32(dst, 0x10) = U32(src, 0x10);
    U32(dst, 0x14) = U32(src, 0x14);
    return 1;
}

// FUNCTION: HGSIM 0x10047af2
int32_t __cdecl Sim_10047af2(void *p)
{
    int32_t distance = (int32_t)(U32(p, 4) - U32(p, 8));

    if (distance != 0) {
        int32_t passed = (int32_t)(G32(0x1015dc78) - U32(p, 0));
        int32_t step = (int32_t)((uint32_t)distance * (uint32_t)passed) / S32(p, 0xc);

        if ((distance < 0 && step <= distance) || (distance > 0 && step >= distance))
            U32(p, 8) = U32(p, 4);
        else
            U32(p, 8) = U32(p, 8) + (uint32_t)step;
    }
    U32(p, 0) = G32(0x1015dc78);
    return S32(p, 8);
}

// FUNCTION: HGSIM 0x1004bc5b
int __cdecl Sim_1004bc5b(void)
{
    return 3;
}

// FUNCTION: HGSIM 0x1004d27a
int __cdecl Sim_1004d27a(int value)
{
    return value >> 16;
}

// FUNCTION: HGSIM 0x1004e1a1
void __cdecl Sim_1004e1a1(void)
{
    G32(0x1015f340) = 0;
    GBYTES(0x1015f230)[G32(0x1015f340)] = 0;
    G32(0x1015f348) = 8;
}
