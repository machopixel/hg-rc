#include "w2j.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x1004e4c2
uint32_t __cdecl Sim_1004e4c2(uint32_t prefix, uint32_t byte)
{
    uint32_t slot;
    uint32_t number;

    if (G32(0x1015f350) >= 0x1000)
        return 0xffff;
    slot = (((byte & 0xff) << 8) ^ prefix) % 0x26f5;
    while (U32(G32(0x1015f35c), slot * 4) != 0xffff)
        slot = (slot + 0x7f7) % 0x26f5;
    U32(G32(0x1015f35c), slot * 4) = G32(0x1015f350);
    U8(G32(0x1015f354), G32(0x1015f350)) = (uint8_t)byte;
    U32(G32(0x1015f358), G32(0x1015f350) * 4) = prefix;
    number = G32(0x1015f350);
    G32(0x1015f350) = number + 1;
    return number;
}

// FUNCTION: HGSIM 0x1004e61a
void __cdecl Sim_1004e61a(uint32_t bits)
{
    uint32_t table;
    int32_t i, limit;

    G32(0x1015f350) = 0;
    table = G32(0x1015f35c);
    for (i = 0; i < 0x26f5; i++) {
        U32(table, 0) = 0xffff;
        table += 4;
    }
    limit = (int32_t)((1u << (bits & 31)) + 2);
    for (i = 0; i < limit; i++)
        Sim_1004e4c2(0xffff, (uint8_t)i);
}
