#include "w37.h"
#include "../simmem.h"
#include "../util/access.h"

/* The quarter-wave table in the library's code, 0x385 dwords from 0x100ebd27 (16.16 fixed point); the second
   output reads it backwards from its last entry 0x100ecb37. */
#define WAVE(k) (SIM_PTR(s32a, 0x100ebd27)[k])
#define WAVE_BACK(k) (*SIM_PTR(s32a, 0x100ecb37 - 4 * (k)))

// FUNCTION: HGSIM 0x100ecb3b
void __cdecl Sim_100ecb3b(int32_t angle, int32_t *s, int32_t *c)
{
    uint32_t a = (uint32_t)angle, b;
    uint32_t sv, cv;

    if ((int32_t)a < 0) {
        do
            a += 0xe10;
        while ((int32_t)a < 0);
    }
    while ((int32_t)a > 0xe10)
        a -= 0xe10;
    if (a <= 0x708) {
        if (a <= 0x384) {
            sv = (uint32_t)WAVE(a);
            cv = (uint32_t)WAVE_BACK(a);
        } else {
            b = 0x708 - a;
            sv = 0u - (uint32_t)WAVE(b);
            cv = (uint32_t)WAVE_BACK(b);
        }
    } else {
        b = 0xe10 - a;
        if (b <= 0x384) {
            sv = (uint32_t)WAVE(b);
            cv = 0u - (uint32_t)WAVE_BACK(b);
        } else {
            b = 0x708 - b;
            sv = 0u - (uint32_t)WAVE(b);
            cv = 0u - (uint32_t)WAVE_BACK(b);
        }
    }
    *s = (int32_t)sv;
    *c = (int32_t)cv;
}

/* imul, add 0x8000 with carry into the high half: the 64-bit rounded product. */
static int64_t W37_Product(int32_t a, int32_t b)
{
    return (int64_t)a * (int64_t)b + 0x8000;
}

/* Its bits 16..47 (the original's `mov ax, dx` / `ror eax, 16`). */
#define MID(p) ((int32_t)(uint32_t)((uint64_t)(p) >> 16))
/* Its high half (EDX). */
#define HIGH(p) ((int32_t)(uint32_t)((uint64_t)(p) >> 32))

// FUNCTION: HGSIM 0x100ecc19
int32_t __cdecl Sim_100ecc19(const int32_t *in, int32_t *out, const int32_t *centre, int32_t angle, int32_t sx,
                             int32_t sy)
{
    int32_t s, c, x, y, xs, xc, ys, yc;

    Sim_100ecb3b(angle, &s, &c);
    x = HIGH(W37_Product((int32_t)(((uint32_t)in[0] - (uint32_t)centre[0]) << 16), sx));
    xs = MID(W37_Product(x, s));
    xc = MID(W37_Product(x, c));
    y = HIGH(W37_Product((int32_t)(((uint32_t)in[1] - (uint32_t)centre[1]) << 16), sy));
    ys = MID(W37_Product(y, s));
    yc = MID(W37_Product(y, c));
    out[0] = (int32_t)((uint32_t)xs - (uint32_t)yc + (uint32_t)centre[0]);
    out[1] = (int32_t)((uint32_t)ys + (uint32_t)xc + (uint32_t)centre[1]);
    return yc;
}
