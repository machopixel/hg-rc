#include "w30.h"
#include "../util/access.h"

/*
 * The sim's 2D drawing library (a Miles VFX style library, the 16-bit set), converted from its assembly. A window
 * is {buffer, row pitch in bytes - 1, height - 1} (the sim keeps the pitch where VFX kept "width - 1"); a pane is
 * {window, x0, y0, x1, y1}. Pixels are RGB565 words. Each routine draws exactly the original's cells into the
 * pane's buffer; with the full-resolution HUD on (W30_HudBegin / W30_HudEnd, w30_hud.c) the same drawing also goes
 * to the renderer.
 */

#define W30_WIN(pane) BYTES(U32(pane, 0))

/* The pane's clip box in window cells: x0 / y0 at least 0, x1 / y1 at most the window's "width" / height - 1.
 * Returns -1 for an empty window, -2 for an empty box, 0 otherwise. */
static int32_t W30_Clip(const void *pane, int32_t *w, int32_t *x0, int32_t *y0, int32_t *x1, int32_t *y1)
{
    uint8_t *win = W30_WIN(pane);
    int32_t h;

    *w = S32(win, 4) + 1;
    if (*w <= 0)
        return -1;
    h = S32(win, 8) + 1;
    if (h <= 0)
        return -1;
    *x0 = S32(pane, 4) > 0 ? S32(pane, 4) : 0;
    *y0 = S32(pane, 8) > 0 ? S32(pane, 8) : 0;
    *x1 = S32(pane, 0xc) < *w - 1 ? S32(pane, 0xc) : *w - 1;
    *y1 = S32(pane, 0x10) < h - 1 ? S32(pane, 0x10) : h - 1;
    if (*x1 < *x0 || *y1 < *y0)
        return -2;
    return 0;
}

/* high dword of a * b + 0x80000000: a times a 0.32 slope, rounded */
static uint32_t W30_MulRound(uint32_t a, uint32_t slope)
{
    return (uint32_t)(((uint64_t)a * slope + 0x80000000u) >> 32);
}

/* the x86 "div": (hi:lo) / d with hi < d (else the original faults), the quotient and the remainder */
static uint32_t W30_Div(uint32_t hi, uint32_t lo, uint32_t d, uint32_t *rem)
{
    uint32_t q = 0, top;
    int i;

    for (i = 31; i >= 0; i--) {
        top = hi >> 31;
        hi = hi << 1 | lo >> 31;
        lo <<= 1;
        if (top != 0 || hi >= d) {
            hi -= d;
            q |= 1u << i;
        }
    }
    *rem = hi;
    return q;
}

/* (hi:0x80000000) / slope, then "cmp edx, 1; sbb eax, adj": the major-axis step for a minor-axis distance */
static uint32_t W30_DivRound(uint32_t hi, uint32_t slope, int up)
{
    uint32_t r, q = W30_Div(hi, 0x80000000u, slope, &r);

    return up ? q + 1 - (r < 1) : q - (r < 1);
}

static int32_t W30_Signed(uint32_t v, uint32_t s)
{
    return (int32_t)((v ^ s) - s);
}

static uint32_t W30_Outcode(int32_t x, int32_t y, int32_t xmin, int32_t ymin, int32_t xmax, int32_t ymax)
{
    return ((uint32_t)x - (uint32_t)xmin) >> 31 << 3 | ((uint32_t)xmax - (uint32_t)x) >> 31 << 2 |
           ((uint32_t)y - (uint32_t)ymin) >> 31 << 1 | ((uint32_t)ymax - (uint32_t)y) >> 31;
}

// FUNCTION: HGSIM 0x100ef727
int32_t __cdecl Sim_100ef727(void *pane, int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t mode,
                             uint32_t color)
{
    int32_t w, xmin, ymin, xmax, ymax, r;
    int32_t dx, dy, adx, ady, sx, sy, cx0, cy0, cx1, cy1, count, step;
    uint32_t slope, s, clipped = 0, acc, c0, c1;
    uint8_t *buf, *at;
    uint16_t col = (uint16_t)color;

    if (mode != 0)
        return -3;
    r = W30_Clip(pane, &w, &xmin, &ymin, &xmax, &ymax);
    if (r != 0)
        return r;
    buf = BYTES(U32(W30_WIN(pane), 0));
    x0 += S32(pane, 4);
    x1 += S32(pane, 4);
    y0 += S32(pane, 8);
    y1 += S32(pane, 8);
    dx = x1 - x0;
    sx = dx < 0 ? -1 : 0;
    adx = dx < 0 ? -dx : dx;
    dy = y1 - y0;
    sy = dy < 0 ? -1 : 0;
    ady = dy < 0 ? -dy : dy;
    cx0 = x0;
    cx1 = x1;
    cy0 = y0;
    cy1 = y1;
    if (dx == 0) {
        /* vertical: cut to the box, from the start's row in the line's direction */
        if (x0 < xmin || x0 > xmax || (y0 > y1 ? y0 : y1) < ymin || (y0 < y1 ? y0 : y1) > ymax)
            return 2;
        cy0 = y0 > ymin ? y0 : ymin;
        cy0 = cy0 < ymax ? cy0 : ymax;
        cy1 = y1 > ymin ? y1 : ymin;
        cy1 = cy1 < ymax ? cy1 : ymax;
        cx0 = x0;
        step = (w ^ sy) - sy;
        count = (cy1 - cy0 < 0 ? cy0 - cy1 : cy1 - cy0) + 1;
        goto straight;
    }
    if (dy == 0) {
        if (y0 < ymin || y0 > ymax || (x0 > x1 ? x0 : x1) < xmin || (x0 < x1 ? x0 : x1) > xmax)
            return 2;
        cx0 = x0 > xmin ? x0 : xmin;
        cx0 = cx0 < xmax ? cx0 : xmax;
        cx1 = x1 > xmin ? x1 : xmin;
        cx1 = cx1 < xmax ? cx1 : xmax;
        cy0 = y0;
        step = ((sx + 1) | sx) * 2;
        count = (cx1 - cx0 < 0 ? cx0 - cx1 : cx1 - cx0) + 1;
        goto straight;
    }
    s = (uint32_t)(sx ^ sy);
    if (adx == ady)
        slope = 0xffffffffu;
    else if (adx > ady)
        slope = W30_Div((uint32_t)ady, 0, (uint32_t)adx, &acc);
    else
        slope = W30_Div((uint32_t)adx, 0, (uint32_t)ady, &acc);
    for (;;) {
        c0 = W30_Outcode(cx0, cy0, xmin, ymin, xmax, ymax);
        c1 = W30_Outcode(cx1, cy1, xmin, ymin, xmax, ymax);
        clipped |= c0 | c1 << 8;
        if ((c0 | c1) == 0)
            break;
        if ((c0 & c1) != 0)
            return 2;
        if (adx >= ady) {
            if (c0 & 8) {
                cx0 = xmin;
                cy0 = W30_Signed(W30_MulRound(((uint32_t)xmin - (uint32_t)x0), slope), s) + y0;
            } else if (c0 & 4) {
                cx0 = xmax;
                cy0 = W30_Signed(W30_MulRound(((uint32_t)x0 - (uint32_t)xmax), slope), ~s) + y0;
            } else if (c0 & 2) {
                cy0 = ymin;
                cx0 = W30_Signed(W30_DivRound(((uint32_t)ymin - (uint32_t)y0) - 1, slope, 1), s) + x0;
            } else if (c0 & 1) {
                cy0 = ymax;
                cx0 = W30_Signed(W30_DivRound(((uint32_t)y0 - (uint32_t)ymax) - 1, slope, 1), ~s) + x0;
            } else if (c1 & 8) {
                cx1 = xmin;
                cy1 = W30_Signed(W30_MulRound(((uint32_t)x0 - (uint32_t)xmin), slope), ~s) + y0;
            } else if (c1 & 4) {
                cx1 = xmax;
                cy1 = W30_Signed(W30_MulRound(((uint32_t)xmax - (uint32_t)x0), slope), s) + y0;
            } else if (c1 & 2) {
                cy1 = ymin;
                cx1 = W30_Signed(W30_DivRound(((uint32_t)y0 - (uint32_t)ymin), slope, 0), ~s) + x0;
            } else {
                cy1 = ymax;
                cx1 = W30_Signed(W30_DivRound(((uint32_t)ymax - (uint32_t)y0), slope, 0), s) + x0;
            }
        } else {
            if (c0 & 8) {
                cx0 = xmin;
                cy0 = W30_Signed(W30_DivRound(((uint32_t)xmin - (uint32_t)x0) - 1, slope, 1), s) + y0;
            } else if (c0 & 4) {
                cx0 = xmax;
                cy0 = W30_Signed(W30_DivRound(((uint32_t)x0 - (uint32_t)xmax) - 1, slope, 1), ~s) + y0;
            } else if (c0 & 2) {
                cy0 = ymin;
                cx0 = W30_Signed(W30_MulRound(((uint32_t)ymin - (uint32_t)y0), slope), s) + x0;
            } else if (c0 & 1) {
                cy0 = ymax;
                cx0 = W30_Signed(W30_MulRound(((uint32_t)y0 - (uint32_t)ymax), slope), ~s) + x0;
            } else if (c1 & 8) {
                cx1 = xmin;
                cy1 = W30_Signed(W30_DivRound(((uint32_t)x0 - (uint32_t)xmin), slope, 0), ~s) + y0;
            } else if (c1 & 4) {
                cx1 = xmax;
                cy1 = W30_Signed(W30_DivRound(((uint32_t)xmax - (uint32_t)x0), slope, 0), s) + y0;
            } else if (c1 & 2) {
                cy1 = ymin;
                cx1 = W30_Signed(W30_MulRound(((uint32_t)y0 - (uint32_t)ymin), slope), ~s) + x0;
            } else {
                cy1 = ymax;
                cx1 = W30_Signed(W30_MulRound(((uint32_t)ymax - (uint32_t)y0), slope), s) + x0;
            }
        }
    }
    at = buf + (uint32_t)(cy0 * w) + (uint32_t)cx0 * 2u;
    step = (w ^ sy) - sy;
    if (adx == ady) {
        step += ((sx + 1) | sx) * 2;
        count = (cx1 - cx0 < 0 ? cx0 - cx1 : cx1 - cx0) + 1;
        for (; count > 0; count--, at += step)
            *(u16a *)at = col;
    } else if (adx > ady) {
        count = (cx1 - cx0 < 0 ? cx0 - cx1 : cx1 - cx0) + 1;
        acc = (uint32_t)(cx0 - x0 < 0 ? x0 - cx0 : cx0 - x0) * slope + 0x80000000u;
        for (; count > 0; count--) {
            *(u16a *)at = col;
            at += sx == -1 ? -2 : 2;
            acc += slope;
            if (acc < slope)
                at += step;
        }
    } else {
        count = (cy1 - cy0 < 0 ? cy0 - cy1 : cy1 - cy0) + 1;
        acc = (uint32_t)(cy0 - y0 < 0 ? y0 - cy0 : cy0 - y0) * slope + 0x80000000u;
        for (; count > 0; count--) {
            *(u16a *)at = col;
            acc += slope;
            if (acc < slope)
                at += sx == -1 ? -2 : 2;
            at += step;
        }
    }
#ifdef W30L_MUTATE /* the checker's negative check only */
    return clipped >= 2 ? 1 : 0;
#else
    return clipped >= 1 ? 1 : 0;
#endif

straight:
    /* the original returns ([ebp-0x34] >= 1) here, a flag it only sets on the clipping path: an unset stack word;
       no caller reads the result (DECISIONS.md round 30), 0 here */
    at = buf + (uint32_t)(cy0 * w) + (uint32_t)cx0 * 2u;
    for (; count > 0; count--, at += step)
        *(u16a *)at = col;
    return 0;
}

// FUNCTION: HGSIM 0x100f0865
int32_t __cdecl Sim_100f0865(void *pane, uint32_t color)
{
    int32_t w, x0, y0, x1, y1, r, y, n;
    uint8_t *at;
    uint16_t col = (uint16_t)color;

    r = W30_Clip(pane, &w, &x0, &y0, &x1, &y1);
    if (r != 0)
        return r;
#ifdef W30W_MUTATE /* the checker's negative check only */
    x1--;
#endif
    at = BYTES(U32(W30_WIN(pane), 0)) + (uint32_t)(y0 * w) + (uint32_t)x0 * 2u;
    for (y = y0; y <= y1; y++, at += w)
        for (n = 0; n <= x1 - x0; n++)
            ((u16a *)at)[n] = col;
    return 0;
}

#define W30_LUT(i) (SIM_PTR(u16a, 0x10156dcc)[(i)])

/* One row of a shape (run-length codes) from *src to dst, as the library's decoders: 0 ends the row, 1 skips the
 * next byte's count of cells, an even code repeats the next byte's colour code / 2 times, an odd one copies the
 * next code / 2 bytes. mode 0: no clipping; 1: cut on the left at left (then 0 or 2 from there, by right); 2: cut
 * on the right at right; 3: past the right edge (consume only). left / right are cell pointers, compared signed
 * as the original's "jg". */
static void W30_ShapeRow(const uint8_t **src, uint8_t *dst, int mode, int cutRight, uint8_t *left, uint8_t *right)
{
    const uint8_t *s = *src;
    uint32_t n, k;
    int32_t room, over;

    for (;;) {
        uint8_t b = *s++;

        if (b == 0)
            break;
        n = b >> 1;
        if (b == 1) {
            dst += 2u * *s++;
            continue;
        }
        if (mode == 1) {
            room = (int32_t)((int32_t)(uintptr_t)left - (int32_t)(uintptr_t)dst) >> 1;
            if (room >= (int32_t)n) {
                dst += 2 * n;
                s += (b & 1) ? n : 1;
                continue;
            }
            if (room >= 0) {
                dst += 2 * (uint32_t)room;
                n -= (uint32_t)room;
                if (b & 1)
                    s += room;
            }
            mode = cutRight ? 2 : 0;
        }
        if (mode == 3 || (mode == 2 && (int32_t)(uintptr_t)dst > (int32_t)(uintptr_t)right)) {
            mode = 3;
            dst += 2 * n;
            s += (b & 1) ? n : 1;
            continue;
        }
        over = 0;
        if (mode == 2) {
            over = (int32_t)((uint32_t)(uintptr_t)dst + 2 * n - 2 - (uint32_t)(uintptr_t)right);
            over = over < 0 ? 0 : (int32_t)((uint32_t)over >> 1);
            n -= (uint32_t)over;
        }
        if (b & 1) {
            for (k = 0; k < n; k++, dst += 2)
                *(u16a *)dst = W30_LUT(*s++);
            s += over;
        } else {
            uint16_t v = W30_LUT(*s++);

            for (k = 0; k < n; k++, dst += 2)
                *(u16a *)dst = v;
        }
        dst += 2 * (uint32_t)over;
    }
    *src = s;
}

// FUNCTION: HGSIM 0x100f0727
int32_t __cdecl Sim_100f0727(void *pane, const uint8_t *shape, int32_t x, int32_t y, int32_t slot)
{
    uint8_t *win = W30_WIN(pane), *row;
    const uint8_t *s;
    uint32_t w;
    int32_t rows;

    (void)slot;                         /* the original keeps the pitch in this argument slot */
    x += S32(pane, 4);
    y += S32(pane, 8);
    if (S32(win, 4) + 1 <= 0)
        return 0;
    w = (uint32_t)(S32(win, 4) + 1);
    row = BYTES(U32(win, 0)) + 2u * (uint32_t)(S32(shape, 8) + x) + (uint32_t)(S32(shape, 0xc) + y) * w;
    rows = S32(shape, 0x14) + 1 - S32(shape, 0xc);
#ifdef W30S_MUTATE /* the checker's negative check only */
    rows--;
#endif
    s = shape + 0x18;
    for (; rows > 0; rows--, row += w)
        W30_ShapeRow(&s, row, 0, 0, 0, 0);
    return 0;
}

// FUNCTION: HGSIM 0x100f0205
int32_t __cdecl Sim_100f0205(void *pane, const uint8_t *table, int32_t number, int32_t x, int32_t y)
{
    int32_t w, xmin, ymin, xmax, ymax, r, bx0, by0, bx1, by1, row;
    uint32_t c0, c1, code;
    const uint8_t *shape, *s;
    uint8_t *buf, *at, *left, *right;

    r = W30_Clip(pane, &w, &xmin, &ymin, &xmax, &ymax);
    if (r != 0)
        return r;
    buf = BYTES(U32(W30_WIN(pane), 0));
    x += S32(pane, 4);
    y += S32(pane, 8);
    shape = table + U32(table, 8 + (uint32_t)number * 8);
    bx0 = S32(shape, 8) + x;
    by0 = S32(shape, 0xc) + y;
    bx1 = S32(shape, 0x10) + x;
    by1 = S32(shape, 0x14) + y;
    s = shape + 0x18;
    if (bx1 < bx0 || by1 < by0)
        return -4;
    c0 = W30_Outcode(bx0, by0, xmin, ymin, xmax, ymax);
    c1 = W30_Outcode(bx1, by1, xmin, ymin, xmax, ymax);
    code = c0 | c1 << 8;
    if ((c0 & c1) != 0)
        return -3;
    if ((c0 | c1) == 0) {
        Sim_100f0727(pane, shape, x - S32(pane, 4), y - S32(pane, 8), w);
        return 0;
    }
    at = buf + (uint32_t)(by0 * w) + (uint32_t)bx0 * 2u;
    for (row = by0; row < ymin; row++, at += w)
        W30_ShapeRow(&s, at, 3, 0, 0, 0);
    left = at + 2u * (uint32_t)(xmin - bx0);
    right = at + 2u * (uint32_t)(xmax - bx0);
    for (; row <= by1; row++, at += w, left += w, right += w) {
        if (row > ymax)
            break;
        if (code & 8)
            W30_ShapeRow(&s, at, 1, (code & 0x400) != 0, left, right);
        else if (code & 0x400)
            W30_ShapeRow(&s, at, 2, 1, left, right);
        else
            W30_ShapeRow(&s, at, 0, 0, left, right);
    }
    return 0;
}

/* The glyph clip shared by the two character routines: cuts the glyph (width cells, h rows) at (*x, *y) to the box,
 * moving *src as the original does (one byte per cell, also for the 1-bit font). 0 when nothing is left. */
static int W30_GlyphClip(int32_t *x, int32_t *y, int32_t width, int32_t *cols, int32_t *rows, const uint8_t **src,
                         int32_t xmin, int32_t ymin, int32_t xmax, int32_t ymax)
{
    int32_t d;

    d = xmax + 1 - *cols - *x;
    if (d < 0 && (*cols += d) <= 0)
        return 0;
    d = *x - xmin;
    if (d < 0) {
        if ((*cols += d) <= 0)
            return 0;
        *src -= d;
        *x -= d;
    }
    d = ymax + 1 - *rows - *y;
    if (d < 0 && (*rows += d) <= 0)
        return 0;
    d = *y - ymin;
    if (d < 0) {
        if ((*rows += d) <= 0)
            return 0;
        *y -= d;
        *src -= d * width;
    }
    (void)width;
    return 1;
}

// FUNCTION: HGSIM 0x100f1c93
int32_t __cdecl Sim_100f1c93(void *pane, int32_t x, int32_t y, const uint8_t *font, int32_t code,
                             const uint8_t *lookaside)
{
    int32_t w, xmin, ymin, xmax, ymax, r, width, cols, rows, i;
    const uint8_t *glyph, *src;
    uint8_t *at;

    r = W30_Clip(pane, &w, &xmin, &ymin, &xmax, &ymax);
    if (r != 0)
        return r;
    x += S32(pane, 4);
    y += S32(pane, 8);
    rows = S32(font, 8);
    glyph = font + U32(font, 0x10 + (uint32_t)code * 4);
    width = S32(glyph, 0);
    if (width == 0)
        return 0;
    src = glyph + 4;
    cols = width;
    if (!W30_GlyphClip(&x, &y, width, &cols, &rows, &src, xmin, ymin, xmax, ymax))
        return width;
    at = BYTES(U32(W30_WIN(pane), 0)) + (uint32_t)(y * w) + (uint32_t)x * 2u;
    for (; rows > 0; rows--, src += width - cols, at += w - 2 * cols) {
        for (i = 0; i < cols; i++, at += 2) {
            uint8_t k = lookaside[*src++];

#ifdef W30C_MUTATE /* the checker's negative check only */
            if (k != 0xfe)
#else
            if (k != 0xff)
#endif
                *(u16a *)at = W30_LUT(k);
        }
    }
    return width;
}

// FUNCTION: HGSIM 0x100f1e10
int32_t __cdecl Sim_100f1e10(void *pane, int32_t x, int32_t y, const uint8_t *font, const uint8_t *text,
                             const uint8_t *lookaside)
{
    int32_t r;

    do {
        r = W30_Char(pane, x, y, font, *text, lookaside);   /* the entry point: full resolution per character */
        x += r;
        text++;
    } while (*text != 0);
    return r;
}

/* the glyph number of a character code in the 8-bit set: through the table [0x10157590] for two-byte codes;
 * [0x10157594] = the code's length in bytes (1 or 2) */
static uint32_t W30_TextGlyph(uint32_t code)
{
    uint32_t e = ((code & 0xffff00ffu) << 1);
    uint16_t t = *(u16a *)(BYTES(G32(0x10157590)) + (code & 0xff) * 2u);

    if (t == 0) {
        G32(0x10157594) = 1;
        return (e & 0xffff0000u) | (code & 0xff);   /* the word load cleared the low half first */
    }
    G32(0x10157594) = 2;
    return (e & 0xffff0000u) | (uint16_t)(t + ((code >> 8) & 0xff));
}

// FUNCTION: HGSIM 0x100f2d3c
int32_t __cdecl Sim_100f2d3c(void *pane, int32_t x, int32_t y, const uint8_t *font, uint32_t code,
                             const uint8_t *lookaside)
{
    int32_t w, xmin, ymin, xmax, ymax, r, width, cols, rows;
    const uint8_t *glyph, *src;
    uint8_t *at, colour, first, left, dh, nrows;
    uint32_t bits, k;

    r = W30_Clip(pane, &w, &xmin, &ymin, &xmax, &ymax);
    if (r != 0)
        return r;
    x += S32(pane, 4);
    y += S32(pane, 8);
    rows = S32(font, 8);
    glyph = font + U32(font, 0x10 + W30_TextGlyph(code) * 4);
    width = S32(glyph, 0);
    if (width == 0)
        return 0;
    src = glyph + 4;
    cols = width;
    if (!W30_GlyphClip(&x, &y, width, &cols, &rows, &src, xmin, ymin, xmax, ymax))
        return width;
    nrows = (uint8_t)rows;
    at = BYTES(U32(W30_WIN(pane), 0)) + (uint32_t)(y * w) + (uint32_t)x;   /* one byte per cell: the 8-bit set */
    colour = *GBYTES(0x10157598);
    if (lookaside != 0)
        colour = lookaside[colour];
    if (colour == 0xff)
        return width;
#ifdef W30T_MUTATE /* the checker's negative check only */
    colour ^= 1;
#endif
    first = (uint8_t)cols;
    do {
        bits = (int8_t)first > 8 ? 8 : first;
        left = first;
        for (;;) {
            dh = *src;
            for (k = 0; k < bits; k++, at++) {
                if (dh & 0x80)
                    *at = colour;
                dh = (uint8_t)(dh << 1);
            }
            src++;
            if (left <= 8)
                break;
            left = (uint8_t)(left - 8);
            bits = left;
        }
        src += width - cols;
        at += w - cols;
    } while (--nrows != 0);
    return width;
}

// FUNCTION: HGSIM 0x100f2f01
int32_t __cdecl Sim_100f2f01(void *pane, int32_t x, int32_t y, const uint8_t *font, const uint8_t *text,
                             const uint8_t *lookaside)
{
    int32_t r;

    do {
        r = Sim_100f2d3c(pane, x, y, font, *(const u16a *)text, lookaside);
        x += r;
        text += G32(0x10157594);
    } while (*text != 0);
    return r;
}
