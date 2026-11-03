#include "w10.h"
#include "w5x.h"
#include "w8z.h"
#include "w7q.h"
#include "../simmem.h"
#include "../util/access.h"
#include "../util/util.h"
#include "../unsorted_1008/w3o.h"
#include "../unsorted_100d/w5c.h"
#include "../unsorted_1005/w8v.h"
#include "../unsorted_100b/w6g.h"
#include "../render/w2n.h"
#include "../unsorted_100f/w8z.h"
#include "../libm/hg_math.h"

#define GF(addr) (*SIM_PTR(float, addr))

static uint32_t W10c_Bits(float f)
{
    union {
        float f;
        uint32_t u;
    } v;

    v.f = f;
    return v.u;
}

// FUNCTION: HGSIM 0x100cbe94
void __cdecl Sim_100cbe94(void)
{
    uint32_t unit = PTRTAB(G32(0x1017b8c8));
    uint32_t old = U32(unit, 0xf0);
    int flag = U32(U32(unit, 0x28), 0x50c) == 1;
    uint32_t seen = 0, best = 0xffffffffu, sel;
    float bestDist = 3.4e38f;          /* 0x7f7fc99e */

    Sim_100cbe68(0);
    while (seen != best) {
        sel = U32(unit, 0xf0);
        seen = sel;
        if (sel & 0x1000)
            break;
        if (!(*(float *)(BYTES(unit) + 0xd8) >= bestDist)) {     /* fcomp: unordered counts as below */
            best = sel;
            bestDist = *(float *)(BYTES(unit) + 0xd8);
            seen = 0;
        }
        Sim_100cbe68(1);
    }
    if (best == 0xffffffffu || (double)bestDist > (double)GF(0x1012bc48)) {      /* NaN is kept */
        U32(unit, 0xf0) = old;
        if (flag)
            U8(U32(unit, 0x54), 0x46) = 0;
        return;
    }
    U32(unit, 0xf0) = best;
    if (Sim_100cb4fb((void *)(uintptr_t)unit) == 0) {
        U32(unit, 0xf0) = old;
        if (flag)
            U8(U32(unit, 0x54), 0x46) = 0;
    }
}

typedef double(__cdecl *W10cSqrtFn)(double);

// FUNCTION: HGSIM 0x100c97c0
float __cdecl Sim_100c97c0(void *obj, void *wpn)
{
    uint32_t rec = U32(obj, 0x28);
    const uint8_t *kind = GBYTES(0x10145508) + U32(wpn, 4) * 0xd6u;
    float x, y, z, px, py, pz, vy, g, h, t;
    double s53;

    U32(obj, 0x50) = U32(rec, U32(wpn, 0x11c) * 4u + 0x498u);
    if (obj == 0 || U32(obj, 0x50) == 0)
        return GF(0x1012bc2c);
    Sim_100d6ea7(obj, &x, &y, &z, wpn);
    Sim_10084c95(&x, &y, &z);
    vy = *(const float *)(kind + 0x3c) * y;
    /* x87 at 53-bit precision: every step rounds to double, each fstp to float, as here */
    g = (float)((double)-GF(0x1013abc0) * (double)S32(kind, 0x38) * (double)GF(0x1012bbf8));
    Sim_100a9cfb((void *)(uintptr_t)U32(obj, 0x50), (uint32_t *)&px, (uint32_t *)&py, (uint32_t *)&pz);
    h = py - *(const float *)((const uint8_t *)obj + 0x7c);
    s53 = hg_sqrt((double)vy * (double)vy -
                                         (double)GF(0x1012bbf4) * (double)g * (double)h);
    t = (float)(((double)-vy - s53) / ((double)GF(0x1012bc30) * (double)g));
    if (W10c_Bits(t) & 0x80000000u) {
        s53 = hg_sqrt((double)vy * (double)vy -
                                             (double)GF(0x1012bbf4) * (double)g * (double)h);
        t = (float)((s53 + (double)-vy) / ((double)GF(0x1012bc30) * (double)g));
    }
    return t;
}

typedef void(__cdecl *W10cMethodFn)(void *pane, uint32_t text);
typedef void *(__cdecl *W10cLoadFn)(void *name, void *key, uint32_t zero);
typedef void(__cdecl *W10cTextFn)(uint32_t target, uint32_t x, uint32_t y, void *font, const char *text,
                                   void *colours);

/* The common part of Sim_100cfff3 and Sim_100cfe48 after their first call: the state text, the pane text at
 * the position of pane+0xf0 (own) or of [0x109e8934]+0xf0, and the font. */
static void W10c_StateText(uint8_t *pane, uint32_t rec, int ownPos)
{
    W10cMethodFn method;
    void *font;
    uint32_t pos, x, y, target;
    const char *text;

    font = SIM_FN(W10cLoadFn, 0x100ba7d5)((void *)(uintptr_t)G32(0x109b9900), (void *)(uintptr_t)G32(0x1014cec4), 0);
    if (font == 0)
        return;
    method = (W10cMethodFn)(uintptr_t)U32(pane, 0x114);
    if (U16(rec, 0x560) & 4) {
        method(pane, (U16(rec, 0x560) & 8) ? G32(0x101c4c58) : G32(0x101c4c50));
        U8(GBYTES(0x109b992e), 0) = 7;
    } else {
        method(pane, G32(0x101c4c60));
        U8(GBYTES(0x109b992e), 0) = 0xc;
    }
    text = (const char *)pane + 0x10;
    pos = ownPos ? U32(pane, 0xf0) : U32(G32(0x109e8934), 0xf0);
    y = U32(pos, 4);
    x = U32(pos, 0);
    target = U32(pane, 0xec);
    if (G32(0x101668e8) != 0)
        SIM_FN(W10cTextFn, 0x100f2f01)(target, x, y, font, text, SIM_PTR(void, 0x109b9920));
    else
        SIM_FN(W10cTextFn, 0x100f1e10)(target, x, y, font, text, SIM_PTR(void, 0x109b9920));
    U8(GBYTES(0x109b992e), 0) = 0xc;
    Sim_100bb2b0((const uint32_t *)(uintptr_t)G32(0x109b9900), (const char *)(uintptr_t)G32(0x1014cec4));
}

// FUNCTION: HGSIM 0x100cfff3
void __cdecl Sim_100cfff3(uint8_t *pane)
{
    uint32_t rec = U32(PTRTAB(G32(0x1017b8c8)), 0x28);

    if (S16(pane, 4) == 0)
        return;
    Sim_100510d9(pane);
    W10c_StateText(pane, rec, 1);
}

// FUNCTION: HGSIM 0x100cfe48
void __cdecl Sim_100cfe48(uint8_t *pane)
{
    uint32_t rec = U32(PTRTAB(G32(0x1017b8c8)), 0x28);

    if (S16(pane, 4) == 0)
        return;
    Sim_10051372(pane);
    W10c_StateText(pane, rec, 0);
}

typedef int32_t(__cdecl *W10cHeightFn)(void *font);
typedef void(__cdecl *W10cText2Fn)(uint32_t target, int32_t x, int32_t y, void *font, const char *text,
                                    void *colours);

static void W10c_Text(uint32_t target, int32_t x, int32_t y, void *font, const char *text)
{
    if (G32(0x101668e8) != 0)
        SIM_FN(W10cText2Fn, 0x100f2f01)(target, x, y, font, text, SIM_PTR(void, 0x109b9920));
    else
        SIM_FN(W10cText2Fn, 0x100f1e10)(target, x, y, font, text, SIM_PTR(void, 0x109b9920));
}

// FUNCTION: HGSIM 0x100cf96c
void __cdecl Sim_100cf96c(uint8_t *pane)
{
    uint32_t rec;
    char text[0x44];     /* the original's buffer at ebp-0x48 (up to the next local at ebp-4) */
    void *font;
    int32_t speed, h, y, x;
    uint32_t pos;

    if (S16(pane, 4) == 0)
        return;
    rec = U32(PTRTAB(G32(0x1017b8c8)), 0x28);
    /* x87 at 53-bit precision: each product rounds to double, as here */
    speed = Render_Ftol(Sim_10084a94(*(float *)(BYTES(rec) + 0x548), *(float *)(BYTES(rec) + 0x54c),
                                     *(float *)(BYTES(rec) + 0x550)) *
                        (double)GF(0x1012bc68) * (double)GF(0x1012bc6c));
    if (!(*(float *)(BYTES(rec) + 0x2c) >= GF(0x1012bc70))) {
        speed = (int32_t)(0u - (uint32_t)speed);
        U8(GBYTES(0x109b992e), 0) = 0x14;
    } else {
        U8(GBYTES(0x109b992e), 0) = 0xc;
    }
    Sim_100516c8(pane);
    font = SIM_FN(W10cLoadFn, 0x100ba7d5)((void *)(uintptr_t)G32(0x109b9900), (void *)(uintptr_t)G32(0x1014cec4), 0);
    if (font == 0)
        return;
    h = SIM_FN(W10cHeightFn, 0x100ecce0)(font);
    Crt_SPrintf(text, SIM_PTR(const char, 0x10151e70), speed);     /* "%3d" */
    pos = U32(G32(0x109e8928), 0xf0);
    W10c_Text(U32(pane, 0xec), S32(pos, 0), S32(pos, 4), font, text);
    pos = U32(G32(0x109e8928), 0xf0);
    W10c_Text(U32(pane, 0xec), S32(pos, 0), (int32_t)(U32(pos, 4) + (uint32_t)h), font,
              (const char *)(uintptr_t)G32(0x101c4c6c));
    y = Render_Ftol((double)GS32(0x109e3e1c) * *SIM_PTR(double, 0x1012bc78));
    if (U16(rec, 0x562) & 1) {
        if (U32(rec, 0x613) == 0) {
            W10c_Text((uint32_t)(uintptr_t)GBYTES(0x109e3e00), 1, y, font, (const char *)(uintptr_t)G32(0x101c4c68));
        } else {
            U8(GBYTES(0x109b992e), 0) = 4;
            x = Render_Ftol((double)GS32(0x109e3de8) * *SIM_PTR(double, 0x1012bc80));
            W10c_Text((uint32_t)(uintptr_t)GBYTES(0x109e3e00), x, y, font, (const char *)(uintptr_t)G32(0x101c4c68));
        }
    }
    U8(GBYTES(0x109b992e), 0) = 9;
    Sim_100bb2b0((const uint32_t *)(uintptr_t)G32(0x109b9900), (const char *)(uintptr_t)G32(0x1014cec4));
}


typedef void(__cdecl *W10cLineFn)(void *surface, int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t zero,
                                  uint32_t colour);

/* one line on the surface 0x109e3e00: 0x100ef727 with the colour word */
static void W10c_Line(int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t word_addr)
{
    SIM_FN(W10cLineFn, 0x100ef727)(SIM_PTR(void, 0x109e3e00), x1, y1, x2, y2, 0, *SIM_PTR(u16a, word_addr));
}

/* a chevron of three lines at (x, y), opening up (s = 1: y + 2 / y + 5) or down (s = -1) */
static void W10c_Chevron(int32_t x, int32_t y, int32_t s, uint32_t word_addr)
{
    W10c_Line(x - 5, y + 2 * s, x + 5, y + 2 * s, word_addr);
    W10c_Line(x + 5, y + 2 * s, x, y + 5 * s, word_addr);
    W10c_Line(x, y + 5 * s, x - 5, y + 2 * s, word_addr);
}

// FUNCTION: HGSIM 0x100c83ed
void __cdecl Sim_100c83ed(float rate, int32_t x, int32_t y)
{
    double mid = *SIM_PTR(double, 0x1012bc18);

    if (!((double)rate >= mid)) {
        W10c_Chevron(x, y, 1, 0x101659f4);
        if (G32(0x1017e578) != 0)
            W10c_Chevron(x, y, -1, 0x101659e8);
    } else if ((double)rate > mid) {
        W10c_Chevron(x, y, -1, 0x101659f4);
        if (G32(0x1017e578) != 0)
            W10c_Chevron(x, y, 1, 0x101659e8);
    } else {
        W10c_Chevron(x, y, -1, 0x101659e8);
        W10c_Chevron(x, y, 1, 0x101659e8);
    }
    G32(0x1017e578) = (uint32_t)((int32_t)(G32(0x1017e578) + 1) % 2);
}

typedef void(__cdecl *W10cGaugeTextFn)(void *surface, int32_t x, int32_t y, void *font, const char *text,
                                       void *colours);
typedef void *(__cdecl *W10cGaugeLoadFn)(uint32_t name, uint32_t ext, uint32_t zero);

/* the weapon label box at (x, y) with its pointer line */
static void W10c_GaugeBox(int32_t x, int32_t y)
{
    W10c_Line(x, y + 10, x + 20, y + 10, 0x101659f4);
    W10c_Line(x + 20, y + 10, x + 20, y + 12, 0x101659f4);
    W10c_Line(x + 20, y + 12, x, y + 12, 0x101659f4);
    W10c_Line(x, y + 12, x, y + 10, 0x101659f4);
    W10c_Line(x + 20, y + 12, x + 32, y - 4, 0x101659f0);
}

/* the text with the font [0x109b98c0] by the display mode (0x100f2f01 / 0x100f1e10 / 0x100ecea6), colours 0x109b9920 */
static void W10c_GaugeText(int32_t x, int32_t y, void *font, const char *text)
{
    if (G32(0x101668e8) != 0)
        SIM_FN(W10cGaugeTextFn, 0x100f2f01)(SIM_PTR(void, 0x109e3e00), x, y, font, text, SIM_PTR(void, 0x109b9920));
    else
        SIM_FN(W10cGaugeTextFn, 0x100f1e10)(SIM_PTR(void, 0x109e3e00), x, y, font, text, SIM_PTR(void, 0x109b9920));
    *GBYTES(0x109b992e) = 9;
    Sim_100bb2b0((const uint32_t *)(uintptr_t)G32(0x109b98c0), (const char *)(uintptr_t)G32(0x1014cec4));
}

// FUNCTION: HGSIM 0x100c8ba7
void __cdecl Sim_100c8ba7(uint8_t *obj, int32_t x, int32_t y, const uint8_t *weapon)
{
    char text[0x50];          /* the original's 0x50-byte buffer at ebp-0x50 */
    uint8_t *rec = BYTES(U32(PTRTAB(G32(0x1017b8c8)), 0x28));
    uint32_t sel = U32(obj, 0xf0), mode, slot;
    float range, off;
    void *font;
    int32_t w;
    double d;

    if (sel == 0 || (sel & 0x1000) != 0)
        return;
    mode = sel & 0xf00;
    if (!((mode == 0x200 || mode == 0x400) && U8(weapon, 0xb2) != 0))
        return;
    slot = U32(rec, 0x84) + U32(rec, 0x4fc) * 0x128u;
    if (S32(slot, 0x20) == -2)
        return;
    if ((U32(weapon, 0x86) & 0x40) != 0) {
        /* guided weapon: range to the target, the closing rate arrows and the range text */
        range = (float)Sim_100c9572(obj, (int)mode, (int)(sel & 0xff));
        font = SIM_FN(W10cGaugeLoadFn, 0x100ba7d5)(G32(0x109b98c0), G32(0x1014cec4), 0);
        d = (double)range - (double)*(float *)(obj + 0xd8) / *SIM_PTR(float, 0x1012bc20);
        off = (float)d;
        if (d > (double)*SIM_PTR(float, 0x1012bc24) && !(off >= *SIM_PTR(float, 0x1012bc28))) {
            Sim_100c83ed(0.0f, x - 10, y);
            *GBYTES(0x109b992e) = 7;
        } else {
            Sim_100c83ed(off, x - 10, y);
            *GBYTES(0x109b992e) = 0xf;
        }
        W10c_GaugeBox(x, y);
        if (!((double)range * *SIM_PTR(float, 0x1012bc20) >= (double)*(const float *)(weapon + 0x50))) {
            if (range > *SIM_PTR(float, 0x1012bc2c))
                Crt_SPrintf(text, SIM_PTR(const char, 0x10151d20), (double)range);
            else
                Crt_SPrintf(text, SIM_PTR(const char, 0x10151d1c));
        } else {
            Crt_SPrintf(text, (const char *)(uintptr_t)G32(0x101c47c4));
        }
        x -= 5;
        W10c_GaugeText(x, y, font, text);
        return;
    }
    if ((U32(weapon, 0x86) & 1) != 0) {
        /* locking weapon: the lock state text of the own unit's record */
        if ((U16(rec, 0x560) & 0x80) != 0)
            Crt_SPrintf(text, (const char *)(uintptr_t)G32(0x101c47d0));
        else if ((U16(rec, 0x560) & 0x40) != 0)
            Crt_SPrintf(text, (const char *)(uintptr_t)G32(0x101c47cc));
        else
            return;
        W10c_GaugeBox(x, y);
        font = SIM_FN(W10cGaugeLoadFn, 0x100ba7d5)(G32(0x109b98c0), G32(0x1014cec4), 0);
        *GBYTES(0x109b992e) = 7;
        x -= 10;
        W10c_GaugeText(x, y, font, text);
        return;
    }
    /* out of range: the warning text every third frame */
    range = *(float *)(obj + 0xd8);
    if (!(range > *(const float *)(weapon + 0x50)))
        return;
    if (G32(0x1017e574) == 1) {
        font = SIM_FN(W10cGaugeLoadFn, 0x100ba7d5)(G32(0x109b98c0), G32(0x1014cec4), 0);
        *GBYTES(0x109b992e) = 7;
        Crt_SPrintf(text, (const char *)(uintptr_t)G32(0x101c47c4));
        w = Sim_100ccb0d(text, font);
        x += 0x3c;
        y -= 0x1e;
        W10c_Line(x + 20, y - 10, x + w + 20, y - 10, 0x101659f4);
        W10c_Line(x, y, x + 20, y - 10, 0x101659f4);
        W10c_GaugeText(x + 20, y - 20, font, text);
    }
    G32(0x1017e574) = (uint32_t)((int32_t)(G32(0x1017e574) + 1) % 3);
}