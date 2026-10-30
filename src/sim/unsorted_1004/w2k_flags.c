#include "w2k.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x10044311
uint32_t __cdecl Sim_10044311(void)
{
    return G32(0x1015EE98);
}

// FUNCTION: HGSIM 0x10045f3b
void __cdecl Sim_10045f3b(unsigned kind, uint32_t a, uint32_t b)
{
    switch (kind) {
    case 0:
        G32(0x1013AF48) = 1;
        G32(0x1013AF4C) = a;
        G32(0x1013AF50) = b;
        break;
    case 1:
        G32(0x1013AF60) = 1;
        G32(0x1013AF64) = a;
        G32(0x1013AF68) = 0x32;
        break;
    case 2:
        G32(0x1013AF60) = 1;
        G32(0x1013AF64) = a;
        G32(0x1013AF68) = 0x3c;
        break;
    case 3:
        G32(0x1013AF60) = 1;
        G32(0x1013AF64) = a;
        G32(0x1013AF68) = 0x50;
        break;
    case 4:
        G32(0x1013AF54) = 1;
        G32(0x1013AF58) = a;
        G32(0x1013AF5C) = 0x32;
        break;
    case 5:
        G32(0x1013AF54) = 1;
        G32(0x1013AF58) = a;
        G32(0x1013AF5C) = 0x3c;
        break;
    case 6:
        G32(0x1013AF54) = 1;
        G32(0x1013AF58) = a;
        G32(0x1013AF5C) = 0x50;
        break;
    case 7:
        G32(0x1013AF6C) = 1;
        G32(0x1013AF70) = a;
        G32(0x1013AF74) = b;
        break;
    case 8:
        G32(0x1013AF78) = 1;
        G32(0x1013AF7C) = a;
        G32(0x1013AF80) = b;
        break;
    case 9:
        G32(0x1013AF84) = 1;
        G32(0x1013AF88) = a;
        G32(0x1013AF8C) = b;
        break;
    case 10:
        G32(0x1013AF90) = 1;
        G32(0x1013AF94) = a;
        G32(0x1013AF98) = b;
        break;
    default:
        break;
    }
}

// FUNCTION: HGSIM 0x1004c5b7
int __cdecl Sim_1004c5b7(void) { return 1; }

// FUNCTION: HGSIM 0x1004cdaf
int __cdecl Sim_1004cdaf(unsigned flags) { return (flags & 0x004) != 0; }

// FUNCTION: HGSIM 0x1004cdc0
int __cdecl Sim_1004cdc0(unsigned flags) { return (flags & 0x008) != 0; }

// FUNCTION: HGSIM 0x1004cdf3
int __cdecl Sim_1004cdf3(unsigned flags) { return (flags & 0x040) != 0; }

// FUNCTION: HGSIM 0x1004ce04
int __cdecl Sim_1004ce04(unsigned flags) { return (flags & 0x080) != 0; }

// FUNCTION: HGSIM 0x1004ce17
int __cdecl Sim_1004ce17(unsigned flags) { return (flags & 0x200) != 0; }

// FUNCTION: HGSIM 0x1004ce2a
int __cdecl Sim_1004ce2a(unsigned flags) { return (flags & 0x400) != 0; }

// FUNCTION: HGSIM 0x1004ce3d
int __cdecl Sim_1004ce3d(unsigned flags) { return (flags & 0x800) != 0; }

#define ADD_IF(test, addr) \
    do { if (test(flags)) sum += G32(addr); } while (0)

// FUNCTION: HGSIM 0x1004ce64
int __cdecl Sim_1004ce64(int row, unsigned flags)
{
    uint32_t sum = G32(0x101116BEu + (uint32_t)row * 0xAAu);

    ADD_IF(Sim_1004cd8d, 0x10126300);
    ADD_IF(Sim_1004cd9e, 0x10126398);
    ADD_IF(Sim_1004cdaf, 0x10126430);
    ADD_IF(Sim_1004cdc0, 0x101264C8);
    ADD_IF(Sim_1004cdd1, 0x10126560);
    ADD_IF(Sim_1004cde2, 0x101265F8);
    ADD_IF(Sim_1004cdf3, 0x10126690);
    ADD_IF(Sim_1004ce04, 0x10126728);
    ADD_IF(Sim_1004ce17, 0x101267C0);
    ADD_IF(Sim_1004ce2a, 0x10126858);
    ADD_IF(Sim_1004ce3d, 0x101268F0);
    return (int)sum;
}

// FUNCTION: HGSIM 0x1004cfb5
int __cdecl Sim_1004cfb5(int row, unsigned flags)
{
    uint32_t sum = G32(0x101116BAu + (uint32_t)row * 0xAAu);

    ADD_IF(Sim_1004cd8d, 0x101262F4);
    ADD_IF(Sim_1004cd9e, 0x1012638C);
    ADD_IF(Sim_1004cdaf, 0x10126424);
    ADD_IF(Sim_1004cdc0, 0x101264BC);
    ADD_IF(Sim_1004cdd1, 0x10126554);
    ADD_IF(Sim_1004cde2, 0x101265EC);
    ADD_IF(Sim_1004cdf3, 0x10126684);
    ADD_IF(Sim_1004ce04, 0x1012671C);
    ADD_IF(Sim_1004ce17, 0x101267B4);
    ADD_IF(Sim_1004ce2a, 0x1012684C);
    ADD_IF(Sim_1004ce3d, 0x101268E4);
    return (int)sum;
}

// FUNCTION: HGSIM 0x1004d106
int __cdecl Sim_1004d106(int row, unsigned flags)
{
    uint32_t sum = G32(0x101116A6u + (uint32_t)row * 0xAAu);

    ADD_IF(Sim_1004cd8d, 0x101262DC);
    ADD_IF(Sim_1004cd9e, 0x10126374);
    ADD_IF(Sim_1004cdaf, 0x1012640C);
    ADD_IF(Sim_1004cdc0, 0x101264A4);
    ADD_IF(Sim_1004cdd1, 0x1012653C);
    ADD_IF(Sim_1004cde2, 0x101265D4);
    ADD_IF(Sim_1004cdf3, 0x1012666C);
    ADD_IF(Sim_1004ce04, 0x10126704);
    ADD_IF(Sim_1004ce17, 0x1012679C);
    ADD_IF(Sim_1004ce2a, 0x10126834);
    ADD_IF(Sim_1004ce3d, 0x101268CC);
    return (int)sum;
}

// FUNCTION: HGSIM 0x1004db86
int __cdecl Sim_1004db86(unsigned n)
{
    int bits = 1;

    if (n == 0)
        return 0;
    n = (n - 1) >> 1;
    while (n != 0) {
        bits++;
        n >>= 1;
    }
    return bits;
}
