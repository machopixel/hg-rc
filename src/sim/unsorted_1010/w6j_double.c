#include "w6j.h"
#include "../util/access.h"

typedef union {
    double d;
    uint32_t w[2];
} W6jDoubleBits;

// FUNCTION: HGSIM 0x101037d0
void __cdecl Sim_101037d0(int type)
{
    if (type == 1)
        G32(0x1017e888) = 0x21;
    else if (type > 1 && type <= 3)
        G32(0x1017e888) = 0x22;
}

// FUNCTION: HGSIM 0x10103870
double __cdecl Sim_10103870(double x, int e)
{
    W6jDoubleBits u;
    uint32_t top;

    u.d = x;
    top = ((u.w[1] >> 16) & 0x800fu) | (((uint32_t)e + 0x3feu) << 4);
    u.w[1] = (u.w[1] & 0xffffu) | ((top & 0xffffu) << 16);
    return u.d;
}

// FUNCTION: HGSIM 0x10103940
int __cdecl Sim_10103940(double x)
{
    W6jDoubleBits u;
    uint32_t top;

    u.d = x;
    if (u.w[1] == 0x7ff00000u && u.w[0] == 0)
        return 1;
    if (u.w[1] == 0xfff00000u && u.w[0] == 0)
        return 2;
    top = (u.w[1] >> 16) & 0x7ff8u;
    if (top == 0x7ff8u)
        return 3;
    if (top == 0x7ff0u && ((u.w[1] & 0x7ffffu) != 0 || u.w[0] != 0))
        return 4;
    return 0;
}
