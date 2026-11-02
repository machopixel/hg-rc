#include "w37.h"
#include "w37_int.h"
#include "../util/access.h"

/*
 * The 8-bit line routine: its clipping is instruction for instruction that of the 16-bit one (0x100ef727, round 30,
 * src/sim/render/w30_vfx.c), whose helpers are repeated here; only the drawing differs (byte cells, and the modes:
 * 0 the colour, 1 each cell through a 256-byte table, above 1 a callback).
 */

/* high dword of a * b + 0x80000000: a times a 0.32 slope, rounded */
static uint32_t W37_MulRound(uint32_t a, uint32_t slope)
{
    return (uint32_t)(((uint64_t)a * slope + 0x80000000u) >> 32);
}

/* the x86 "div": (hi:lo) / d with hi < d (else the original faults), the quotient and the remainder */
static uint32_t W37_Div(uint32_t hi, uint32_t lo, uint32_t d, uint32_t *rem)
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
static uint32_t W37_DivRound(uint32_t hi, uint32_t slope, int up)
{
    uint32_t r, q = W37_Div(hi, 0x80000000u, slope, &r);

    return up ? q + 1 - (r < 1) : q - (r < 1);
}

static int32_t W37_Signed(uint32_t v, uint32_t s)
{
    return (int32_t)((v ^ s) - s);
}

static uint32_t W37_Outcode(int32_t x, int32_t y, int32_t xmin, int32_t ymin, int32_t xmax, int32_t ymax)
{
    return ((uint32_t)x - (uint32_t)xmin) >> 31 << 3 | ((uint32_t)xmax - (uint32_t)x) >> 31 << 2 |
           ((uint32_t)y - (uint32_t)ymin) >> 31 << 1 | ((uint32_t)ymax - (uint32_t)y) >> 31;
}

/*
 * One cell: mode 1 maps the cell through the table (the colour argument), modes up to 0 set the colour byte. Modes
 * above 1 call the colour argument as a callback (with the pane-relative cell in EDI / ESI and the steps in EAX /
 * EBX, under a pushad) for each cell; that is not converted and draws nothing: no caller passes such a mode
 * (DECISIONS.md, "DLL removal step 1").
 */
#define PUT(at)                                                                                                    \
    do {                                                                                                           \
        if (mode == 1)                                                                                             \
            *(at) = ((const uint8_t *)(uintptr_t)color)[*(at)];                                                    \
        else if (mode < 1)                                                                                         \
            *(at) = (uint8_t)color;                                                                              \
    } while (0)

// FUNCTION: HGSIM 0x100e86a1
int32_t __cdecl Sim_100e86a1(const uint8_t *pane, int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t mode,
                             uint32_t color)
{
    const uint8_t *window = (const uint8_t *)(uintptr_t)U32(pane, 0);
    int32_t w, xmin, ymin, xmax, ymax;
    int32_t dx, dy, adx, ady, sx, sy, cx0, cy0, cx1, cy1, count, step;
    uint32_t slope, s, clipped = 0, acc, c0, c1;
    uint8_t *buf, *at;

    if (S32(window, 4) < 0 || S32(window, 8) < 0)
        return -1;
    w = ADD(S32(window, 4), 1);
    xmin = S32(pane, 4) > 0 ? S32(pane, 4) : 0;
    ymin = S32(pane, 8) > 0 ? S32(pane, 8) : 0;
    xmax = S32(pane, 0xc) < S32(window, 4) ? S32(pane, 0xc) : S32(window, 4);
    ymax = S32(pane, 0x10) < S32(window, 8) ? S32(pane, 0x10) : S32(window, 8);
    if (xmax < xmin || ymax < ymin)
        return -2;
    buf = (uint8_t *)(uintptr_t)U32(window, 0);
    x0 = ADD(x0, S32(pane, 4));
    x1 = ADD(x1, S32(pane, 4));
    y0 = ADD(y0, S32(pane, 8));
    y1 = ADD(y1, S32(pane, 8));
    dx = DIFF(x1, x0);
    sx = dx < 0 ? -1 : 0;
    adx = W37_Signed((uint32_t)dx, (uint32_t)sx);
    dy = DIFF(y1, y0);
    sy = dy < 0 ? -1 : 0;
    ady = W37_Signed((uint32_t)dy, (uint32_t)sy);
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
        step = W37_Signed((uint32_t)w, (uint32_t)sy);
        count = W37_Signed((uint32_t)DIFF(cy1, cy0), DIFF(cy1, cy0) < 0 ? 0xffffffffu : 0) + 1;
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
        step = (sx + 1) | sx;
        count = W37_Signed((uint32_t)DIFF(cx1, cx0), DIFF(cx1, cx0) < 0 ? 0xffffffffu : 0) + 1;
        goto straight;
    }
    s = (uint32_t)(sx ^ sy);
    if (adx == ady)
        slope = 0xffffffffu;
    else if (adx > ady)
        slope = W37_Div((uint32_t)ady, 0, (uint32_t)adx, &acc);
    else
        slope = W37_Div((uint32_t)adx, 0, (uint32_t)ady, &acc);
    for (;;) {
        c0 = W37_Outcode(cx0, cy0, xmin, ymin, xmax, ymax);
        c1 = W37_Outcode(cx1, cy1, xmin, ymin, xmax, ymax);
        clipped |= c0 | c1 << 8;
        if ((c0 | c1) == 0)
            break;
        if ((c0 & c1) != 0)
            return 2;
        if (adx >= ady) {
            if (c0 & 8) {
                cx0 = xmin;
                cy0 = W37_Signed(W37_MulRound(((uint32_t)xmin - (uint32_t)x0), slope), s) + y0;
            } else if (c0 & 4) {
                cx0 = xmax;
                cy0 = W37_Signed(W37_MulRound(((uint32_t)x0 - (uint32_t)xmax), slope), ~s) + y0;
            } else if (c0 & 2) {
                cy0 = ymin;
                cx0 = W37_Signed(W37_DivRound(((uint32_t)ymin - (uint32_t)y0) - 1, slope, 1), s) + x0;
            } else if (c0 & 1) {
                cy0 = ymax;
                cx0 = W37_Signed(W37_DivRound(((uint32_t)y0 - (uint32_t)ymax) - 1, slope, 1), ~s) + x0;
            } else if (c1 & 8) {
                cx1 = xmin;
                cy1 = W37_Signed(W37_MulRound(((uint32_t)x0 - (uint32_t)xmin), slope), ~s) + y0;
            } else if (c1 & 4) {
                cx1 = xmax;
                cy1 = W37_Signed(W37_MulRound(((uint32_t)xmax - (uint32_t)x0), slope), s) + y0;
            } else if (c1 & 2) {
                cy1 = ymin;
                cx1 = W37_Signed(W37_DivRound(((uint32_t)y0 - (uint32_t)ymin), slope, 0), ~s) + x0;
            } else {
                cy1 = ymax;
                cx1 = W37_Signed(W37_DivRound(((uint32_t)ymax - (uint32_t)y0), slope, 0), s) + x0;
            }
        } else {
            if (c0 & 8) {
                cx0 = xmin;
                cy0 = W37_Signed(W37_DivRound(((uint32_t)xmin - (uint32_t)x0) - 1, slope, 1), s) + y0;
            } else if (c0 & 4) {
                cx0 = xmax;
                cy0 = W37_Signed(W37_DivRound(((uint32_t)x0 - (uint32_t)xmax) - 1, slope, 1), ~s) + y0;
            } else if (c0 & 2) {
                cy0 = ymin;
                cx0 = W37_Signed(W37_MulRound(((uint32_t)ymin - (uint32_t)y0), slope), s) + x0;
            } else if (c0 & 1) {
                cy0 = ymax;
                cx0 = W37_Signed(W37_MulRound(((uint32_t)y0 - (uint32_t)ymax), slope), ~s) + x0;
            } else if (c1 & 8) {
                cx1 = xmin;
                cy1 = W37_Signed(W37_DivRound(((uint32_t)x0 - (uint32_t)xmin), slope, 0), ~s) + y0;
            } else if (c1 & 4) {
                cx1 = xmax;
                cy1 = W37_Signed(W37_DivRound(((uint32_t)xmax - (uint32_t)x0), slope, 0), s) + y0;
            } else if (c1 & 2) {
                cy1 = ymin;
                cx1 = W37_Signed(W37_MulRound(((uint32_t)y0 - (uint32_t)ymin), slope), ~s) + x0;
            } else {
                cy1 = ymax;
                cx1 = W37_Signed(W37_MulRound(((uint32_t)ymax - (uint32_t)y0), slope), s) + x0;
            }
        }
    }
    at = buf + (uint32_t)cy0 * (uint32_t)w + (uint32_t)cx0;
    step = W37_Signed((uint32_t)w, (uint32_t)sy);
    if (adx == ady) {
        step += (sx + 1) | sx;
        count = W37_Signed((uint32_t)DIFF(cx1, cx0), DIFF(cx1, cx0) < 0 ? 0xffffffffu : 0) + 1;
        do {
            PUT(at);
            at += step;
        } while (--count != 0);
    } else if (adx > ady) {
        count = W37_Signed((uint32_t)DIFF(cx1, cx0), DIFF(cx1, cx0) < 0 ? 0xffffffffu : 0) + 1;
        acc = (uint32_t)W37_Signed((uint32_t)DIFF(cx0, x0), DIFF(cx0, x0) < 0 ? 0xffffffffu : 0) * slope
              + 0x80000000u;
        do {
            PUT(at);
            at += sx == -1 ? -1 : 1;
            acc += slope;
            if (acc < slope)
                at += step;
        } while (--count != 0);
    } else {
        count = W37_Signed((uint32_t)DIFF(cy1, cy0), DIFF(cy1, cy0) < 0 ? 0xffffffffu : 0) + 1;
        acc = (uint32_t)W37_Signed((uint32_t)DIFF(cy0, y0), DIFF(cy0, y0) < 0 ? 0xffffffffu : 0) * slope
              + 0x80000000u;
        do {
            PUT(at);
            acc += slope;
            if (acc < slope)
                at += sx == -1 ? -1 : 1;
            at += step;
        } while (--count != 0);
    }
    return clipped >= 1 ? 1 : 0;

straight:
    /* the original returns ([ebp-0x34] >= 1) here, a flag it only sets on the clipping path: an unset stack word;
       no caller reads the result (as the 16-bit routine, DECISIONS.md round 30), 0 here */
    at = buf + (uint32_t)cy0 * (uint32_t)w + (uint32_t)cx0;
    do {
        PUT(at);
        at += step;
    } while (--count != 0);
    return 0;
}

/* The ellipses' clip box and cell writer. */
typedef struct {
    uint8_t *buffer;
    int32_t pitch, cx0, cy0, cx1, cy1;
} W37Box;

static void W37_Plot(const W37Box *b, int32_t x, int32_t y, uint8_t colour)
{
    if (x < b->cx0 || x > b->cx1 || y < b->cy0 || y > b->cy1)
        return;
    b->buffer[(uint32_t)y * (uint32_t)b->pitch + (uint32_t)x] = colour;
}

static void W37_Span(const W37Box *b, int32_t x0, int32_t x1, int32_t y, uint8_t colour)
{
    uint8_t *p = b->buffer + (uint32_t)y * (uint32_t)b->pitch + (uint32_t)x0;
    uint32_t n = (uint32_t)x1 - (uint32_t)x0 + 1, k;

    for (k = 0; k < n; k++)
        p[k] = colour;
}

/* The four cells (outline) or the two rows (filled) of one step. */
static void W37_EllipseStep(const W37Box *b, int filled, int32_t cx, int32_t cy, int32_t dx, int32_t dy,
                            uint8_t colour)
{
    int32_t xr, xl, y;

    if (!filled) {
        W37_Plot(b, ADD(cx, dx), ADD(cy, dy), colour);
        W37_Plot(b, ADD(cx, dx), DIFF(cy, dy), colour);
        W37_Plot(b, DIFF(cx, dx), ADD(cy, dy), colour);
        W37_Plot(b, DIFF(cx, dx), DIFF(cy, dy), colour);
        return;
    }
    xr = ADD(cx, dx);
    if (xr < b->cx0)
        return;
    if (xr >= b->cx1)
        xr = b->cx1;
    xl = DIFF(cx, dx);
    if (xl > b->cx1)
        return;
    if (xl <= b->cx0)
        xl = b->cx0;
    y = ADD(cy, dy);
    if (y < b->cy0)
        return;
    if (y <= b->cy1)
        W37_Span(b, xl, xr, y, colour);
    y = DIFF(cy, dy);
    if (y < b->cy0 || y > b->cy1)
        return;
    W37_Span(b, xl, xr, y, colour);
}

/*
 * The midpoint ellipse of 0x100eb6e8 (outline) and 0x100eba29 (filled) about pane-relative (x, y) with radii rx /
 * ry; a radius of 0 draws the line 0x100e86a1 (x - rx, y - ry) .. (x + rx, y + ry) instead. Returns the line's
 * result, -1 / -2 as the character draw, else the last ry-step term (the original's EAX).
 */
static int32_t W37_Ellipse(const uint8_t *pane, int32_t x, int32_t y, int32_t rx, int32_t ry, uint32_t colour,
                           int filled)
{
    const uint8_t *window = (const uint8_t *)(uintptr_t)U32(pane, 0);
    W37Box b;
    uint32_t a2, b2, a2x2, b2x2, px, py, p;
    int32_t dx, dy, left;
    uint32_t t;

    if (rx == 0 || ry == 0)
        return Sim_100e86a1(pane, DIFF(x, rx), DIFF(y, ry), ADD(x, rx), ADD(y, ry), 0, colour);
    if (S32(window, 4) < 0 || S32(window, 8) < 0)
        return -1;
    b.pitch = ADD(S32(window, 4), 1);
    b.cx0 = S32(pane, 4) > 0 ? S32(pane, 4) : 0;
    b.cy0 = S32(pane, 8) > 0 ? S32(pane, 8) : 0;
    b.cx1 = S32(pane, 0xc) < S32(window, 4) ? S32(pane, 0xc) : S32(window, 4);
    b.cy1 = S32(pane, 0x10) < S32(window, 8) ? S32(pane, 0x10) : S32(window, 8);
    if (b.cx1 < b.cx0 || b.cy1 < b.cy0)
        return -2;
    b.buffer = (uint8_t *)(uintptr_t)U32(window, 0);
    x = ADD(x, S32(pane, 4));
    y = ADD(y, S32(pane, 8));
    dx = 0;
    dy = ry;
    b2 = (uint32_t)ry * (uint32_t)ry;
    b2x2 = b2 << 1;
    a2 = (uint32_t)rx * (uint32_t)rx;
    a2x2 = a2 << 1;
    px = 0;
    py = a2x2 * (uint32_t)ry;
    p = (a2 >> 2) + b2 - a2 * (uint32_t)ry;
    left = ry;
    /* the region where x steps every time */
    while ((int32_t)(px - py) < 0) {
        W37_EllipseStep(&b, filled, x, y, dx, dy, (uint8_t)colour);
        if ((int32_t)p >= 0) {
            dy--;
            left--;
            py -= a2x2;
            p -= py;
        }
        dx++;
        px += b2x2;
        p += px + b2;
    }
    t = a2 - b2;
    p += (uint32_t)((int32_t)((uint32_t)((int32_t)t >> 1) + t - px - py) >> 1);
    /* the region where y steps every time */
    do {
        W37_EllipseStep(&b, filled, x, y, dx, dy, (uint8_t)colour);
        if ((int32_t)p < 0) {
            dx++;
            px += b2x2;
            p += px;
        }
        dy--;
        py -= a2x2;
        t = py - a2;
        p -= t;
    } while ((left = DIFF(left, 1)) >= 0);
    return (int32_t)t;
}

// FUNCTION: HGSIM 0x100eb6e8
int32_t __cdecl Sim_100eb6e8(const uint8_t *pane, int32_t x, int32_t y, int32_t rx, int32_t ry, uint32_t colour)
{
    return W37_Ellipse(pane, x, y, rx, ry, colour, 0);
}

// FUNCTION: HGSIM 0x100eba29
int32_t __cdecl Sim_100eba29(const uint8_t *pane, int32_t x, int32_t y, int32_t rx, int32_t ry, uint32_t colour)
{
    return W37_Ellipse(pane, x, y, rx, ry, colour, 1);
}
