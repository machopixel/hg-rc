#include "w5y.h"
#include "../util/access.h"

typedef void(__cdecl *Call12Fn)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t,
                                uint32_t, uint32_t, uint32_t, uint32_t);

// FUNCTION: HGSIM 0x1002001f
void __cdecl Sim_1002001f(void)
{
    uint32_t node = G32(0x1015d528);

    if (node == 0)
        return;
    node = U32(node, 0x67);
    while (node != 0)
        node = SIM_FN(uint32_t(__cdecl *)(uint32_t), 0x1001ffc8)(node);
}

// FUNCTION: HGSIM 0x10020441
int __cdecl Sim_10020441(void *a)
{
    return SIM_FN(int(__cdecl *)(void *, int, int), 0x1001f790)(a, 0, -1);
}

// FUNCTION: HGSIM 0x1002464b
void __cdecl Sim_1002464b(uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4, uint32_t a5, uint32_t a6, uint32_t a7)
{
    SIM_FN(Call12Fn, 0x100246b3)((uint32_t)-2, 0, a1, a5, a6, a7, a5, a6, a7, a2, a3, a4);
}

// FUNCTION: HGSIM 0x10024684
void __cdecl Sim_10024684(uint32_t a1, uint32_t a2)
{
    G32(0x1015dc08) = a2;
    SIM_FN(Call12Fn, 0x100246b3)((uint32_t)-2, 0, a1, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

// FUNCTION: HGSIM 0x10025dac
int __cdecl Sim_10025dac(void)
{
    return SIM_FN(int(__cdecl *)(const void *, const void *), 0x10046f7f)(GBYTES(0x1013489c), GBYTES(0x101653e0));
}

// FUNCTION: HGSIM 0x100291a2
void __cdecl Sim_100291a2(int i, float d)
{
    float *value = SIM_PTR(float, 0x109f09ecu + (uint32_t)i * 0x14u);

    *value = (float)((double)*value - (double)d);
    if ((*(u32a *)value & 0x80000000u) != 0)
        SIM_FN(void(__cdecl *)(int), 0x10029125)(i);
}

// FUNCTION: HGSIM 0x1002d454
uint32_t __cdecl Sim_1002d454(void *obj, uint32_t arg)
{
    return SIM_FN(uint32_t(__cdecl *)(uint32_t, uint32_t, uint32_t), 0x10030c87)(U32(obj, 0x98), U32(obj, 0x2c), arg) & 1;
}
