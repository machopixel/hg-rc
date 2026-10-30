#include "w2l.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x10010124
void __cdecl Sim_10010124(const void *src, void *dst)
{
    if (src == 0 || dst == 0)
        return;
    U32(dst, 0xc) = U32(src, 8);
}

// FUNCTION: HGSIM 0x100128dd
void __cdecl Sim_100128dd(int group, int low, unsigned value)
{
    int i;

    for (i = 0; i < GS32(0x1015dc64); i++) {
        uint32_t rec = PTRTAB(i);

        if (S32(rec, 8) == group && (U16(rec, 0x1aa) & 0xf000) == 0 && (U16(rec, 0x1aa) & 0x200) != 0 &&
            (int)(U16(rec, 0x1aa) & 0xff) == low)
            U16(rec, 0x1aa) = (uint16_t)(value | 0x200);
    }
}

// FUNCTION: HGSIM 0x10013040
uint32_t __cdecl Sim_10013040(uint32_t value, uint16_t unused1, uint16_t unused2)
{
    (void)unused1;
    (void)unused2;
    return value & 0xffff0000u;
}

// FUNCTION: HGSIM 0x10017aa9
void __cdecl Sim_10017aa9(void *obj)
{
    uint32_t p = U32(obj, 0x28);

    U32(obj, 0x23e) = U32(p, 0x588);
    U32(p, 0x58c) = 2;
}

// FUNCTION: HGSIM 0x1001db6a
void __cdecl Sim_1001db6a(void)
{
}
