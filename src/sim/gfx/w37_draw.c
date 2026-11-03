#include "w37.h"
#include "w37_int.h"
#include "../simmem.h"
#include "../util/access.h"

/*
 * The pane cut shared by the drawing routines: {window, x0, y0, x1, y1} against the window {buffer, pitch - 1,
 * height - 1}. -1 for an empty window, -2 for an empty pane, else 0 with the edges.
 */
typedef struct {
    uint8_t *buffer;
    int32_t pitch, x0, y0, cx0, cy0, cx1, cy1;
} W37Clip;

static int32_t W37_Clip(const uint8_t *pane, W37Clip *c)
{
    const uint8_t *window = (const uint8_t *)(uintptr_t)U32(pane, 0);

    if (S32(window, 4) < 0 || S32(window, 8) < 0)
        return -1;
    c->pitch = ADD(S32(window, 4), 1);
    c->x0 = S32(pane, 4);
    c->y0 = S32(pane, 8);
    c->cx0 = c->x0 > 0 ? c->x0 : 0;
    c->cy0 = c->y0 > 0 ? c->y0 : 0;
    c->cx1 = S32(pane, 0xc) < S32(window, 4) ? S32(pane, 0xc) : S32(window, 4);
    c->cy1 = S32(pane, 0x10) < S32(window, 8) ? S32(pane, 0x10) : S32(window, 8);
    if (c->cx1 < c->cx0 || c->cy1 < c->cy0)
        return -2;
    c->buffer = (uint8_t *)(uintptr_t)U32(window, 0);
    return 0;
}

// FUNCTION: HGSIM 0x100e84f0
int32_t __cdecl Sim_100e84f0(const uint8_t *pane, int32_t x, int32_t y, uint32_t colour)
{
    W37Clip c;
    int32_t r = W37_Clip(pane, &c);
    uint8_t *p, old;

    if (r != 0)
        return r;
    x = ADD(x, c.x0);
    y = ADD(y, c.y0);
    if (x < c.cx0 || x > c.cx1 || y < c.cy0 || y > c.cy1)
        return -3;
    p = c.buffer + (uint32_t)y * (uint32_t)c.pitch + (uint32_t)x;
    old = *p;
    *p = (uint8_t)colour;
    return old;
}

// FUNCTION: HGSIM 0x100eb070
int32_t __cdecl Sim_100eb070(const uint8_t *pane, uint32_t colour)
{
    W37Clip c;
    int32_t r = W37_Clip(pane, &c), y;
    uint32_t width, k;
    uint8_t *p;

    if (r != 0)
        return r;
    p = c.buffer + (uint32_t)c.cy0 * (uint32_t)c.pitch + (uint32_t)c.cx0;
    width = (uint32_t)c.cx1 + 1 - (uint32_t)c.cx0;
    for (y = c.cy0; y <= c.cy1; y++) {
        for (k = 0; k < width; k++)
            *p++ = (uint8_t)colour;
        p += (uint32_t)c.pitch - width;
    }
    return 0;
}

/* A colour through the remap table, when there is one (the library's remapping twins, table 0x10154eac). */
#define MAP(map, b) ((map) != 0 ? (map)[(b)] : (b))
#define REMAP GBYTES(0x10154eac)

/*
 * One run-length row of a shape at `dst`: code byte 0 ends the row, 1 skips the next byte's count of cells, an even
 * code fills code / 2 cells with the next byte, an odd one copies code / 2 bytes.
 */
static const uint8_t *W37_ShapeRow(const uint8_t *s, uint8_t *dst, const uint8_t *map)
{
    for (;;) {
        uint32_t code = *s++, n = code >> 1, k;

        if (code == 0)
            return s;
        if (code == 1) {
            dst += *s++;
        } else if ((code & 1) == 0) {
            uint8_t v = MAP(map, *s++);

            for (k = 0; k < n; k++)
                *dst++ = v;
        } else {
            for (k = 0; k < n; k++)
                *dst++ = MAP(map, *s++);
        }
    }
}

/* The unclipped shape draw of 0x100e95e0 / 0x100e9b9e. */
static int32_t W37_ShapeRaw(const uint8_t *pane, const uint8_t *shape, int32_t x, int32_t y, const uint8_t *map)
{
    const uint8_t *window = (const uint8_t *)(uintptr_t)U32(pane, 0);
    uint32_t pitch, rows;
    uint8_t *row;
    const uint8_t *s;

    x = ADD(x, S32(pane, 4));
    y = ADD(y, S32(pane, 8));
    if (S32(window, 4) < 0)
        return 0;
    pitch = U32(window, 4) + 1;
    row = (uint8_t *)(uintptr_t)(U32(window, 0) + U32(shape, 8) + (uint32_t)x
                                 + (U32(shape, 0xc) + (uint32_t)y) * pitch);
    if (SUB_LE0(ADD(S32(shape, 0x14), 1), S32(shape, 0xc)))
        return 0;
    rows = U32(shape, 0x14) + 1 - U32(shape, 0xc);
    s = shape + 0x18;
    do {
        s = W37_ShapeRow(s, row, map);
        row += pitch;
    } while (--rows != 0);
    return 0;
}

// FUNCTION: HGSIM 0x100e95e0
int32_t __cdecl Sim_100e95e0(const uint8_t *pane, const uint8_t *shape, int32_t x, int32_t y, int32_t slot)
{
    (void)slot;                         /* the original keeps the pitch in this argument slot */
    return W37_ShapeRaw(pane, shape, x, y, 0);
}

// FUNCTION: HGSIM 0x100e9b9e
int32_t __cdecl Sim_100e9b9e(const uint8_t *pane, const uint8_t *shape, int32_t x, int32_t y, int32_t slot)
{
    (void)slot;                         /* the original keeps the pitch in this argument slot */
    return W37_ShapeRaw(pane, shape, x, y, REMAP);
}

/* Signed comparisons of addresses, as the original makes them. */
#define ADDR(p) ((int32_t)(uint32_t)(uintptr_t)(p))

enum { ROW_PLAIN, ROW_LEFT, ROW_RIGHT, ROW_DISCARD };

/*
 * One run-length row of the clipped shape draw (codes as W37_ShapeRow). ROW_LEFT: runs wholly left of `left` are
 * passed over, the first one that reaches it is cut there and the row goes on as ROW_RIGHT (`both`) or ROW_PLAIN.
 * ROW_RIGHT: a run starting right of `right` ends the drawing (ROW_DISCARD: the rest only read), one reaching past it
 * is cut. The original's four code loops.
 */
static const uint8_t *W37_ClipRow(const uint8_t *s, uint8_t *dst, const uint8_t *map, int mode, int both,
                                  const uint8_t *left, const uint8_t *right)
{
    for (;;) {
        uint32_t code = *s++, n = code >> 1, k;
        int run = (code & 1) == 0;
        int32_t d;

        if (code == 0)
            return s;
        if (code == 1) {
            dst += *s++;
            continue;
        }
        if (mode == ROW_DISCARD) {
            dst += n;
            s += run ? 1 : n;
            continue;
        }
        if (mode == ROW_LEFT) {
            d = DIFF(ADDR(left), ADDR(dst));
            if (d >= (int32_t)n) {
                dst += n;
                s += run ? 1 : n;
                continue;
            }
            if (d >= 0) {
                dst += d;
                n -= (uint32_t)d;
                if (!run)
                    s += d;
            }
            mode = both ? ROW_RIGHT : ROW_PLAIN;
        }
        if (mode == ROW_RIGHT) {
            int32_t extra;

            if (ADDR(dst) > ADDR(right)) {
                dst += n;
                s += run ? 1 : n;
                mode = ROW_DISCARD;
                continue;
            }
            extra = DIFF((uint32_t)ADDR(dst) + n - 1, ADDR(right));
            extra = extra < 0 ? 0 : extra;
            n -= (uint32_t)extra;
            if (run) {
                uint8_t v = MAP(map, *s++);

                for (k = 0; k < n; k++)
                    *dst++ = v;
            } else {
                for (k = 0; k < n; k++)
                    *dst++ = MAP(map, *s++);
                s += extra;
            }
            dst += extra;
            continue;
        }
        if (run) {
            uint8_t v = MAP(map, *s++);

            for (k = 0; k < n; k++)
                *dst++ = v;
        } else {
            for (k = 0; k < n; k++)
                *dst++ = MAP(map, *s++);
        }
    }
}

/* The clipped shape draw of 0x100e91dc / 0x100e96c6 (whose unclipped call is 0x100e95e0 / 0x100e9b9e). */
static int32_t W37_Shape(const uint8_t *pane, const uint8_t *table, int32_t number, int32_t x, int32_t y,
                         const uint8_t *map)
{
    W37Clip c;
    int32_t r = W37_Clip(pane, &c), bx0, by0, bx1, by1, row;
    uint32_t c0, c1, code;
    const uint8_t *shape, *s;
    uint8_t *at, *left, *right;

    if (r != 0)
        return r;
    x = ADD(x, c.x0);
    y = ADD(y, c.y0);
    shape = table + U32(table, 8 + 8 * number);
    bx0 = ADD(S32(shape, 8), x);
    by0 = ADD(S32(shape, 0xc), y);
    bx1 = ADD(S32(shape, 0x10), x);
    by1 = ADD(S32(shape, 0x14), y);
    s = shape + 0x18;
    if (bx1 < bx0 || by1 < by0)
        return -4;
    /* outcodes: left, right, above, below of the box's corners (the signs of the differences) */
    c0 = (uint32_t)(DIFF(bx0, c.cx0) < 0) << 3 | (uint32_t)(DIFF(c.cx1, bx0) < 0) << 2
         | (uint32_t)(DIFF(by0, c.cy0) < 0) << 1 | (uint32_t)(DIFF(c.cy1, by0) < 0);
    c1 = (uint32_t)(DIFF(bx1, c.cx0) < 0) << 3 | (uint32_t)(DIFF(c.cx1, bx1) < 0) << 2
         | (uint32_t)(DIFF(by1, c.cy0) < 0) << 1 | (uint32_t)(DIFF(c.cy1, by1) < 0);
    code = c0 | c1 << 8;
    if ((c0 & c1) != 0)
        return -3;
    if ((c0 | c1) == 0) {
        if (map == 0)
            Sim_100e95e0(pane, shape, DIFF(x, c.x0), DIFF(y, c.y0), c.pitch);
        else
            Sim_100e9b9e(pane, shape, DIFF(x, c.x0), DIFF(y, c.y0), c.pitch);
        return 0;
    }
    at = c.buffer + (uint32_t)by0 * (uint32_t)c.pitch + (uint32_t)bx0;
    for (row = by0; row < c.cy0; row++) {
        s = W37_ClipRow(s, at, map, ROW_DISCARD, 0, 0, 0);
        at += c.pitch;
    }
    left = at - bx0 + c.cx0;
    right = at - bx0 + c.cx1;
    for (; row <= by1; row++) {
        if (row > c.cy1)
            return 0;
        if (code & 8)
            s = W37_ClipRow(s, at, map, ROW_LEFT, (code & 0x400) != 0, left, right);
        else if (code & 0x400)
            s = W37_ClipRow(s, at, map, ROW_RIGHT, 0, left, right);
        else
            s = W37_ClipRow(s, at, map, ROW_PLAIN, 0, left, right);
        at += c.pitch;
        left += c.pitch;
        right += c.pitch;
    }
    return 0;
}

// FUNCTION: HGSIM 0x100e91dc
int32_t __cdecl Sim_100e91dc(const uint8_t *pane, const uint8_t *table, int32_t number, int32_t x, int32_t y)
{
    return W37_Shape(pane, table, number, x, y, 0);
}

// FUNCTION: HGSIM 0x100e96c6
int32_t __cdecl Sim_100e96c6(const uint8_t *pane, const uint8_t *table, int32_t number, int32_t x, int32_t y)
{
    return W37_Shape(pane, table, number, x, y, REMAP);
}

// FUNCTION: HGSIM 0x100e90a3
int32_t __cdecl Sim_100e90a3(const uint8_t *pane, int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint32_t colour)
{
    W37Clip c;
    int32_t r = W37_Clip(pane, &c), rows;
    uint32_t width;
    uint8_t *row;

    if (r != 0)
        return r;
    x0 = ADD(x0, c.x0);
    x1 = ADD(x1, c.x0);
    y0 = ADD(y0, c.y0);
    y1 = ADD(y1, c.y0);
    if (x0 <= c.cx0)
        x0 = c.cx0;
    if (y0 <= c.cy0)
        y0 = c.cy0;
    if (x1 >= c.cx1)
        x1 = c.cx1;
    if (y1 >= c.cy1)
        y1 = c.cy1;
    if ((int64_t)x1 - x0 < 0)
        return -4;
    width = (uint32_t)x1 - (uint32_t)x0 + 1;
    row = c.buffer + (uint32_t)y0 * (uint32_t)c.pitch + (uint32_t)x0;
    if ((int64_t)y1 - y0 < 0)
        return -4;
    rows = DIFF(y1, y0);
    /* every other cell, the rows shifted by one in turn (a shifted row of width 1 still sets the cell right of it) */
    do {
        uint8_t *p = row;
        int32_t n = (int32_t)width;

        if (rows & 1) {
            p++;
            n--;
        }
        do {
            *p = (uint8_t)colour;
            p += 2;
            n -= 2;
        } while (n > 0);
        row += c.pitch;
    } while (--rows >= 0);
    return 0;
}

// FUNCTION: HGSIM 0x100e85cb
int32_t __cdecl Sim_100e85cb(const uint8_t *pane, int32_t x, int32_t y)
{
    W37Clip c;
    int32_t r = W37_Clip(pane, &c);

    if (r != 0)
        return r;
    x = ADD(x, c.x0);
    y = ADD(y, c.y0);
    if (x < c.cx0 || x > c.cx1 || y < c.cy0 || y > c.cy1)
        return -3;
    return c.buffer[(uint32_t)y * (uint32_t)c.pitch + (uint32_t)x];
}

// FUNCTION: HGSIM 0x100eda16
uint32_t __cdecl Sim_100eda16(const uint8_t *src, const uint8_t *dst, int32_t count, uint32_t state)
{
    W37Clip s, d;
    int32_t r, widthS, heightS, widthD, heightD, rows, cols;
    uint32_t edi, k, n, rowBits, bits, rowMask, taps, x, y;

    r = W37_Clip(src, &s);
    if (r != 0)
        return (uint32_t)r;
    widthS = ADD(DIFF(S32(src, 0xc), s.x0), 1);
    heightS = ADD(DIFF(S32(src, 0x10), s.y0), 1);
    /* the start of each source row from the pane's (unclipped) corner, [0x10153695] */
    edi = (uint32_t)s.y0 * (uint32_t)s.pitch + (uint32_t)(uintptr_t)s.buffer + (uint32_t)s.x0;
    for (k = 0, n = (uint32_t)heightS; k < n; k++, edi += (uint32_t)s.pitch)
        G32(0x10153695u + 4u * k) = edi;
    r = W37_Clip(dst, &d);
    if (r != 0)
        return (uint32_t)r;
    widthD = ADD(DIFF(S32(dst, 0xc), d.x0), 1);
    heightD = ADD(DIFF(S32(dst, 0x10), d.y0), 1);
    edi = (uint32_t)d.y0 * (uint32_t)d.pitch + (uint32_t)(uintptr_t)d.buffer + (uint32_t)d.x0;
    for (k = 0, n = (uint32_t)heightD; k < n; k++, edi += (uint32_t)d.pitch)
        G32(0x10154295u + 4u * k) = edi;
    rows = heightS < heightD ? heightS : heightD;
    cols = widthD > widthS ? widthS : widthD;
    /* the bits of the row number (low) and of the column (above them); the shift register's taps for that many
       bits come from the table in the library's code (0x100ed992) */
    bits = 0;
    rowMask = 0;
    n = (uint32_t)rows;
    do {
        bits++;
        rowMask = rowMask << 1 | 1u;
        n >>= 1;
    } while (n != 0);
    rowBits = bits;
    n = (uint32_t)cols;
    do {
        bits++;
        n >>= 1;
    } while (n != 0);
    taps = G32(0x100ed992u + 4u * bits);
    if (state == 0) {
        state = 1;
        y = 0;
        x = 0;
        goto copy;
    }
    for (;;) {
        x = state >> 1;
        if ((state & 1) != 0)
            x ^= taps;
        state = x;
        if (x == 1)
            count = 0;
        x >>= rowBits & 31;
        if (x >= (uint32_t)cols)
            continue;
        y = state & rowMask;
        if (y >= (uint32_t)rows)
            continue;
copy:
        count--;
        if (ADD(x, d.x0) < d.cx0 || ADD(x, d.x0) > d.cx1 || ADD(x, s.x0) < s.cx0 || ADD(x, s.x0) > s.cx1 ||
            ADD(y, d.y0) < d.cy0 || ADD(y, d.y0) > d.cy1 || ADD(y, s.y0) < s.cy0 || ADD(y, s.y0) > s.cy1)
            continue;
        *BYTES(G32(0x10154295u + 4u * y) + x) = *BYTES(G32(0x10153695u + 4u * y) + x);
        if (count < 0)
            return state;
    }
}

// FUNCTION: HGSIM 0x100e96a7
void __cdecl Sim_100e96a7(const uint32_t *table)
{
    int i;

    /* rep movsd of 0x40 dwords into the library's table at 0x10154eac */
    for (i = 0; i < 0x40; i++)
        G32(0x10154eacu + 4u * (uint32_t)i) = table[i];
}
