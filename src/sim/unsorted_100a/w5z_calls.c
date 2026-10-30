#include "w5z.h"
#include "../simmem.h"
#include "../util/access.h"
#include "../unsorted_1002/w2j.h"
#include "../unsorted_1000/w3s.h"
#include "../unsorted_1003/w2g.h"
#include "../view/w2d.h"

typedef uint32_t(__cdecl *Fn100a7c4c)(uint32_t a);
typedef uint32_t(__cdecl *Fn100282d4)(uint32_t a, int key, const void *name, uint32_t d);
typedef uint32_t(__cdecl *Fn100ba7d5)(uint32_t a, uint32_t b, uint32_t c);
typedef void(__cdecl *Fn100bb2b0)(uint32_t a, uint32_t b);
typedef void(__cdecl *Fn100a9c25)(void *obj);
typedef uint32_t(__cdecl *Fn10060681)(void *p, uint32_t a, uint32_t b, uint32_t c);
typedef void(__cdecl *Fn1006123b)(uint32_t a, uint32_t b, uint32_t c, uint32_t d, void *p);
typedef void(__cdecl *Fn100a9ba7)(uint32_t t);
typedef uint32_t(__cdecl *Fn100fe830)(void *buf, uint32_t size, uint32_t count, uint32_t handle);

// FUNCTION: HGSIM 0x100a773b
uint32_t __cdecl Sim_100a773b(void)
{
    return SIM_FN(Fn100a7c4c, 0x100a7c4c)(G32(0x1014cf8c));
}

// FUNCTION: HGSIM 0x100a7758
int __cdecl Sim_100a7758(int key, const void *name)
{
    int result;

    if (SIM_FN(Fn100282d4, 0x100282d4)(G32(0x1014cf8c), key, name, 0) != 0) {
        result = 1;
        Sim_1002828e(key, name);
    } else {
        result = 0;
    }
    return result;
}

// FUNCTION: HGSIM 0x100a77a7
int __cdecl Sim_100a77a7(uint32_t a, uint32_t b)
{
    int result;

    if (SIM_FN(Fn100ba7d5, 0x100ba7d5)(a, b, 0) != 0) {
        result = 1;
        SIM_FN(Fn100bb2b0, 0x100bb2b0)(a, b);
    } else {
        result = 0;
    }
    return result;
}

// FUNCTION: HGSIM 0x100a9ccf
uint32_t __cdecl Sim_100a9ccf(void *obj, uint32_t a, uint32_t b, uint32_t c)
{
    SIM_FN(Fn100a9c25, 0x100a9c25)(obj);
    return SIM_FN(Fn10060681, 0x10060681)(BYTES(obj) + 0xc, a, b, c);
}

// FUNCTION: HGSIM 0x100aa0da
void __cdecl Sim_100aa0da(void *obj, uint32_t a, uint32_t b, uint32_t c, uint32_t d)
{
    SIM_FN(Fn1006123b, 0x1006123b)(a, b, c, d, BYTES(obj) + 0x3c);
    U32(obj, 0x94) &= ~0x4000u;
    U32(obj, 0x94) |= 1u;
}

// FUNCTION: HGSIM 0x100aa307
void __cdecl Sim_100aa307(void *obj)
{
    uint32_t blk = U32(obj, 0x30);
    uint32_t t = U32(obj, 0x28);

    SIM_FN(Fn100a9ba7, 0x100a9ba7)(t);
    Sim_100098bb(BYTES(blk), (const float *)(uintptr_t)(t + 0x4c));
    U32(blk, 0x10) = U32(obj, 0x9c);
}

// FUNCTION: HGSIM 0x100aa406
void __cdecl Sim_100aa406(void *obj)
{
    uint32_t child;

    if (U32(obj, 0x8c) != 0)
        Sim_1008a37f(BYTES(U32(obj, 0x8c)));
    for (child = U32(obj, 4); child != 0; child = U32(child, 8))
        Sim_100aa406(BYTES(child));
}

// FUNCTION: HGSIM 0x100aa541
void __cdecl Sim_100aa541(void *obj, uint32_t v)
{
    uint32_t child;

    if (U32(obj, 0x8c) != 0)
        U16(U32(obj, 0x8c), 0x24) = (uint16_t)v;
    for (child = U32(obj, 4); child != 0; child = U32(child, 8))
        Sim_100aa541(BYTES(child), v);
}

// FUNCTION: HGSIM 0x100ae470
uint32_t __cdecl Sim_100ae470(void *f, void *buf, uint32_t count)
{
    uint32_t r = SIM_FN(Fn100fe830, 0x100fe830)(buf, 1, count, U32(f, 0));

    Sim_1003c860(buf, r, U32(f, 4));
    U32(f, 4) = U32(f, 4) + r;
    return r;
}
