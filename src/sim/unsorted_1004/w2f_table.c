#include "w2f.h"
#include "../util/access.h"

#define TABLE 0x109b95c0
#define COUNT 0x1013b360

// FUNCTION: HGSIM 0x100480f2
void __cdecl Sim_100480f2(void *entry)
{
    U32(entry, 0) = 0xffffffffu;
    U16(entry, 4) = 0;
    U16(entry, 6) = 0;
}

// FUNCTION: HGSIM 0x100480ab
void __cdecl Sim_100480ab(void)
{
    int i;

    for (i = 0; i < 0x60; i++)
        Sim_100480f2(GBYTES(TABLE + (uint32_t)i * 8));
    G32(COUNT) = 0;
}

// FUNCTION: HGSIM 0x10048030
int __cdecl Sim_10048030(uint32_t a, int16_t b, int16_t c)
{
    uint8_t *entry;
    int32_t used;

    if (GS32(COUNT) == -1)
        Sim_100480ab();
    if (GS32(COUNT) == -1 || GS32(COUNT) >= 0x60)
        return -1;
    entry = GBYTES(TABLE + G32(COUNT) * 8);
    U32(entry, 0) = a & 0xffff;
    S16(entry, 4) = b;
    S16(entry, 6) = c;
    used = GS32(COUNT);
    G32(COUNT) = (uint32_t)GS32(COUNT) + 1;
    return used;
}

// FUNCTION: HGSIM 0x10048422
void __cdecl Sim_10048422(int i)
{
    U32(G32(0x109b94e0 + (uint32_t)i * 4), 0) = 0;
    U32(G32(0x109b94e0 + (uint32_t)i * 4), 0xc) = 0;
}
