#include "w5z.h"
#include "../simmem.h"
#include "../util/access.h"

typedef uint32_t(__cdecl *Fn10020256)(uint32_t a, uint32_t b);
typedef uint32_t(__cdecl *Fn100246b3)(uint32_t a0, uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4, uint32_t a5, uint32_t a6,
                                      uint32_t a7, uint32_t a8, uint32_t a9, uint32_t a10, uint32_t a11);
typedef uint32_t(__cdecl *Fn100282d4)(uint32_t a, uint32_t b, uint32_t c, uint32_t d);
typedef void(__cdecl *Fn10027fb8)(uint32_t a);
typedef void(__cdecl *Fn1002ce45)(uint32_t obj, uint32_t f1, uint32_t f2, uint32_t f3, uint32_t *r, uint32_t *b, uint32_t *c);

// FUNCTION: HGSIM 0x100203b8
uint32_t __cdecl Sim_100203b8(uint32_t unused)
{
    (void)unused;
    return SIM_FN(Fn10020256, 0x10020256)(2, 0);
}

// FUNCTION: HGSIM 0x10024616
uint32_t __cdecl Sim_10024616(uint32_t a0, uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4, uint32_t a5, uint32_t a6, uint32_t a7)
{
    return SIM_FN(Fn100246b3, 0x100246b3)(a0, 0, a1, a2, a3, a4, a5, a6, a7, 0, 0, 0);
}

// FUNCTION: HGSIM 0x1002851b
uint32_t __cdecl Sim_1002851b(uint32_t a, uint32_t b)
{
    return SIM_FN(Fn100282d4, 0x100282d4)(0, a, b, 0);
}

// FUNCTION: HGSIM 0x10028534
int __cdecl Sim_10028534(void)
{
    if (G32(0x109f13f0) == 0)
        return 0;
    SIM_FN(Fn10027fb8, 0x10027fb8)(G32(0x109f13f0));
    return 1;
}

// FUNCTION: HGSIM 0x1002cdb0
uint32_t __cdecl Sim_1002cdb0(uint32_t obj, uint32_t f1, uint32_t f2, uint32_t f3)
{
    uint32_t r, b, c;

    SIM_FN(Fn1002ce45, 0x1002ce45)(obj, f1, f2, f3, &r, &b, &c);
    return r;
}
