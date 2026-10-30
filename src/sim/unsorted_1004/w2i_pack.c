#include "w2i.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x1004bf28
int __cdecl Sim_1004bf28(int a, int b, int c, int d)
{
    uint32_t word = 0;

    word |= (uint32_t)a - 6u;
    word |= ((uint32_t)b - 9u) << 4;
    word |= ((uint32_t)c - 0xbu) << 8;
    if (d != 0)
        word |= 0x1000u;
    return (int)word;
}

// FUNCTION: HGSIM 0x1004c2f7
int __cdecl Sim_1004c2f7(int hi, int lo)
{
    return (int)(((uint32_t)hi << 16) | ((uint32_t)lo & 0xFFFFu));
}

// FUNCTION: HGSIM 0x1004c501
int __cdecl Sim_1004c501(int a, int b, int c, int d, int e)
{
    uint32_t word = 0;

    word |= (uint32_t)a - 0xeu;
    word |= ((uint32_t)b - 0xeu) << 4;
    word |= ((uint32_t)c - 0xeu) << 8;
    word |= ((uint32_t)d - 0x16u) << 12;
    word |= ((uint32_t)e - 0x13u) << 16;
    return (int)word;
}

// FUNCTION: HGSIM 0x1004c8a5
int __cdecl Sim_1004c8a5(int a, int b, int c, int d, int e)
{
    uint32_t word = 0;

    word |= (uint32_t)a - 0x18u;
    word |= ((uint32_t)b - 0x18u) << 4;
    word |= ((uint32_t)c - 0x18u) << 8;
    word |= ((uint32_t)d - 0x1du) << 12;
    word |= ((uint32_t)e - 0x20u) << 16;
    return (int)word;
}

// FUNCTION: HGSIM 0x1004ccd9
int __cdecl Sim_1004ccd9(int a0, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8, int a9, int a10)
{
    uint32_t word = 0;

    if (a0 != 0)
        word |= 1;
    if (a1 != 0)
        word |= 2;
    if (a2 != 0)
        word |= 4;
    if (a3 != 0)
        word |= 8;
    if (a4 != 0)
        word |= 0x10;
    if (a5 != 0)
        word |= 0x20;
    if (a6 != 0)
        word |= 0x40;
    if (a7 != 0)
        word |= 0x80;
    if (a8 != 0)
        word |= 0x200;
    if (a9 != 0)
        word |= 0x400;
    if (a10 != 0)
        word |= 0x800;
    return (int)word;
}

// FUNCTION: HGSIM 0x1004d257
int __cdecl Sim_1004d257(int lo, int hi)
{
    return (int)(((uint32_t)hi << 16) | ((uint32_t)lo & 0xFFFFu));
}

// FUNCTION: HGSIM 0x1004bac0
int __cdecl Sim_1004bac0(int index)
{
    int32_t type = GS32(0x1011168C + (uint32_t)index * 0xaau);
    int kind;

    if (type == 0)
        kind = 0;
    else if (type == 1)
        kind = 1;
    else if (type == 3)
        kind = 2;
    else if (type == 4)
        kind = 3;
    else if (type == 2)
        kind = 4;
    else if (type >= 5 && type <= 10)
        kind = 5;
    else
        kind = -1;

    switch (kind) {
    case 0:
        return Sim_1004bf28(6, 10, 11, 0);
    case 1:
        return Sim_1004c2f7(0x2d, (int)G32(0x101116BE + (uint32_t)index * 0xaau));
    case 2:
        return Sim_1004c501(14, 14, 14, 22, 19);
    case 3:
        return Sim_1004c8a5(24, 24, 24, 29, 32);
    case 4:
        return Sim_1004ccd9(0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    case 5:
        return Sim_1004d257(0x2f, 0);
    default:
        return 0;
    }
}

// FUNCTION: HGSIM 0x1004d285
int __cdecl Sim_1004d285(int index)
{
    uint32_t type = G32(0x101116C6 + (uint32_t)index * 0xaau);

    switch (type) {
    case 0xf:
    case 0x10:
    case 0x11:
    case 0x12:
        return 4;
    case 0x13:
    case 0x14:
    case 0x15:
        return 3;
    case 0x16:
    case 0x17:
        return 6;
    case 0x18:
        return 1;
    case 0x19:
        return 2;
    case 0x1a:
        return 3;
    default:
        return 1;
    }
}
