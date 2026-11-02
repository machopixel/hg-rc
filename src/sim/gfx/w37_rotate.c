#include "w37.h"
#include "w37_int.h"
#include "../simmem.h"
#include "../util/access.h"

/*
 * The rotated / scaled shape draw. The shape is first drawn upright into the caller's scratch image (0xff
 * transparent), its four corners turned and scaled about its origin (0x100ecc19) into the vertex table 0x10154fac
 * (4 x {x, y, unused, u, v}), and the quad is filled row by row from the image: two edges walked from the top
 * vertex (one backwards, one forwards through the table), each row's texels stepped in 16.16 with the four source
 * steps of the table 0x10154ffc picked by the u / v fraction carries.
 */
#define VTX(i, f) (SIM_PTR(s32a, 0x10154fac)[(i) * 5 + (f)])     /* f: 0 x, 1 y, 3 u, 4 v */
#define STEP SIM_PTR(s32a, 0x10154ffc)

/* x86 idiv of a 64-bit dividend by `d` (the quotient fits, d is not 0: else the original faults). */
static int32_t W37_IDiv(int64_t n, int32_t d)
{
    uint64_t un = n < 0 ? 0 - (uint64_t)n : (uint64_t)n;
    uint32_t ud = d < 0 ? 0u - (uint32_t)d : (uint32_t)d, q = 0, hi = (uint32_t)(un >> 32), lo = (uint32_t)un, top;
    int i;

    for (i = 31; i >= 0; i--) {
        top = hi >> 31;
        hi = hi << 1 | lo >> 31;
        lo <<= 1;
        if (top != 0 || hi >= ud) {
            hi -= ud;
            q |= 1u << i;
        }
    }
    return (n < 0) != (d < 0) ? (int32_t)(0u - q) : (int32_t)q;
}

/* An edge's 16.16 slope: (int16)d << 32 / (dy << 16). */
static int32_t W37_EdgeSlope(int32_t d, int32_t dy)
{
    return W37_IDiv((int64_t)(int16_t)d * 4294967296LL, (int32_t)((uint32_t)dy << 16));
}

/* A row's 16.16 step: d << 16 / (n << 16). */
static int32_t W37_RowSlope(int32_t d, int32_t n)
{
    return W37_IDiv((int64_t)d * 65536, (int32_t)((uint32_t)n << 16));
}

/* slope * (c << 16), bits 16..47 (imul, shrd 16). */
static int32_t W37_Advance(int32_t slope, int32_t c)
{
    return (int32_t)(uint32_t)((uint64_t)((int64_t)slope * (int32_t)((uint32_t)c << 16)) >> 16);
}

/* One edge being walked: the from / to vertices, the rows left, the 16.16 x, u, v and their steps. */
typedef struct {
    int from, to;
    int32_t left, x, u, v, dx, du, dv;
} W37Edge;

static void W37_EdgeSet(W37Edge *e, int32_t dy)
{
    e->left = dy;
    e->dx = W37_EdgeSlope(VTX(e->to, 0) - VTX(e->from, 0), dy);
    e->du = W37_EdgeSlope(VTX(e->to, 3) - VTX(e->from, 3), dy);
    e->dv = W37_EdgeSlope(VTX(e->to, 4) - VTX(e->from, 4), dy);
    e->x = (int32_t)(((uint32_t)VTX(e->from, 0) << 16) + 0x8000);
    e->u = (int32_t)(((uint32_t)VTX(e->from, 3) << 16) + 0x8000);
    e->v = (int32_t)(((uint32_t)VTX(e->from, 4) << 16) + 0x8000);
}

/* The first edge from `start` (`dir` 3 backwards, 1 forwards) that reaches below the top of the box and is not
   flat. */
static void W37_EdgeFirst(W37Edge *e, int start, int dir, int32_t cy0)
{
    int32_t dy;

    e->to = start;
    for (;;) {
        e->from = e->to;
        e->to = (e->to + dir) & 3;
        if (VTX(e->from, 1) < cy0 && VTX(e->to, 1) <= cy0)
            continue;
        dy = DIFF(VTX(e->to, 1), VTX(e->from, 1));
        if (dy != 0)
            break;
    }
    W37_EdgeSet(e, dy);
}

/* The next edge (a flat one counts as one row). */
static void W37_EdgeNext(W37Edge *e, int dir)
{
    int32_t dy;

    e->from = e->to;
    e->to = (e->to + dir) & 3;
    dy = DIFF(VTX(e->to, 1), VTX(e->from, 1));
    W37_EdgeSet(e, dy == 0 ? 1 : dy);
}

static void W37_EdgeStep(W37Edge *e)
{
    e->x += e->dx;
    e->u += e->du;
    e->v += e->dv;
}

/* The edge moved down to row `row` (from its from-vertex's row). */
static void W37_EdgeClip(W37Edge *e, int32_t row)
{
    int32_t c = DIFF(row, VTX(e->from, 1));

    e->left -= c;
    e->x += W37_Advance(e->dx, c);
    e->u += W37_Advance(e->du, c);
    e->v += W37_Advance(e->dv, c);
}

// FUNCTION: HGSIM 0x100e9cd1
int32_t __cdecl Sim_100e9cd1(const uint8_t *pane, const uint8_t *table, int32_t number, int32_t x, int32_t y,
                             uint8_t *image, int32_t angle, int32_t sx, int32_t sy, uint32_t flags)
{
    const uint8_t *window;
    uint32_t swin[3], spane[5], size, origin, spitch;
    int32_t w1, h1, centre[2], in[2], out[2], bx, by, k;
    int32_t pitch, cx0, cy0, cx1, cy1, x0, y0, miny, maxy, last, rows, row, t;
    int32_t qu = 0, qv = 0;
    uint32_t outside;
    int top = 0, i;
    uint8_t *buffer, *rowp;
    W37Edge l, r;

    if (sx == 0x10000 && sy == 0x10000 && angle == 0) {
        if (flags & 1)
            return Sim_100e96c6(pane, table, number, x, y);
        return Sim_100e91dc(pane, table, number, x, y);
    }
    /* the scratch pane over the image, as large as the shape; the corners' texel coordinates */
    size = Sim_100ed788(table, number);
    w1 = (int32_t)(size >> 16) - 1;
    h1 = (int32_t)(size & 0xffff) - 1;
    spane[0] = (uint32_t)(uintptr_t)swin;
    swin[0] = (uint32_t)(uintptr_t)image;
    spane[1] = 0;
    VTX(0, 3) = 0;
    VTX(3, 3) = 0;
    spane[2] = 0;
    VTX(0, 4) = 0;
    VTX(1, 4) = 0;
    swin[1] = (uint32_t)w1;
    spane[3] = (uint32_t)w1;
    VTX(1, 3) = w1;
    VTX(2, 3) = w1;
    swin[2] = (uint32_t)h1;
    spane[4] = (uint32_t)h1;
    VTX(2, 4) = h1;
    VTX(3, 4) = h1;
    origin = Sim_100ed7bc(table, number);
    centre[1] = -(int32_t)(int16_t)origin;
    centre[0] = -((int32_t)origin >> 16);
    if ((flags & 2) == 0) {
        Sim_100eb070((const uint8_t *)spane, 0xff);
        if (flags & 1)
            Sim_100e96c6((const uint8_t *)spane, table, number, centre[0], centre[1]);
        else
            Sim_100e91dc((const uint8_t *)spane, table, number, centre[0], centre[1]);
    }
    window = (const uint8_t *)(uintptr_t)U32(pane, 0);
    if (S32(window, 4) < 0 || S32(window, 8) < 0)
        return -1;
    pitch = ADD(S32(window, 4), 1);
    x0 = S32(pane, 4);
    y0 = S32(pane, 8);
    cx0 = x0 > 0 ? x0 : 0;
    cy0 = y0 > 0 ? y0 : 0;
    cx1 = S32(pane, 0xc) < S32(window, 4) ? S32(pane, 0xc) : S32(window, 4);
    cy1 = S32(pane, 0x10) < S32(window, 8) ? S32(pane, 0x10) : S32(window, 8);
    if (cx1 < cx0 || cy1 < cy0)
        return -2;
    buffer = (uint8_t *)(uintptr_t)U32(window, 0);
    /* the corners turned and scaled about the origin, placed at (x, y) in the pane */
    bx = DIFF(x, centre[0]);
    by = DIFF(y, centre[1]);
    for (k = 0; k < 4; k++) {
        in[0] = (k == 1 || k == 2) ? w1 : 0;
        in[1] = k >= 2 ? h1 : 0;
        Sim_100ecc19(in, out, centre, angle, sx, sy);
        VTX(k, 0) = ADD(ADD(out[0], bx), x0);
        VTX(k, 1) = ADD(ADD(out[1], by), y0);
    }
    spitch = swin[1] + 1;
    /* the top vertex (the last of equal ones), the bottom row; nothing when all corners are off one side */
    miny = 0x7fff;
    maxy = -0x8000;
    outside = 0xf;
    last = 0;
    for (i = 0; i < 4; i++) {
        uint32_t code = (uint32_t)DIFF(VTX(i, 0), cx0) >> 31 << 3 | (uint32_t)DIFF(cx1, VTX(i, 0)) >> 31 << 2
                        | (uint32_t)DIFF(VTX(i, 1), cy0) >> 31 << 1 | (uint32_t)DIFF(cy1, VTX(i, 1)) >> 31;

        last = VTX(i, 1);
        if (last <= miny) {
            miny = last;
            top = i;
        }
        if (last >= maxy)
            maxy = last;
        outside &= code;
    }
    if (outside != 0)
        return last;
    if (maxy == miny)
        return (int32_t)(uintptr_t)&VTX(top, 0);
    W37_EdgeFirst(&l, top, 3, cy0);
    W37_EdgeFirst(&r, top, 1, cy0);
    rows = DIFF(cy1, miny);
    if ((int64_t)maxy - cy1 <= 0)
        rows = ADD(rows, DIFF(maxy, cy1));
    row = miny;
    t = DIFF(cy0, miny);
    if ((int64_t)cy0 - miny > 0) {
        rows -= t;
        row = cy0;
        W37_EdgeClip(&l, cy0);
        W37_EdgeClip(&r, cy0);
    }
    rowp = buffer + (uint32_t)row * (uint32_t)pitch;
    for (;;) {
        int32_t xa = l.x, xb = r.x, ua = l.u, ub = r.u, va = l.v, vb = r.v;
        int32_t sx0, sx1, su, sv, count, n, c;

        if (!(r.x > l.x)) {
            xa = r.x;
            xb = l.x;
            ua = r.u;
            ub = l.u;
            va = r.v;
            vb = l.v;
        }
        sx0 = xa >> 16;
        sx1 = xb >> 16;
        if (sx0 <= cx1 && sx1 >= cx0) {
            const uint8_t *src;
            uint8_t *dst;
            uint32_t fu, fv, stu, stv;

            su = ua;
            sv = va;
            n = DIFF(sx1, sx0);
            if (n != 0) {
                uint32_t ui, uc, vi, vf;
                int32_t vstep, vrow;

                qu = W37_RowSlope(DIFF(ub, ua), n);
                ui = (uint32_t)qu >> 16 & 0xffff;
                uc = 1;
                if (ui & 0x8000) {
                    ui |= 0xffff0000u;
                    uc = 0xffffffffu;
                    ui += ((uint32_t)qu & 0xffff) != 0;
                }
                uc += ui;
                qv = W37_RowSlope(DIFF(vb, va), n);
                vi = (uint32_t)qv >> 16 & 0xffff;
                vf = (uint32_t)qv & 0xffff;
                vstep = (int32_t)spitch;
                if (vi & 0x8000) {
                    vstep = -vstep;
                    vi += vf != 0;
                }
                vrow = (int16_t)(uint16_t)((uint32_t)(int16_t)spitch * (uint32_t)(int16_t)vi);
                STEP[0] = (int32_t)(ui + (uint32_t)vrow);
                STEP[1] = (int32_t)(ui + (uint32_t)vrow + (uint32_t)vstep);
                STEP[2] = (int32_t)(uc + (uint32_t)vrow);
                STEP[3] = (int32_t)(uc + (uint32_t)vrow + (uint32_t)vstep);
                c = DIFF(cx0, sx0);
                if ((int64_t)cx0 - sx0 > 0) {
                    sx0 += c;
                    su += W37_Advance(qu, c);
                    sv += W37_Advance(qv, c);
                }
                if ((int64_t)sx1 - cx1 > 0)
                    sx1 -= DIFF(sx1, cx1);
            }
            src = image + ((uint32_t)sv >> 16) * spitch + ((uint32_t)su >> 16);
            dst = rowp + sx0;
            count = DIFF(sx1, sx0);
            fu = (uint32_t)su;
            stu = (uint32_t)qu;
            if (qu < 0) {
                stu = 0u - stu;
                fu = ~fu;
            }
            stu <<= 16;
            fu <<= 16;
            fv = (uint32_t)sv;
            stv = (uint32_t)qv;
            if (qv < 0) {
                stv = 0u - stv;
                fv = ~fv;
            }
            stv <<= 16;
            fv <<= 16;
            do {
                uint32_t cu, cv;

                if (*src != 0xff)
                    *dst = *src;
                dst++;
                cu = fu + stu < fu;
                fu += stu;
                cv = fv + stv < fv;
                fv += stv;
                src += STEP[cu * 2 + cv];
            } while (--count >= 0);
        }
        rowp += pitch;
        rows--;
        if (rows < 0)
            return l.x;
        if (rows == 0) {
            W37_EdgeStep(&l);
            W37_EdgeStep(&r);
            continue;
        }
        if (--l.left == 0)
            W37_EdgeNext(&l, 3);
        else
            W37_EdgeStep(&l);
        if (--r.left == 0)
            W37_EdgeNext(&r, 1);
        else
            W37_EdgeStep(&r);
    }
}
