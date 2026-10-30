#include "w2f.h"
#include "../util/access.h"

/* 16.16 multiply, rounded: the 64-bit product shifted right 16 plus the last bit shifted out (shrd / adc). */
static int32_t FixMul(int32_t a, int32_t b)
{
    int64_t p = (int64_t)a * b;

    return (int32_t)((uint32_t)(p >> 16) + (uint32_t)((p >> 15) & 1));
}

// FUNCTION: HGSIM 0x10087b32
void __cdecl Sim_10087b32(void)
{
    G32(0x1017c8c4) = 0;
}

// FUNCTION: HGSIM 0x1008988f
void __cdecl Sim_1008988f(void)
{
    uint32_t node;

    if (G32(0x1017c8c4) != 0)
        return;
    for (node = G32(0x1017b998); node != 0; node = U32(node, 0x10)) {
        G32(0x1017c8e0) = G32(0x1017c8e0) + 1;
        if (U32(node, 0x28) == 0 || U32(U32(node, 0x28), 0) == 0)
            G32(0x1017c8d8) = G32(0x1017c8d8) + 1;
        else
            G32(0x1017c8e4) = G32(0x1017c8e4) + 1;
    }
}

// FUNCTION: HGSIM 0x1008cc04
int32_t *__cdecl Sim_1008cc04(void *unused, const int32_t *rect, int32_t *out)
{
    (void)unused;
    S32(out, 0) = FixMul(GS32(0x109e3e14), S32(rect, 0));
    S32(out, 4) = FixMul(GS32(0x109e3de0), S32(rect, 4));
    S32(out, 8) = FixMul(GS32(0x109e3e14), S32(rect, 8));
    S32(out, 12) = FixMul(GS32(0x109e3de0), S32(rect, 12));
    return out;
}
