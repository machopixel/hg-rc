#include "w2i.h"
#include "../util/access.h"

/* Address of a piece of the DLL's data, as stored in its own tables. */
#define DLL_ADDR(a) ((uint32_t)(uintptr_t)GBYTES(a))

// FUNCTION: HGSIM 0x1002ba28
void __cdecl Sim_1002ba28(int which)
{
    uint32_t set = (uint32_t)which + G32(0x101C64F0) * 2u;
    int i, k;

    G32(0x1017E5E4) = G32(0x10151D58 + G32(0x101C64F0) * 4u);
    for (i = 0; i < 0x1e; i++) {
        uint32_t src = 0x101355C0u + set * 0x258u + (uint32_t)i * 0x14u;
        uint32_t dst = 0x109E8660u + (uint32_t)i * 0x14u;

        for (k = 0; k < 5; k++)
            G32(dst + (uint32_t)k * 4u) = G32(src + (uint32_t)k * 4u);
    }
    for (i = 0; i < 0x77; i++)
        G32(0x109E8480u + (uint32_t)i * 4u) = DLL_ADDR(0x101362E0u + set * 0x770u + (uint32_t)i * 16u);
    if (G32(0x1014CB38) != 0)
        U8(G32(0x109E85C4), 7) = 0x47;
    for (i = 0; i < 0x1e; i++) {
        uint32_t src = 0x10135F20u + set * 0xf0u + (uint32_t)i * 8u;
        uint32_t lo = G32(src);
        uint32_t hi = G32(src + 4);

        G32(0x109E8240u + (uint32_t)i * 8u) = lo;
        G32(0x109E8244u + (uint32_t)i * 8u) = hi;
    }
    G32(0x101508A4) = 7;
    G32(0x101508AC) = 8;
    G32(0x101508B4) = 9;
    G32(0x101508BC) = 0xa;
    G32(0x101508C4) = 0xb;
    G32(0x101508CC) = 0xc;
    G32(0x101508D4) = 0xd;
    G32(0x109B98C0) = DLL_ADDR(0x101380A0u + G32(0x101C64F0) * 0x12u);
    G32(0x109B9900) = DLL_ADDR(0x101380A9u + G32(0x101C64F0) * 0x12u);
    if (which == 0) {
        G32(0x10145290) = 0;
        G32(0x10145294) = 0;
        G32(0x10145298) = 0xeb;
        G32(0x1014529C) = 0xbe;
    } else {
        G32(0x10145290) = 0x17e;
        G32(0x10145294) = 0;
        G32(0x10145298) = 0x102;
        G32(0x1014529C) = 0xbe;
    }
}
