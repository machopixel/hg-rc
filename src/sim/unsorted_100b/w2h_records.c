#include "w2h.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x100b4073
void __cdecl Sim_100b4073(int32_t i)
{
    uint8_t *rec = GBYTES(0x101a2100 + (uint32_t)i * 0x88);

    U8(rec, 0) = 0;
    U32(rec, 0x10) = 0xffffffffu;
    U32(rec, 0x14) = 0xffffffffu;
    U32(rec, 0x18) = 0;
    U32(rec, 0x1c) = 0;
    U32(rec, 0x20) = 0xffffffffu;
    U32(rec, 0x24) = 0xffffffffu;
    U32(rec, 0x28) = 0;
    U32(rec, 0x2c) = 0;
    U32(rec, 0x38) = 0x3f800000u;
    U32(rec, 0x34) = 0x3f800000u;
    U32(rec, 0x30) = 0x3f800000u;
    U32(rec, 0x44) = 0;
    U32(rec, 0x40) = 0;
    U32(rec, 0x3c) = 0;
    U32(rec, 0x50) = 0;
    U32(rec, 0x4c) = 0;
    U32(rec, 0x48) = 0;
    U32(rec, 0x84) = 0;
}

// FUNCTION: HGSIM 0x100b4149
void __cdecl Sim_100b4149(void)
{
    int32_t i;

    for (i = 0; i < 0x402; i++)
        Sim_100b4073(i);
    G32(0x1017d218) = 0;
    G32(0x1017d21c) = 1;
}

// FUNCTION: HGSIM 0x100b391c
int32_t __cdecl Sim_100b391c(uint32_t value)
{
    int32_t index = -1;

    if (G32(0x1017d21c) == 0)
        Sim_100b4149();
    if (GS32(0x1017d218) < 0x402 && GS32(0x101a1168) < GS32(0x101c42a4)) {
        index = GS32(0x1017d218);
        U32(G32(0x1017d204), G32(0x101a1168) * 4) = (uint32_t)index;
        U32(G32(0x1017d208), G32(0x101a1168) * 4) = value;
        G32(0x1017d218) = G32(0x1017d218) + 1;
        G32(0x101a1168) = G32(0x101a1168) + 1;
    }
    return index;
}

// FUNCTION: HGSIM 0x100b39af
uint32_t __cdecl Sim_100b39af(int32_t i)
{
    return G32(0x101a2128 + (uint32_t)i * 0x88);
}
