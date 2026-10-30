#include "w8x.h"
#include "w5y.h"
#include "w6f.h"
#include "w4u.h"
#include "w3q.h"
#include "../unsorted_1000/fns.h"
#include "../unsorted_1000/w5x.h"
#include "../unsorted_1001/w5x.h"
#include "../util/access.h"
#include "../util/util.h"
#include "../util/w3o.h"
#include "../view/view.h"

#define W8X_ADDR(a) ((uint32_t)(uintptr_t)SIM_PTR(uint8_t, (a)))
#define CAMERA BYTES(G32(0x1014cae0))

typedef void(__cdecl *W8xDriverFn)(void);
/* The display driver's routine +0x18 (the driver table [0x1015e68c]). */
#define W8X_DRIVER_18() ((*(W8xDriverFn *)(BYTES(G32(0x1015e68c)) + 0x18))())

/* (value << 16) / divisor as the original's cdq / shld / idiv gives it (the quotient fits in 32 bits). */
static int32_t W8x_DivShift16(int32_t value, int32_t divisor)
{
    uint32_t a = value < 0 ? 0u - (uint32_t)value : (uint32_t)value;
    uint32_t d = divisor < 0 ? 0u - (uint32_t)divisor : (uint32_t)divisor;
    uint64_t num = (uint64_t)a << 16, rem = 0, q = 0;
    int bit;

    for (bit = 47; bit >= 0; bit--) {
        rem = (rem << 1) | ((num >> bit) & 1);
        if (rem >= d) {
            rem -= d;
            q |= (uint64_t)1 << bit;
        }
    }
    return ((value < 0) != (divisor < 0)) ? (int32_t)(0u - (uint32_t)q) : (int32_t)(uint32_t)q;
}

/* The pane colour: through the word table 0x101659e0 in the high-colour mode ([0x1017b940]). */
static int W8x_Line(void *pane, uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1, uint32_t colour)
{
    if (G32(0x1017b940) != 0)
        return W8X8_CALL_100ef727(pane, x0, y0, x1, y1, 0, SIM_PTR(u16a, 0x101659e0)[colour]);
    return W8X8_CALL_100e86a1(pane, x0, y0, x1, y1, 0, colour);
}

static int W8x_Shape(void *pane, void *shape, uint32_t number, uint32_t x, uint32_t y)
{
    if (G32(0x1017b940) != 0)
        return W8X8_CALL_100f0205(pane, shape, number, x, y);
    return W8X8_CALL_100e91dc(pane, shape, number, x, y);
}

/* Fits the rectangle 0x1014b938 to 0x109e3e20 the first time ([0x1014b94c]) and draws the shape at (0, 0). */
static int W8x_ShowShape(void *shape, int checkShape)
{
    if (G32(0x1014b94c) != 0 && (!checkShape || shape != 0)) {
        G32(0x1014b938) = W8X_ADDR(0x109e3e20);
        View_ScaleRect(SIM_PTR(void, 0x109e3e20), SIM_PTR(int32_t, 0x1014b938), SIM_PTR(int32_t, 0x1014b938));
        Sim_1008d2d0(SIM_PTR(int32_t, 0x1014b938), SIM_PTR(int32_t, 0x1014b938), shape, 0);
        G32(0x1014b94c) = 0;
    }
    return W8x_Shape(SIM_PTR(void, 0x1014b938), shape, 0, 0, 0);
}

// FUNCTION: HGSIM 0x1008b6a0
int __cdecl Sim_1008b6a0(void)
{
    void *shape = W8X8_CALL_100ba7d5((void *)(uintptr_t)G32(0x109e85c4), (void *)(uintptr_t)G32(0x1014cec0), 0);

    if (shape == 0)
        return 0;
    return W8x_ShowShape(shape, 1);
}

// FUNCTION: HGSIM 0x1008b78f
void __cdecl Sim_1008b78f(void)
{
    void *shape;

    if (W8X8_CALL_100766ca(5) != 0 || G32(0x1015ecd0) != 0)
        return;
    Sim_1008b8de();
    Sim_1000e983((const char *)GBYTES(0x1014b968), 0, 0, Sim_1000edce());
    shape = W8X8_CALL_100282d4(G32(0x1014cf8c), G32(0x101c64f0) + 0x74, G32(0x1014cec0), 0);
    if (shape != 0) {
        W8x_ShowShape(shape, 0);
        if (G32(0x1017b920) != 0)
            W8X_DRIVER_18();
    }
    *SIM_PTR(u16a, 0x1015fd4c) = 0;
    while (*SIM_PTR(s16a, 0x1015fd4c) == 0) {
        W8X8_CALL_10055048();
        Sim_1001f454();
    }
    *SIM_PTR(u16a, 0x1015fd4c) = 0;
    Sim_1000e983((const char *)GBYTES(0x1014b974), 0xffffec78u, 0, Sim_1000edce());
    Sim_1008b900();
}

// FUNCTION: HGSIM 0x1008d65d
int __cdecl Sim_1008d65d(void *pane, uint32_t unused, int32_t y, void *font, uint32_t colour)
{
    uint32_t line = (uint32_t)y + (uint32_t)W8X8_CALL_100ecce0(font);
    uint32_t width = U32(pane, 0xc) - U32(pane, 4) + 1;

    (void)unused;
    return W8x_Line(pane, 0, line, width - 1, line, colour);
}

/* The summed character widths of a text (two-byte characters stepped over as one in the [0x101668e8] mode). */
static int32_t W8x_TextWidth(const uint8_t *text, void *font)
{
    int32_t width = 0;

    while (*text != 0) {
        uint32_t c = *text;
        int two = 0;

        if (G32(0x101668e8) != 0)
            width += (int32_t)Sim_100f2cec(font, c);
        else
            width += W8X8_CALL_100eccf3(font, c);
        if (G32(0x101668e8) != 0) {
            c = *text;
            if (c < 0x81 || c == 0x85 || c == 0x86 || c == 0xeb || c == 0xec)
                two = 0;
            else if ((c & 0xf0) > 0x90 && (c & 0xf0) < 0xe0)
                two = 0;
            else
                two = c <= 0xee || c >= 0xfa;
        }
        text += two ? 2 : 1;
    }
    return width;
}

// FUNCTION: HGSIM 0x1008d6f7
int __cdecl Sim_1008d6f7(void *pane, const uint8_t *text, int32_t x, int32_t y, void *font, uint32_t colour)
{
    uint32_t line = (uint32_t)y + (uint32_t)W8X8_CALL_100ecce0(font);
    int32_t width = W8x_TextWidth(text, font);

    return W8x_Line(pane, (uint32_t)x, line, (uint32_t)x + (uint32_t)width - 1, line, colour);
}

// FUNCTION: HGSIM 0x1008d8c0
int __cdecl Sim_1008d8c0(void *pane, const uint8_t *text, int32_t x, int32_t y, void *font, uint32_t colour)
{
    int32_t height = W8X8_CALL_100ecce0(font);
    int32_t width = W8x_TextWidth(text, font);
    uint32_t x0 = (uint32_t)x, y0 = (uint32_t)y - 1;
    uint32_t x1 = (uint32_t)x + (uint32_t)width, y1 = (uint32_t)y + (uint32_t)height + 1;

    W8x_Line(pane, x0, y0, x1, y0, colour);
    W8x_Line(pane, x0, y1, x1, y1, colour);
    W8x_Line(pane, x0, y0, x0, y1, colour);
    return W8x_Line(pane, x1, y0, x1, y1, colour);
}

// FUNCTION: HGSIM 0x1008de0a
int32_t *__cdecl Sim_1008de0a(const char *text, void *font, int32_t *out)
{
    int32_t widest = 0, total, line = 0, height;
    const char *p;

    if (out == 0 || text == 0 || font == 0)
        return 0;
    p = text;
    height = W8X8_CALL_100ecce0(font);
    total = height;
    for (;;) {
        signed char c = (signed char)*p++;

        if (c == 0)
            break;
        if (c == '\n') {
            if (line > widest)
                widest = line;
            line = 0;
            total = (int32_t)((uint32_t)total + (uint32_t)height);
        } else if (G32(0x101668e8) != 0) {
            line = (int32_t)((uint32_t)line + Sim_100f2cec(font, (uint32_t)(int32_t)c));
        } else {
            line = (int32_t)((uint32_t)line + (uint32_t)W8X8_CALL_100eccf3(font, (uint32_t)(int32_t)c));
        }
    }
    if (line > widest)
        widest = line;
    S32(out, 8) = 0;
    S32(out, 4) = 0;
    S32(out, 0xc) = W8x_DivShift16(widest, (int32_t)(0x10000u - G32(0x1014b9a8)));
    S32(out, 0x10) = W8x_DivShift16(total, (int32_t)(0x10000u - G32(0x1014b9ac)));
    return out;
}

// FUNCTION: HGSIM 0x1008df64
int __cdecl Sim_1008df64(void *a, float x, float y)
{
    int32_t pt[2];
    void *pane;
    int32_t px, py;
    int ret = 1;

    if (a == 0)
        return 1;
    pane = (void *)(uintptr_t)U32(a, 0);
    if (pane == 0)
        return 1;
    px = Render_Ftol((double)x);
    py = Render_Ftol((double)y);
    pt[0] = (int32_t)((uint32_t)(px / 2) + 0x8000u);
    pt[1] = (int32_t)(0x8000u - (uint32_t)(py / 2));
    Rect_ScalePoint((const int32_t *)pane, pt, pt);
    W8X8_CALL_100e84f0(pane, (uint32_t)pt[0], (uint32_t)pt[1], U8(a, 4));
    if (G32(0x1017b920) != 0)
        W8X_DRIVER_18();
    if (W8X8_CALL_10057f83() == 0x1b)
        ret = 0;
    return ret;
}

// FUNCTION: HGSIM 0x1008e0fe
void __cdecl Sim_1008e0fe(void *pane, void *shape, uint32_t number)
{
    uint32_t size, x = 0, y = 0;
    int32_t w, h, nx, ny, i, j;

    if (pane == 0 || shape == 0)
        return;
    size = W8X8_CALL_100ed743(shape, number);
    w = (int32_t)size >> 16;
    h = (int32_t)(size & 0xffff);
    nx = (int32_t)(U32(pane, 0xc) - U32(pane, 4) + (uint32_t)w) / w;
    ny = (int32_t)(U32(pane, 0x10) - U32(pane, 8) + (uint32_t)h) / h;
    for (j = 0; j < ny; j++) {
        for (i = 0; i < nx; i++) {
            W8x_Shape(pane, shape, number, x, y);
            x += (uint32_t)w;
        }
        x = 0;
        y += (uint32_t)h;
    }
}

/* Half the pane's width and height ((right - left + 1) >> 1, (bottom - top + 1) >> 1). */
static void W8x_Centre(const int32_t *rect, int32_t *c)
{
    c[0] = (int32_t)(U32(rect, 0xc) - U32(rect, 4) + 1) >> 1;
    c[1] = (int32_t)(U32(rect, 0x10) - U32(rect, 8) + 1) >> 1;
}

// FUNCTION: HGSIM 0x1008e320
int32_t *__cdecl Sim_1008e320(const int32_t *rect, float angle, int32_t *out)
{
    int32_t c[2];
    uint32_t flags = 0;
    float a, s, k, slope = 0.0f;

    a = (float)(W8X8_CALL_100fcb30(W8X8_CALL_100fcb30((double)angle, 360.0) + GF64(0x1012b918), 360.0));
    W8x_Centre(rect, c);
    flags = (flags & ~1u) | ((a >= GFL(0x1012b920) && !(a > GFL(0x1012b924))) ? 1u : 0u);
    flags = (flags & ~2u) | ((!(a > GFL(0x1012b928)) ? 1u : 0u) << 1);
    s = (float)W8X8_CALL_10084766((float)((double)a + (double)GFL(0x1012b920)));
    k = (float)W8X8_CALL_10084766(a);
    k = (float)((double)k * (double)FL(CAMERA, 0x44));
    /* the slope is left unset when Sim_1008e211 reports a small sine; the modes with bit 2 set do not read it */
    flags = (flags & ~4u) | (((uint32_t)Sim_1008e211(s, k, &slope) & 1) << 2);
    slope = (float)(-(double)slope);
    Sim_1008e489(c, flags, slope, out);
    return out;
}

/* The step count n = (half width - 1) * (f / [0x1012b92c]) and n * camera +0x44, both truncated. */
static void W8x_Steps(const int32_t *rect, float f, int32_t *n, int32_t *m)
{
    int32_t half = (int32_t)(U32(rect, 0xc) - U32(rect, 4) + 1) / 2 - 1;
    float q = (float)((double)f / (double)GFL(0x1012b92c));

    *n = Render_Ftol((double)half * (double)q);
    *m = Render_Ftol((double)*n * (double)FL(CAMERA, 0x44));
}

// FUNCTION: HGSIM 0x1008e707
int __cdecl Sim_1008e707(const int32_t *rect, uint32_t c, float f)
{
    int32_t centre[2], n, m;

    W8x_Centre(rect, centre);
    W8x_Steps(rect, f, &n, &m);
    return W8X8_CALL_100eb6e8(rect, centre[0], centre[1], n, m, c);
}

// FUNCTION: HGSIM 0x1008e7a1
int __cdecl Sim_1008e7a1(const int32_t *rect, uint32_t c, float f)
{
    int32_t centre[2], n, m;

    W8x_Centre(rect, centre);
    W8x_Steps(rect, f, &n, &m);
    return W8X8_CALL_100eba29(rect, centre[0], centre[1], n, m, c);
}

// FUNCTION: HGSIM 0x1008e83b
int __cdecl Sim_1008e83b(const int32_t *rect, const int32_t *box, uint32_t c)
{
    int32_t centre[2], n, m;
    uint32_t moved[5];

    W8x_Centre(rect, centre);
    n = (int32_t)(U32(rect, 0xc) - U32(rect, 4) + 1) / 2 - 1;
    m = Render_Ftol((double)n * (double)FL(CAMERA, 0x44));
    moved[0] = U32(rect, 0);
    moved[1] = U32(box, 0) + U32(rect, 4);
    moved[2] = U32(box, 4) + U32(rect, 8);
    moved[3] = U32(box, 8) + U32(rect, 4);
    moved[4] = U32(box, 0xc) + U32(rect, 8);
    centre[0] = (int32_t)((uint32_t)centre[0] - U32(box, 0));
    centre[1] = (int32_t)((uint32_t)centre[1] - U32(box, 4));
    return W8X8_CALL_100eba29(moved, centre[0], centre[1], n, m, c);
}

// FUNCTION: HGSIM 0x1008e9af
int32_t *__cdecl Sim_1008e9af(const int32_t *rect, const int32_t *pt, int32_t *out)
{
    int32_t c[2], dx, dy, q;
    uint32_t flags = 0, big;
    float slope;

    W8x_Centre(rect, c);
    dx = (int32_t)((uint32_t)c[0] - U32(pt, 0));
    if (dx >= 0)
        flags |= 1;
    dy = (int32_t)((uint32_t)c[1] - U32(pt, 4));
    if (dy >= 0)
        flags |= 2;
    big = 1;
    if (dx != 0) {
        q = dy / dx;
        if (q <= 0x7fff && q >= -0x8000)
            big = 0;
    }
    flags = (flags & ~4u) | (big << 2);
    dy = Render_Ftol((double)dy / (double)FL(CAMERA, 0x44));
    slope = (float)W8X8_CALL_1008486e((float)dy, (float)dx);
    W8X8_CALL_1008ec46(rect, c, flags, Bits_OfFloat(slope), out);
    return out;
}
