#include "w7q.h"
#include "w5z.h"
#include "../unsorted_1000/w5a.h"
#include "../unsorted_1004/fns.h"
#include "../unsorted_100b/w2f.h"
#include "../unsorted_100b/w2j.h"
#include "../unsorted_100b/fns.h"
#include "../util/util.h"
#include "../util/access.h"

typedef float HG_MAY_ALIAS w7qf32;

#define W7Q_FL(p, off) (*(w7qf32 *)(BYTES(p) + (off)))

/* The bits of a - b in single precision (the original: fld, fsub dword, fstp dword). */
static uint32_t W7q_Sub(uint32_t a, uint32_t b)
{
    union {
        float f;
        uint32_t u;
    } x, y;

    x.u = a;
    y.u = b;
    x.f = x.f - y.f;
    return x.u;
}

static float W7q_Float(uint32_t u)
{
    union {
        float f;
        uint32_t u;
    } x;

    x.u = u;
    return x.f;
}

/* The entry of the table 0x109a8640 (0x54 bytes each). */
#define W7Q_TARGET(i) ((uint32_t)(uintptr_t)SIM_PTR(uint8_t, 0x109a8640u + (uint32_t)(i) * 0x54u))

// FUNCTION: HGSIM 0x100ca996
int __cdecl Sim_100ca996(void)
{
    uint32_t ent = PTRTAB(G32(0x1017b8c8));
    uint32_t index = U32(ent, 0x118) & 0xffu;
    uint32_t t = W7Q_TARGET(index);
    uint32_t x, y, z;
    uint32_t a, b, c, d;

    if (U32(t, 0) == 0)
        return 0;
    if (U32(t, 4) != 0)
        Sim_100a9cfb((void *)(uintptr_t)U32(t, 4), &U32(t, 0x18), &U32(t, 0x1c), &U32(t, 0x20));
    x = U32(t, 0x18);
    y = U32(t, 0x1c);
    z = U32(t, 0x20);
    W7Q_CALL_100cbc6b(W7q_Sub(x, U32(ent, 0x58)), W7q_Sub(y, U32(ent, 0x5c)), W7q_Sub(z, U32(ent, 0x60)), &a, &b,
                      &c, &d);
    G32(0x1015e4c8) = x;
    G32(0x1015e4cc) = y;
    G32(0x1015e4d0) = z;
    G32(0x1015e4d4) = a;
    G32(0x1015e4c4) = b;
    G32(0x1015e4c0) = c;
    G32(0x1015e4d8) = d;
    G32(0x1015e4dc) = U32(ent, 0x118);
    t = W7Q_TARGET(index);
    if (!(W7q_Float(G32(0x1015e4c0)) >= W7Q_FL(t, 0x10)) && U32(U32(ent, 0x28), 0x50c) != 1 &&
        (S16(t, 0x24) & 0x20) == 0) {
        U16(t, 0x24) = (uint16_t)(U16(t, 0x24) | 0x20u);
        U16(t, 0x26) = (uint16_t)(U16(t, 0x26) | (uint16_t)(1u << (U32(ent, 8) & 31u)));
        Sim_100cbdb8(ent, 1, 0);
        Sim_1000ebfb((const char *)SIM_PTR(char, 0x10151d34), 0xfffff63cu, 0, 5, 0x50);
    }
    return 1;
}

// FUNCTION: HGSIM 0x100cab7b
int __cdecl Sim_100cab7b(int who, int index, uint32_t flags)
{
    uint32_t ent;
    uint32_t t;
    uint32_t x, y, z;
    uint32_t a, b, c, d;

    if (index < 0)
        return -1;
    if (index >= GS32(0x1017e5a8))
        return -2;
    if ((flags & 0xeu) != 0)
        return -9;
    ent = PTRTAB(who);
    t = W7Q_TARGET(index);
    if (U32(t, 0) == 0)
        return -3;
    if ((flags & 0x10000u) != 0 && (S16(t, 0x24) & 0x40) == 0)
        return -4;
    if ((S16(t, 0x24) & 1) != 0) {
        if (U32(t, 8) != ((uint32_t)who | 0x200u))
            return -6;
    } else if ((flags & 0x100u) != 0) {
        return -6;
    }
    if (U32(t, 0xc) != U32(ent, 8))
        return -6;
    if (U32(t, 4) != 0)
        Sim_100a9cfb((void *)(uintptr_t)U32(t, 4), &U32(t, 0x18), &U32(t, 0x1c), &U32(t, 0x20));
    x = U32(t, 0x18);
    y = U32(t, 0x1c);
    z = U32(t, 0x20);
    W7Q_CALL_100cbc6b(W7q_Sub(x, U32(ent, 0x58)), W7q_Sub(y, U32(ent, 0x5c)), W7q_Sub(z, U32(ent, 0x60)), &a, &b,
                      &c, &d);
    U32(ent, 0xdc) = x;
    U32(ent, 0xe0) = y;
    U32(ent, 0xe4) = z;
    U32(ent, 0xe8) = a;
    U32(ent, 0xd8) = b;
    U32(ent, 0xd4) = c;
    U32(ent, 0xec) = d;
    U32(ent, 0x104) = x;
    U32(ent, 0x108) = y;
    U32(ent, 0x10c) = z;
    U32(ent, 0x110) = a;
    U32(ent, 0x100) = b;
    U32(ent, 0xfc) = c;
    U32(ent, 0x114) = d;
    return 1;
}

// FUNCTION: HGSIM 0x100cb299
int __cdecl Sim_100cb299(int who, int index, uint32_t flags)
{
    uint32_t ent;
    uint32_t r;
    uint32_t x, y, z;
    uint32_t a, b, c, d;

    if (index < 0)
        return -1;
    if (index >= GS32(0x1015dc68))
        return -2;
    if ((flags & 0x23u) != 0)
        return -9;
    ent = PTRTAB(who);
    r = (uint32_t)(uintptr_t)SIM_PTR(uint8_t, 0x109e9ba0u + (uint32_t)index * 0x50u);
    if ((S16(r, 0) & 4) != 0)
        return -3;
    /* the original masks 0x10000 with a 0/1 value, so this test never fires; kept as written */
    if (((flags & 0x10000u) & ((S16(r, 0) & 0x40) == 0 ? 1u : 0u)) != 0)
        return -4;
    if ((flags & 0x20000u) != 0 && Sim_10052c02(index) != 0)
        return -9;
    if ((flags & 0x40000u) != 0 && Sim_10052c02(index) != 1)
        return -9;
    if (G32(0x1017e5b0) == 0 && (S16(r, 0) & 0x1400) == 0 && (uint32_t)who == G32(0x1017b8c8))
        return -5;
    if ((S16(r, 0) & 0x800) != 0 && (uint32_t)who == G32(0x1017b8c8))
        return -5;
    if (Sim_100b5386(S32(r, 8)) == 0)
        return -8;
    if (Sim_100b53bb(S32(r, 8)) == 0)
        return -8;
    Sim_100b53f0(S32(r, 8), &x, &y, &z);
    W7Q_CALL_100cbc6b(W7q_Sub(x, U32(ent, 0x58)), W7q_Sub(y, U32(ent, 0x5c)), W7q_Sub(z, U32(ent, 0x60)), &a, &b,
                      &c, &d);
    if ((S16(r, 0) & 0x1000) == 0 && W7q_Float(b) > *SIM_PTR(w7qf32, 0x1012bc48) &&
        (uint32_t)who == G32(0x1017b8c8))
        return -7;
    U32(ent, 0xdc) = x;
    U32(ent, 0xe0) = y;
    U32(ent, 0xe4) = z;
    U32(ent, 0xe8) = a;
    U32(ent, 0xd8) = b;
    U32(ent, 0xd4) = c;
    U32(ent, 0xec) = d;
    return 1;
}
