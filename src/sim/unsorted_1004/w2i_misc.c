#include "w2i.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x1004044c
void __cdecl Sim_1004044c(void *p)
{
    uint32_t block;
    int i;

    if (PTRTAB(U8(p, 2)) == 0)
        return;
    if (G32(0x1017B8C8) == U8(p, 2))
        return;
    if (U32(PTRTAB(U8(p, 2)), 0x14) == 3 && U32(PTRTAB(U8(p, 2)), 0xc) == G32(0x1017B8C8))
        return;
    if (!((int32_t)U8(p, 2) < GS32(0x1015DC64)))
        return;

    block = U32(PTRTAB(U8(p, 2)), 0x28);
    for (i = 0; i < 8; i++) {
        if ((U16(p, 3) & 1) != 0) {
            uint32_t rec = U32(block, 0x84) + (uint32_t)i * 0x128u;

            U32(block, 0x4fc) = (uint32_t)i;
            U32(rec, 8) = 2;
            G32(0x101853C0u + (uint32_t)i * 4u) = 1;
        }
        U16(p, 3) = (uint16_t)(U16(p, 3) >> 1);
    }
    Sim_100d63fa((void *)(uintptr_t)block);
}

// FUNCTION: HGSIM 0x10040645
void __cdecl Sim_10040645(void *dst, void *src)
{
    int i;

    U16(dst, 8) = U16(src, 0x198);
    for (i = 0; i < 2; i++)
        U32(dst, i * 4) = U32(src, 0x180 + i * 4);
    U8(dst, 0xa) = U8(src, 0x1a0);
    U16(dst, 0xb) = U16(src, 0x1a2);
    U16(dst, 0xd) = U16(src, 0x1a4);
    U8(dst, 0xf) = U8(src, 0x1a6);
    U16(dst, 0x10) = U16(src, 0x1a8);
    U16(dst, 0x12) = U16(src, 0x1aa);
    U16(dst, 0x14) = U16(src, 0x1ae);
    U32(dst, 0x16) = U32(src, 0x23e);
    U32(dst, 0x1a) = U32(src, 0x242);
}

// FUNCTION: HGSIM 0x10040f2d
void __cdecl Sim_10040f2d(void *p)
{
    U8(G32(0x1015D004), 0x138) = U8(p, 4);
    U8(G32(0x1015D004), 0x137) = U8(p, 2);
}

// FUNCTION: HGSIM 0x1004f0a6
void __cdecl Sim_1004f0a6(void)
{
    int i;

    for (i = 0; i < 0x19; i++)
        G32(0x109B8B60u + (uint32_t)i * 4u) = 0xFFFFFFFFu;
}
