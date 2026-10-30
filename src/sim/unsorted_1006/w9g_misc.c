#include "w9g.h"
#include "../util/access.h"
#include "../util/util.h"
#include "../unsorted_1006/w2l.h"
#include "../unsorted_1006/w8w.h"
#include "../unsorted_100a/w5b.h"
#include "../unsorted_100a/w5x.h"

// FUNCTION: HGSIM 0x100622f0
void __cdecl Sim_100622f0(void *obj, uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4, uint32_t a5,
                          uint32_t a6)
{
    (void)a2;
    if (G32(0x10144fa0) == 0)
        return;
    if (U32(obj, 0xc4d) != 0)
        Sim_10062956();
    if (G32(0x10176938) != 0)
        Sim_10065131();
    if (G32(0x10144fac) != 0)
        W9G6_CALL_10064364(a1, a3);
    if (G32(0x10144fa4) != 0) {
        W9G6_CALL_10062b7c(obj, a4, a5, a6);
        if (G32(0x10176950) != 0)
            Sim_10062abd(obj);
    }
    if (G32(0x10144fa8) != 0)
        W9G6_CALL_10063529(obj);
}

// FUNCTION: HGSIM 0x10062956
void __cdecl Sim_10062956(void)
{
    uint8_t *block = BYTES(U32(PTRTAB(G32(0x1017b8c8)), 0x28));
    uint32_t level = U32(block, 0xc4d);
    int32_t i;

    switch (level) {
    case 1:
        G32(0x10165be0) = 0xa;
        G32(0x10165be4) = 2;
        break;
    case 2:
        G32(0x10165be0) = 0x32;
        G32(0x10165be4) = 4;
        break;
    case 3:
        G32(0x10165be0) = 0x64;
        G32(0x10165be4) = 0xa;
        break;
    default:
        return;
    }
    if (Sim_1009c929(0x64) < GS32(0x10165be0)) {
        for (i = 0; i < GS32(0x10165be4); i++)
            U32(GBYTES(0x10165bfc), (uint32_t)Sim_1009c929(0xa) * 0x18u) = 1;
    }
    Sim_10062780();
}

// FUNCTION: HGSIM 0x1006ca00
int __cdecl Sim_1006ca00(void)
{
    int on;

    if (G32(0x10179ca8) == 0)
        Sim_100aa351((void *)(uintptr_t)U32(PTRTAB(G32(0x1017b8c8)), 0x48));
    else
        Sim_100aa39f((void *)(uintptr_t)U32(PTRTAB(G32(0x1017b8c8)), 0x48));
    on = G32(0x10179ca8) == 0;
    G32(0x10179ca8) = (uint32_t)on;
    return on;
}
