#include "w2k.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x100526a9
int __cdecl Sim_100526a9(unsigned index, unsigned value)
{
    int stored = 0;

    if (index < G32(0x1015DC64) && value < 8) {
        U32(PTRTAB(index), 0x10) = value;
        stored = 1;
    }
    return stored;
}

// FUNCTION: HGSIM 0x10052c30
int __cdecl Sim_10052c30(unsigned index)
{
    int result = 2;

    if (index < 0x10)
        result = (int)G32(0x109B7D54u + G32(0x109A864Cu + index * 0x54u) * 0x6Cu);
    return result;
}

// FUNCTION: HGSIM 0x10052ff0
uint32_t __cdecl Sim_10052ff0(uint32_t value)
{
    return value;
}

// FUNCTION: HGSIM 0x10052ffb
uint32_t __cdecl Sim_10052ffb(uint32_t value)
{
    return value;
}

// FUNCTION: HGSIM 0x1005571d
void __cdecl Sim_1005571d(void)
{
    G32(0x1013F140) = 1;
}

// FUNCTION: HGSIM 0x1005572c
uint32_t __cdecl Sim_1005572c(int index)
{
    int16_t i = (int16_t)index;

    return ((uint32_t)(int32_t)i & 0xFFFF0000u) | *GBYTES(0x10164BC8u + (uint32_t)(int32_t)i);
}

// FUNCTION: HGSIM 0x10057a43
int __cdecl Sim_10057a43(void)
{
    return 0;
}

// FUNCTION: HGSIM 0x10057a58
int __cdecl Sim_10057a58(uint32_t a, uint32_t b, uint32_t *out)
{
    int i;

    if ((G32(0x1015E65C) & 0x1000000) == 0 && out != NULL) {
        for (i = 0; i < 4; i++)
            out[i] = G32(0x1015E650 + i * 4);
    }
    return 0;
}

// FUNCTION: HGSIM 0x1005b7e0
void __cdecl Sim_1005b7e0(void *obj)
{
    int i, j;

    for (i = 0; i < 0x11; i++) {
        uint32_t d = G32(0x10138D70 + i * 4);
        uint32_t count = G32(0x10138FE8u + U32(obj, 0xBDB) * 0x24Cu + d * 4);

        U32(obj, 0xB67 + i * 4) = count;
        /* obj+0xbdb and the table index are read again on every pass, as the original does */
        for (j = 0; j < (int)count; j++)
            U32(obj, 0x617 + i * 0x50 + j * 4) =
                G32(0x10138DB8u + U32(obj, 0xBDB) * 0x24Cu + G32(0x10138D70 + i * 4) * 0x50u + j * 4);
    }
}

// FUNCTION: HGSIM 0x1005b923
int __cdecl Sim_1005b923(unsigned kind)
{
    switch (kind) {
    case 8:
        return 7;
    case 9:
    case 10:
        return 3;
    case 11:
    case 12:
        return 4;
    case 13:
    case 14:
    case 15:
    case 16:
        return 2;
    default:
        return 0;
    }
}
