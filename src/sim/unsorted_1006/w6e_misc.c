#include "w6e.h"
#include "w5a.h"
#include "w5b.h"
#include "w5c.h"
#include "../unsorted_1000/w5x.h"
#include "../unsorted_1002/w5y.h"
#include "../unsorted_1002/w5z.h"
#include "../unsorted_1004/w2e.h"
#include "../unsorted_1004/w2k.h"
#include "../unsorted_1004/w3o.h"
#include "../unsorted_1004/w5y.h"
#include "../unsorted_1008/w3o.h"
#include "../unsorted_100a/w5a.h"
#include "../unsorted_100f/w2e.h"
#include "../util/util.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x100607d5
void __cdecl Sim_100607d5(float *m)
{
    Sim_10060882(m, m + 1, m + 2);
    Sim_10060882(m + 3, m + 4, m + 5);
    Sim_10060882(m + 6, m + 7, m + 8);
    Sim_10060882(m, m + 3, m + 6);
    Sim_10060882(m + 1, m + 4, m + 7);
    Sim_10060882(m + 2, m + 5, m + 8);
}

/* The number at +0x150 of the object, as the signed 16-bit value the original passes on. */
static uint32_t Number(const uint32_t *obj)
{
    return (uint32_t)(int32_t)(int16_t)(U32(obj[0], 0x150) & 0xffff);
}

// FUNCTION: HGSIM 0x10061c32
void __cdecl Sim_10061c32(uint32_t *obj)
{
    uint32_t x = U32(obj[0], 0x58);
    uint32_t y = U32(obj[0], 0x5c);
    uint32_t z = U32(obj[0], 0x60);

    if (U32(obj, 0x4d8) == 0) {
        Sim_10024616(Number(obj), 0x24, x, y, z, x, y, z);
        return;
    }
    if (Sim_1009c929(100) > 0x14)
        return;
    x = W6e_Bits((float)((double)(int32_t)Sim_1009c969() / 2.0 + (double)W6e_Float(x)));
    z = W6e_Bits((float)((double)(int32_t)Sim_1009c969() / 2.0 + (double)W6e_Float(z)));
    if (Sim_1009c929(100) < 0x3c)
        Sim_10024616(Number(obj), 0x3e, x, y, z, x, y, z);
    else
        Sim_10024616(Number(obj), 0x10b, x, y, z, x, y, z);
}

// FUNCTION: HGSIM 0x10061dfc
void __cdecl Sim_10061dfc(uint32_t *obj)
{
    uint32_t target = obj[0];
    uint32_t a, b, c;
    int slot;

    for (slot = 4; slot <= 5; slot++) {
        if (U32(obj, 0x498 + slot * 4) != 0) {
            U32(target, 0x50) = U32(obj, 0x498 + slot * 4);
            Sim_10024684(0x19, obj[0]);
            Sim_100a9cfb((void *)(uintptr_t)U32(target, 0x50), &a, &b, &c);
        }
    }
}

// FUNCTION: HGSIM 0x10061ecd
void __cdecl Sim_10061ecd(uint32_t *obj, float value)
{
    char name[16];
    float x, y, z;

    if (U32(obj[0], 4) == G32(0x1017b8c8))
        Sim_10061f8e(0.0f, -value, 0.0f);
    if (value > 140.0f)
        Str_Copy(name, SIM_PTR(char, 0x10144ea4));
    else
        Str_Copy(name, SIM_PTR(char, 0x10144eb0));
    z = W6E_F(obj[0], 0x60) - W6E_F(G32(0x1014cae0), 8);
    y = (float)(((double)W6E_F(obj[0], 0x5c) - (double)W6E_F(obj, 0x520)) - (double)W6E_F(G32(0x1014cae0), 4));
    x = W6E_F(obj[0], 0x58) - W6E_F(G32(0x1014cae0), 0);
    Sim_1000ed52(W6e_Bits(x), W6e_Bits(y), W6e_Bits(z), (uint32_t)(uintptr_t)name, G32(0x1015ef0c), obj[0]);
}

/* The original tests the FPU "equal" flag, which an unordered compare (NaN) also sets. */
static int EqualsZero(float v)
{
    return !(v < 0.0f) && !(v > 0.0f);
}

// FUNCTION: HGSIM 0x10061f8e
void __cdecl Sim_10061f8e(float x, float y, float z)
{
    float spread;
    float zero = 0.0f;

    if ((S16(PTRTAB(G32(0x1017b8c8)), 0x1c) & 2) != 0)
        return;
    if (GS32(0x1015dc80) <= 0x12)
        return;
    if (Sim_10044311() != 0)
        return;
    Sim_10084c95(&x, &y, &z);
    if (EqualsZero(x) && EqualsZero(z))
        spread = y * 30.0f;
    else
        spread = (float)(((double)y - (double)z) * 5.0);
    x = x * -25.0f;
    y = y * -25.0f;
    z = z * -25.0f;
    Sim_10044255();
    Sim_10044296(W6e_Bits(x), W6e_Bits(y), W6e_Bits(z), W6e_Bits(spread), W6e_Bits(zero), W6e_Bits(zero), 0.2);
    Sim_10044296(W6e_Bits(-(x / 2.0f)), W6e_Bits(-(y / 2.0f)), W6e_Bits(-(z / 2.0f)), W6e_Bits(-(spread / 2.0f)),
                 W6e_Bits(-(zero / 2.0f)), W6e_Bits(-(zero / 2.0f)), 0.5);
    Sim_10044296(0, 0, 0, 0, 0, 0, 0.2);
    Sim_10044069();
}

// FUNCTION: HGSIM 0x1006326b
void __cdecl Sim_1006326b(const uint32_t *rect)
{
    G32(0x101cc2bc) = rect[0] - rect[2];
    G32(0x101ccc6c) = rect[1] - rect[3];
    Sim_100a2b8c(rect[0] + rect[2], rect[1] - rect[3], rect[7]);
    Sim_100a2b8c(rect[0] + rect[2], rect[1] + rect[3], rect[7]);
    Sim_100a2b8c(rect[0] - rect[2], rect[1] + rect[3], rect[7]);
    Sim_100a2b8c(rect[0] - rect[2], rect[1] - rect[3], rect[7]);
    G32(0x101cc2bc) = rect[0];
    G32(0x101ccc6c) = rect[1] - rect[3];
    Sim_100a2b8c(rect[0], 0, rect[7]);
    G32(0x101cc2bc) = rect[0] + rect[2];
    G32(0x101ccc6c) = rect[1];
    Sim_100a2b8c(G32(0x109e3de8), rect[1], rect[7]);
    G32(0x101cc2bc) = rect[0];
    G32(0x101ccc6c) = rect[1] + rect[3];
    Sim_100a2b8c(rect[0], G32(0x109e3da8), rect[7]);
    G32(0x101cc2bc) = rect[0] - rect[2];
    G32(0x101ccc6c) = rect[1];
    Sim_100a2b8c(0, rect[1], rect[7]);
}

// FUNCTION: HGSIM 0x1006863d
void __cdecl Sim_1006863d(uint32_t *obj)
{
    volatile float unused;

    if (G32(0x1015ecd0) == 0 || (U32(obj[0], 0x14) == 3 && U32(obj[0], 0xc) == G32(0x1017b8c8)) ||
        U32(obj[0], 4) == G32(0x1017b8c8) || U32(obj, 0x4f0) == 4) {
        W6E_CALL_1006abf6(0xfffffffeu, obj, 2);
        U16(obj, 0x560) = (uint16_t)(U16(obj, 0x560) | 0x100);
        /* the original then computes the offset of the object from the point at [0x1014cae0] into locals that are
           never read; the reads are kept */
        unused = W6E_F(obj[0], 0x58) - W6E_F(G32(0x1014cae0), 0);
        unused = W6E_F(obj[0], 0x5c) - W6E_F(G32(0x1014cae0), 4);
        unused = W6E_F(obj[0], 0x60) - W6E_F(G32(0x1014cae0), 8);
        (void)unused;
    }
}

// FUNCTION: HGSIM 0x10068d76
void __cdecl Sim_10068d76(float x, float y, float z, float a, float b, float c, uint32_t *outX, uint32_t *outY,
                          uint32_t *outZ)
{
    union {
        float f;
        uint32_t u;
    } p[3];

    if (outX == 0 || outY == 0 || outZ == 0)
        return;
    p[0].f = x;
    p[1].f = y;
    p[2].f = z;
    Sim_10068bb2(p[0].u, p[1].f, p[2].f, a, &p[0].u, &p[1].f, &p[2].f);
    Sim_10068c48(p[0].f, p[1].u, p[2].f, b, &p[0].f, &p[1].u, &p[2].f);
    Sim_10068ce0(p[0].f, p[1].f, p[2].u, c, &p[0].f, &p[1].f, &p[2].u);
    *(u32a *)outX = p[0].u;
    *(u32a *)outY = p[1].u;
    *(u32a *)outZ = p[2].u;
}
