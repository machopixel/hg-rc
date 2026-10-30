#include "w2g.h"
#include "../simmem.h"
#include "../unsorted_1003/fns.h"

#define U8(p, off) (*(uint8_t *)((uint8_t *)(p) + (off)))
#define U16(p, off) (*(uint16_t *)((uint8_t *)(p) + (off)))
#define U32(p, off) (*(uint32_t *)((uint8_t *)(p) + (off)))
#define PTR(v) ((uint8_t *)(uintptr_t)(v))
#define G32(addr) (*SIM_PTR(uint32_t, addr))
#define OBJECT(i) PTR(SIM_PTR(uint32_t, 0x109E8B40)[(i)])
#define ROOT PTR(G32(0x1015D004))
#define FLAG(o) G32(0x109B7D54u + U32((o), 8) * 0x6Cu)

// FUNCTION: HGSIM 0x10041e61
void __cdecl Sim_10041e61(void)
{
    uint32_t mode;
    int32_t i;

    if (G32(0x1015EC58) == 0)
        return;
    U16(OBJECT(G32(0x1017B8C8)), 0x1C) &= 0xB7FF;
    mode = U8(PTR(U32(ROOT, 0x126)), (uint32_t)Sim_1003dbc0(G32(0x1017B8C8)) * 0x1Eu + 0x1C);
    for (i = 0; i < (int32_t)U32(ROOT, 0x12); i++) {
        uint8_t *rec = PTR(U32(ROOT, 0x126)) + (uint32_t)i * 0x1Eu;
        uint8_t *obj = OBJECT(U16(rec, 0x1A));

        if (U8(PTR(G32(0x1017B8C4)), 0xA) != 0)
            FLAG(obj) = U8(rec, 0x1C) == mode ? 0 : 1;
        else
            FLAG(obj) = U32(obj, 4) == G32(0x1017B8C8) ? 0 : 1;
    }
    for (i = 0; i < (int32_t)G32(0x1015DC64); i++) {
        uint8_t *obj = OBJECT(i);

        U32(obj, 0x160) = 0;
        U32(obj, 0x164) = 0;
        if (U32(obj, 0x14) == 3)
            U32(obj, 0x18) = 0;
        if ((mode == 0 && FLAG(obj) == 0) || (mode == 1 && FLAG(obj) == 1))
            U32(obj, 0x168) = G32(0x1015D004) + 0x138;
        else
            U32(obj, 0x168) = G32(0x1015D004) + 0x137;
    }
    G32(0x1013BB40) = U32(OBJECT(G32(0x1017B8C8)), 8);
    if (U8(ROOT, 0x137) != 0 || U8(ROOT, 0x138) != 0)
        U8(PTR(G32(0x1017B8C4)), 8) = 1;
    G32(0x1015EC54) = 1;
}

// FUNCTION: HGSIM 0x1004f1df
int __cdecl Sim_1004f1df(int code)
{
    uint32_t c = (uint32_t)code & 0xFF;

    if (c <= 8)
        return 0;
    if (c >= 10 && c <= 13)
        return 1;
    if (c >= 14 && c <= 17)
        return 2;
    if (c >= 18 && c <= 21)
        return 3;
    return 4;
}
