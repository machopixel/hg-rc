#include "w8v.h"
#include "w7m.h"
#include "../util/access.h"
#include "../util/w3o.h"
#include "../unsorted_1004/w2h.h"
#include "../unsorted_1008/w6f.h"

/*
 * Rectangles here are the HUD surfaces' clip rectangles: dwords +4 left, +8 top, +0xc right, +0x10 bottom.
 * Floating point: every product and quotient below is rounded to double exactly where the x87 code (control
 * word 0x27f, 53-bit precision) rounds it, and to float where it stores a float, so the results are bit-exact.
 */
#define HALF_W(r) ((int32_t)(U32(r, 0xc) - U32(r, 4) + 1) >> 1)
#define HALF_H(r) ((int32_t)(U32(r, 0x10) - U32(r, 8) + 1) >> 1)

/* colour16 points to the 16-bit colour */
static void Line(uint32_t surface, int32_t x1, int32_t y1, int32_t x2, int32_t y2, const u16a *colour16)
{
    W8V_LINE16(surface, x1, y1, x2, y2, 0, *colour16);
}

/*
 * A needle at `angle` degrees: from the point at (cos, sin) times the width of rect2, clipped to rect2 and moved
 * to rect's centre, (or from rect's centre when rect2 is NULL) to the point at (cos, sin) times the width of
 * rect, clipped to rect. cos is the table lookup of angle + 90.
 */
// FUNCTION: HGSIM 0x10050e7e
void __cdecl Sim_10050e7e(uint8_t *rect, float angle, int32_t colour, uint8_t *rect2)
{
    int32_t pt[2], out[2], out2[2];
    int32_t n;
    float w, s, c;

    n = (int32_t)(U32(rect, 0x10) - U32(rect, 8) + 1);
    w = (float)n;
    s = W8V_CALL_10084766(angle);
    c = W8V_CALL_10084766((float)((double)angle + (double)GFL(0x1012b434)));
    pt[0] = (int32_t)((uint32_t)Render_Ftol((double)w * (double)c) + (uint32_t)HALF_W(rect));
    pt[1] = (int32_t)((uint32_t)Render_Ftol((double)w * (double)s) + (uint32_t)HALF_H(rect));
    W8V_CALL_1008e9af(rect, pt, out);
    if (rect2 != 0) {
        n = (int32_t)(U32(rect2, 0xc) - U32(rect2, 4) + 1);
        w = (float)n;
        /* the original multiplies the exact width (still on the x87 stack) here, the float copy below */
        pt[0] = (int32_t)((uint32_t)Render_Ftol((double)n * (double)c) + (uint32_t)HALF_W(rect2));
        pt[1] = (int32_t)((uint32_t)Render_Ftol((double)w * (double)s) + (uint32_t)HALF_H(rect2));
        Sim_1008e266((const int32_t *)rect2, pt, out2);
        out2[0] = (int32_t)((uint32_t)out2[0] + (uint32_t)(HALF_W(rect) - HALF_W(rect2)));
        out2[1] = (int32_t)((uint32_t)out2[1] + (uint32_t)(HALF_H(rect) - HALF_H(rect2)));
        Line((uint32_t)(uintptr_t)rect, out[0], out[1], out2[0], out2[1], SIM_PTR(u16a, 0x101659e0) + colour);
    } else {
        Line((uint32_t)(uintptr_t)rect, out[0], out[1], HALF_W(rect), HALF_H(rect), SIM_PTR(u16a, 0x101659e0) + colour);
    }
}

/*
 * The value 0x1013baf8 as a share of the gauge maximum (+0x94) in 16.16 fixed point, times 360 with the
 * original's rounding (the bit shifted out last is added), in whole degrees. The 32-bit quotient equals the
 * original's 64 by 32 bit division, which only uses the low words of both operands (both faulting together).
 */
static int32_t Degrees(int32_t value, int32_t max)
{
    int32_t q = (int32_t)((uint32_t)(int16_t)value << 16) / (int32_t)(int16_t)max;
    int64_t prod = (int64_t)q * 0x1680000;
    uint32_t m = (uint32_t)((uint64_t)prod >> 16) + (uint32_t)((uint64_t)prod >> 15 & 1);

    return (int32_t)m >> 16;
}

/* The compass-like ring of the gauge: a vertical mark, then needles every 15 degrees up to the value. */
// FUNCTION: HGSIM 0x100510d9
void __cdecl Sim_100510d9(uint8_t *gauge)
{
    uint8_t *rect = BYTES(U32(gauge, 0xec));
    uint8_t *blk = BYTES(U32(PTRTAB(G32(0x1017b8c8)), 0x28));
    int32_t v, max, n, deg, k;
    int32_t colour = 0xc;

    v = Render_Ftol(FL(blk, 0x4e8));
    max = S32(gauge, 0x94);
    G32(0x1013bafc) = (uint32_t)Render_Ftol((double)(int32_t)((uint32_t)v * (uint32_t)max) / (double)GFL(0x1012b438));
    n = Sim_10047e95(SIM_PTR(int32_t, 0x1013baf8));
    Line((uint32_t)(uintptr_t)rect, HALF_W(rect), 0, HALF_W(rect), HALF_H(rect), SIM_PTR(u16a, 0x10165a10));
    if (n < max)
        deg = Degrees(n, max);
    else
        deg = 0x163;
    if (deg == 0)
        return;
    for (k = 0; k < deg; k += 0xf) {
        if (k > 0x78 && k < 0x10e)
            colour = 0x1f;
        else if (k >= 0x10e)
            colour = 4;
        Sim_10050e7e(rect, (float)(int32_t)((uint32_t)((k + 0x10e) % 0x168) << 16), colour,
                     BYTES(U32(G32(0x109e8948), 0xec)));
    }
    if (deg > 0x78 && deg < 0x10e)
        colour = 0x1f;
    else if (deg >= 0x10e)
        colour = 4;
    Sim_10050e7e(rect, (float)(int32_t)((uint32_t)((deg + 0x10e) % 0x168) << 16), colour,
                 BYTES(U32(G32(0x109e8948), 0xec)));
}

/* A horizontal bar (+0x7c x, +0x80 y, +0x84 width, +0x88 height) filled from both ends towards the value. */
// FUNCTION: HGSIM 0x10051372
void __cdecl Sim_10051372(uint8_t *gauge)
{
    uint32_t surface = U32(gauge, 0xec);
    uint8_t *blk = BYTES(U32(PTRTAB(G32(0x1017b8c8)), 0x28));
    int32_t v = Render_Ftol(FL(blk, 0x4e8));
    int32_t x = S32(gauge, 0x7c), y = S32(gauge, 0x80), width = S32(gauge, 0x84);
    int32_t height = (int32_t)(U32(gauge, 0x88) - 1);
    int32_t n, part, mid, x1, colour;

    G32(0x1013bafc) = (uint32_t)Render_Ftol((double)(int32_t)((uint32_t)v * (uint32_t)width) / (double)GFL(0x1012b438));
    n = Sim_10047e95(SIM_PTR(int32_t, 0x1013baf8));
    if (n >= width) {
        W8V_CALL_10051c90(surface, x, y, width, height, 0xb);
        return;
    }
    if (n <= 0) {
        W8V_CALL_10051c90(surface, x, y, width, height, 7);
        return;
    }
    if (n < width >> 1) {
        part = n;
        colour = 7;
    } else {
        part = width - n;
        colour = 0xb;
    }
    mid = width - part * 2;
    x1 = x + part + 1;
    W8V_CALL_10051c90(surface, x, y, part, height, 3);
    W8V_CALL_10051c90(surface, x1, y, mid, height, colour);
    W8V_CALL_10051c90(surface, x1 + mid, y, part, height, 3);
}

/* A bar (+0x9c x, +0xa0 y, +0xa4 width, +0xa8 height) for the value of block +0x4e0 - tick * block +0x4ec. */
// FUNCTION: HGSIM 0x10051532
void __cdecl Sim_10051532(uint8_t *gauge)
{
    uint32_t surface = U32(gauge, 0xec);
    uint8_t *blk = BYTES(U32(PTRTAB(G32(0x1017b8c8)), 0x28));
    int32_t fill = 0, n, first, second;

    G32(0x1013bb1c) =
        (uint32_t)Render_Ftol((double)FL(blk, 0x4e0) - (double)GS32(0x1015dc80) * (double)FL(blk, 0x4ec)) << 10;
    n = Sim_10047e95(SIM_PTR(int32_t, 0x1013bb18));
    if (n < 1) {
        first = 7;
        second = 7;
        n = 0;
    } else if (n < 0x300) {
        first = 3;
        second = 7;
    } else {
        first = 0xb;
        second = 3;
        n -= 0x300;
    }
    if (n > 0) {
        fill = Render_Ftol((double)(int32_t)(U32(gauge, 0xa4) * (uint32_t)n) / (double)GFL(0x1012b43c));
        if (fill < 0)
            fill = 0;
        else if (fill > S32(gauge, 0xa4))
            fill = S32(gauge, 0xa4);
    }
    if (fill > 0)
        W8V_CALL_10051c90(surface, S32(gauge, 0x9c), S32(gauge, 0xa0), fill, (int32_t)(U32(gauge, 0xa8) - 1), first);
    if (fill < S32(gauge, 0xa4))
        W8V_CALL_10051c90(surface, (int32_t)(U32(gauge, 0x9c) + (uint32_t)fill), S32(gauge, 0xa0),
                          (int32_t)(U32(gauge, 0xa4) - (uint32_t)fill), (int32_t)(U32(gauge, 0xa8) - 1), second);
}

/* Two images: a background, then the one of 22 (or of 14) picked by the float at +8 of the unit's record. */
// FUNCTION: HGSIM 0x100516c8
void __cdecl Sim_100516c8(uint8_t *gauge)
{
    uint32_t surface = U32(gauge, 0xec);
    uint8_t *blk = BYTES(U32(PTRTAB(G32(0x1017b8c8)), 0x28));
    int32_t k;

#define INFO BYTES(U32(U32(blk, 0), 0x54))
    if ((int8_t)U8(INFO, 0x59) == 0) {
        W8V_CALL_100652e8(0, 0, G32(0x109e84bc), surface);
        k = Render_Ftol((double)FL(INFO, 8) * (double)GFL(0x1012b440) / (double)GFL(0x1012b444));
        if (k < 0)
            k = 0;
        else if (k > 0x15)
            k = 0x15;
        W8V_CALL_100652e8(0, 0, SIM_PTR(u32a, 0x109e84bc)[k], surface);
    } else {
        W8V_CALL_100652e8(0, 0, G32(0x109e8510), surface);
        k = Render_Ftol((double)FL(INFO, 8) * (double)GFL(0x1012b448) / (double)GFL(0x1012b444));
        if (k <= 0)
            k = 0;
        else if (k > 0xd)
            k = 0xd;
        W8V_CALL_100652e8(0, 0, SIM_PTR(u32a, 0x109e8514)[k], surface);
    }
#undef INFO
}

/* A frame (+0x6c left, +0x70 top, +0x74 right, +0x78 bottom) and, when block +0x510 is not negative, a column. */
// FUNCTION: HGSIM 0x100517dc
void __cdecl Sim_100517dc(uint8_t *gauge)
{
    uint32_t surface = U32(gauge, 0xec);
    uint8_t *blk = BYTES(U32(PTRTAB(G32(0x1017b8c8)), 0x28));
    const u16a *c16 = SIM_PTR(u16a, 0x101659f4);
    int32_t n, h;

    Line(surface, S32(gauge, 0x6c), S32(gauge, 0x70), S32(gauge, 0x6c), S32(gauge, 0x78), c16);
    Line(surface, S32(gauge, 0x74), S32(gauge, 0x70), S32(gauge, 0x74), S32(gauge, 0x78), c16);
    Line(surface, S32(gauge, 0x6c), S32(gauge, 0x70), S32(gauge, 0x74), S32(gauge, 0x70), c16);
    Line(surface, S32(gauge, 0x6c), S32(gauge, 0x78), S32(gauge, 0x74), S32(gauge, 0x78), c16);
    if (S32(blk, 0x510) < 0)
        return;
    G32(0x1013bb2c) = U32(blk, 0x510);
    n = Sim_10047e95(SIM_PTR(int32_t, 0x1013bb28));
    h = Render_Ftol((double)(int32_t)(U32(gauge, 0x68) + (uint32_t)n) / (double)GFL(0x1012b44c));
    if (h > S32(gauge, 0x68))
        h = S32(gauge, 0x68);
    Sim_10051a36(surface, S32(gauge, 0x5c), S32(gauge, 0x60), (int32_t)(U32(gauge, 0x64) - 1), h, 0xb);
}
