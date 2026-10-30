/*
 * Full-resolution 2D layer of HGVK.DLL (see hud2d.h, docs/renderer/hud.md).
 *
 * The sim draws its HUD with a small 2D library (lines, rectangle wipes, RLE
 * shapes, text glyphs) that works in 640x480 cells of RGB565. hooks_hud.c
 * reports each of those drawing calls here, and they are redrawn on a picture
 * that has the resolution of the window: lines with smooth edges, fills snapped
 * to whole pixels, and bitmaps (the game's own shapes and font glyphs) scaled
 * with a "sharp bilinear" filter, which keeps every source pixel crisp at any
 * scale and blends only along its edges.
 *
 * Colours are premultiplied RGBA8. An RGB565 value of 0 is transparent, as it
 * always was for the old overlay.
 */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hud2d.h"
#include "vk.h"

static const void *g_target;
static uint32_t *g_px;
static size_t g_cap; /* pixels allocated */
static int g_w, g_h;
static float g_scale = 1.0f;
static int g_used;

/* Per-frame counts, logged by F12. */
static unsigned g_nLine, g_nLineAxis, g_nFill, g_nBitmap;

/* 0: bitmaps keep hard cell edges (the look of the old HUD and the menus); 1: sharp bilinear. */
#define HUD_SMOOTH 0

/* Columns and rows of the bitmap filter. */
static int *g_colA, *g_colB, *g_colW, *g_rowA, *g_rowB, *g_rowW;
static int g_colCap, g_rowCap;

static uint32_t g_expand[65536]; /* RGB565 to premultiplied RGBA8 */
static int g_expandReady;

static void InitExpand(void)
{
    unsigned p;

    for (p = 0; p < 65536; p++) {
        unsigned r = (p >> 11) & 31, gr = (p >> 5) & 63, b = p & 31;

        g_expand[p] = p == 0 ? 0u
                             : (r * 255 / 31) | ((gr * 255 / 63) << 8) | ((b * 255 / 31) << 16) |
                                   0xff000000u;
    }
    g_expandReady = 1;
}

void Hud_SetTarget(const void *rgb565)
{
    g_target = rgb565;
}

static const void *ApiTarget(void)
{
    return g_target;
}

/* Cell edge to pixel edge. Neighbouring cells share their edge, so nothing leaves gaps. */
static int Px(int cell)
{
    return (int)floorf((float)cell * g_scale + 0.5f);
}

static int Clamp(int v, int lo, int hi)
{
    return v < lo ? lo : (v > hi ? hi : v);
}

/* The clip rectangle in pixels, end exclusive. Returns 0 when nothing is left. */
static int ClipPixels(const HudRect *clip, int *x0, int *y0, int *x1, int *y1)
{
    *x0 = Clamp(Px(clip->x0), 0, g_w);
    *y0 = Clamp(Px(clip->y0), 0, g_h);
    *x1 = Clamp(Px(clip->x1 + 1), 0, g_w);
    *y1 = Clamp(Px(clip->y1 + 1), 0, g_h);
    return *x1 > *x0 && *y1 > *y0;
}

void Hud_Frame(int clientW, int clientH)
{
    float sx = (float)clientW / HUD_CELLS_W, sy = (float)clientH / HUD_CELLS_H;
    float s = sx < sy ? sx : sy;
    int w, h;

    if (!g_expandReady)
        InitExpand();
    if (clientW <= 0 || clientH <= 0 || s <= 0.0f) {
        g_w = g_h = 0;
        g_used = 0;
        return;
    }
    w = (int)(HUD_CELLS_W * s);
    h = (int)(HUD_CELLS_H * s);
    if (w < 1)
        w = 1;
    if (h < 1)
        h = 1;
    if ((size_t)w * (size_t)h > g_cap) {
        free(g_px);
        g_px = (uint32_t *)calloc((size_t)w * (size_t)h, sizeof(uint32_t));
        g_cap = g_px != NULL ? (size_t)w * (size_t)h : 0;
        g_used = 0;
    } else if (g_used || w != g_w || h != g_h) {
        memset(g_px, 0, (size_t)w * (size_t)h * sizeof(uint32_t));
    }
    if (g_px == NULL) {
        g_w = g_h = 0;
        return;
    }
    if (w > g_colCap) {
        free(g_colA);
        free(g_colB);
        free(g_colW);
        g_colA = (int *)malloc((size_t)w * sizeof(int));
        g_colB = (int *)malloc((size_t)w * sizeof(int));
        g_colW = (int *)malloc((size_t)w * sizeof(int));
        g_colCap = g_colA != NULL && g_colB != NULL && g_colW != NULL ? w : 0;
    }
    if (h > g_rowCap) {
        free(g_rowA);
        free(g_rowB);
        free(g_rowW);
        g_rowA = (int *)malloc((size_t)h * sizeof(int));
        g_rowB = (int *)malloc((size_t)h * sizeof(int));
        g_rowW = (int *)malloc((size_t)h * sizeof(int));
        g_rowCap = g_rowA != NULL && g_rowB != NULL && g_rowW != NULL ? h : 0;
    }
    g_w = w;
    g_h = h;
    g_scale = s;
    g_used = 0;
    g_nLine = g_nLineAxis = g_nFill = g_nBitmap = 0;
}

const uint32_t *Hud_Pixels(int *w, int *h)
{
    if (!g_used || g_px == NULL || g_w <= 0)
        return NULL;
    *w = g_w;
    *h = g_h;
    return g_px;
}

/* src over dst, both premultiplied. */
static uint32_t Over(uint32_t d, uint32_t s)
{
    unsigned sa = s >> 24, inv;

    if (sa == 255)
        return s;
    if (sa == 0)
        return d;
    inv = 255 - sa;
    return (((s & 0xff) + ((d & 0xff) * inv + 127) / 255) & 0xff) |
           ((((s >> 8 & 0xff) + ((d >> 8 & 0xff) * inv + 127) / 255) & 0xff) << 8) |
           ((((s >> 16 & 0xff) + ((d >> 16 & 0xff) * inv + 127) / 255) & 0xff) << 16) |
           ((((s >> 24) + ((d >> 24) * inv + 127) / 255) & 0xff) << 24);
}

static uint32_t Scale(uint32_t c, float cov)
{
    unsigned k = (unsigned)(cov * 256.0f + 0.5f);

    if (k >= 256)
        return c;
    return (((c & 0xff) * k >> 8) & 0xff) | ((((c >> 8 & 0xff) * k >> 8) & 0xff) << 8) |
           ((((c >> 16 & 0xff) * k >> 8) & 0xff) << 16) | ((((c >> 24) * k >> 8) & 0xff) << 24);
}

/* ---- Fill ---------------------------------------------------------------- */

static void FillPixels(int x0, int y0, int x1, int y1, uint32_t value)
{
    int x, y;

    for (y = y0; y < y1; y++) {
        uint32_t *row = g_px + (size_t)y * (size_t)g_w;

        for (x = x0; x < x1; x++)
            row[x] = value;
    }
}

static void ApiFill(const HudRect *clip, unsigned color)
{
    int x0, y0, x1, y1;

    if (g_px == NULL || !ClipPixels(clip, &x0, &y0, &x1, &y1))
        return;
    g_nFill++;
    g_used = 1;
    FillPixels(x0, y0, x1, y1, g_expand[color & 0xffff]);
}

/* ---- Lines --------------------------------------------------------------- */

static void LineCell(int cx0, int cy0, int cx1, int cy1, int px0, int py0, int px1, int py1,
                     uint32_t color)
{
    int x0 = Clamp(Px(cx0), px0, px1), x1 = Clamp(Px(cx1 + 1), px0, px1);
    int y0 = Clamp(Px(cy0), py0, py1), y1 = Clamp(Px(cy1 + 1), py0, py1);
    int x, y;

    for (y = y0; y < y1; y++) {
        uint32_t *row = g_px + (size_t)y * (size_t)g_w;

        for (x = x0; x < x1; x++)
            row[x] = Over(row[x], color);
    }
}

/*
 * A slanted line is a one-cell-wide band: a box around the segment that also
 * covers its two end cells, with coverage from the distance to the box edge so
 * the edges are smooth. Only the pixels near the line are visited.
 */
static void ApiLine(const HudRect *clip, int x0, int y0, int x1, int y1, unsigned colorIn)
{
    uint32_t color = g_expand[colorIn & 0xffff];
    int cx0, cy0, cx1, cy1;
    float s = g_scale, ax, ay, bx, by, dx, dy, len, ux, uy, mx, my, halfLen, halfT, ext;
    int steep, lo, hi, t;

    if (g_px == NULL || color == 0 || !ClipPixels(clip, &cx0, &cy0, &cx1, &cy1))
        return;
    g_nLine++;
    g_used = 1;
    if (x0 == x1 || y0 == y1) {
        g_nLineAxis++;
        LineCell(x0 < x1 ? x0 : x1, y0 < y1 ? y0 : y1, x0 < x1 ? x1 : x0, y0 < y1 ? y1 : y0, cx0,
                 cy0, cx1, cy1, color);
        return;
    }
    ax = ((float)x0 + 0.5f) * s;
    ay = ((float)y0 + 0.5f) * s;
    bx = ((float)x1 + 0.5f) * s;
    by = ((float)y1 + 0.5f) * s;
    dx = bx - ax;
    dy = by - ay;
    len = sqrtf(dx * dx + dy * dy);
    ux = dx / len;
    uy = dy / len;
    mx = (ax + bx) * 0.5f;
    my = (ay + by) * 0.5f;
    ext = 0.5f * s * (fabsf(ux) + fabsf(uy)); /* reach of an end cell along the line */
    halfLen = len * 0.5f + ext;
    halfT = 0.5f * s;
    steep = fabsf(dy) > fabsf(dx);

    /* Walk along the major axis; for each step cover the band across the minor axis. */
    {
        float aMin = (steep ? (ay < by ? ay : by) : (ax < bx ? ax : bx)) - ext - 1.0f;
        float aMax = (steep ? (ay < by ? by : ay) : (ax < bx ? bx : ax)) + ext + 1.0f;
        int a0 = (int)floorf(aMin), a1 = (int)ceilf(aMax);
        float slope = steep ? dx / dy : dy / dx;
        float reach = halfT / (steep ? fabsf(uy) : fabsf(ux)) + 1.5f;

        lo = steep ? cy0 : cx0;
        hi = steep ? cy1 : cx1;
        if (a0 < lo)
            a0 = lo;
        if (a1 > hi)
            a1 = hi;
        for (t = a0; t < a1; t++) {
            float tc = (float)t + 0.5f;
            float mc = steep ? ax + (tc - ay) * slope : ay + (tc - ax) * slope;
            int m0 = (int)floorf(mc - reach), m1 = (int)ceilf(mc + reach), m;
            int mlo = steep ? cx0 : cy0, mhi = steep ? cx1 : cy1;

            if (m0 < mlo)
                m0 = mlo;
            if (m1 > mhi)
                m1 = mhi;
            for (m = m0; m < m1; m++) {
                float px = steep ? (float)m + 0.5f : tc, py = steep ? tc : (float)m + 0.5f;
                float al = (px - mx) * ux + (py - my) * uy;
                float pe = (px - mx) * uy - (py - my) * ux;
                float qx = fabsf(al) - halfLen, qy = fabsf(pe) - halfT, sd, cov;
                float ox = qx > 0.0f ? qx : 0.0f, oy = qy > 0.0f ? qy : 0.0f;
                uint32_t *dst;

                sd = sqrtf(ox * ox + oy * oy) + (qx > qy ? (qx < 0.0f ? qx : 0.0f)
                                                         : (qy < 0.0f ? qy : 0.0f));
                cov = 0.5f - sd;
                if (cov <= 0.0f)
                    continue;
                if (cov > 1.0f)
                    cov = 1.0f;
                dst = &g_px[(size_t)(steep ? t : m) * (size_t)g_w + (size_t)(steep ? m : t)];
                *dst = Over(*dst, Scale(color, cov));
            }
        }
    }
}

/* ---- Bitmaps ------------------------------------------------------------- */

/*
 * For each destination pixel along one axis: the two source cells to mix and
 * the weight (0..256) of the second. The weight ramps over one destination
 * pixel only, at the edge between two source cells ("sharp bilinear").
 */
static void Taps(int dst0, int dst1, int origin, int count, int *a, int *b, int *wt)
{
    float s = g_scale, o = (float)origin * s;
    int d;

    for (d = dst0; d < dst1; d++) {
        float su = ((float)d + 0.5f - o) / s - 0.5f;
        float fl = floorf(su), fr = su - fl;
        int i = (int)fl;

        if (!HUD_SMOOTH) {
            /* Hard-edged cells, like the rest of the UI: each pixel takes the cell it lies in. */
            i = (int)floorf(((float)d + 0.5f - o) / s);
            a[d - dst0] = b[d - dst0] = Clamp(i, 0, count - 1);
            wt[d - dst0] = 0;
            continue;
        }
        fr = (fr - 0.5f) * s + 0.5f;
        fr = fr < 0.0f ? 0.0f : (fr > 1.0f ? 1.0f : fr);
        a[d - dst0] = Clamp(i, 0, count - 1);
        b[d - dst0] = Clamp(i + 1, 0, count - 1);
        wt[d - dst0] = (int)(fr * 256.0f + 0.5f);
    }
}

static void ApiBitmap(const HudRect *clip, int x, int y, int w, int h, const uint16_t *src)
{
    int cx0, cy0, cx1, cy1, X0, Y0, X1, Y1, dx, dy;

    if (g_px == NULL || w <= 0 || h <= 0 || g_colCap == 0 || g_rowCap == 0 ||
        !ClipPixels(clip, &cx0, &cy0, &cx1, &cy1))
        return;
    X0 = Px(x) > cx0 ? Px(x) : cx0;
    X1 = Px(x + w) < cx1 ? Px(x + w) : cx1;
    Y0 = Px(y) > cy0 ? Px(y) : cy0;
    Y1 = Px(y + h) < cy1 ? Px(y + h) : cy1;
    if (X1 <= X0 || Y1 <= Y0)
        return;
    g_nBitmap++;
    g_used = 1;
    Taps(X0, X1, x, w, g_colA, g_colB, g_colW);
    Taps(Y0, Y1, y, h, g_rowA, g_rowB, g_rowW);
    for (dy = Y0; dy < Y1; dy++) {
        const uint16_t *r0 = src + (size_t)g_rowA[dy - Y0] * (size_t)w;
        const uint16_t *r1 = src + (size_t)g_rowB[dy - Y0] * (size_t)w;
        int wy = g_rowW[dy - Y0];
        uint32_t *out = g_px + (size_t)dy * (size_t)g_w;

        for (dx = X0; dx < X1; dx++) {
            int ca = g_colA[dx - X0], cb = g_colB[dx - X0], wx = g_colW[dx - X0];
            unsigned p00 = r0[ca], p01 = r0[cb], p10 = r1[ca], p11 = r1[cb];
            uint32_t c;

            if (p00 == p01 && p10 == p11 && p00 == p10) {
                c = g_expand[p00];
            } else if ((wx == 0 || p00 == p01) && (wy == 0 || p00 == p10) &&
                       (wx == 0 || wy == 0 || p00 == p11)) {
                c = g_expand[p00]; /* all the weight is on one cell */
            } else {
                uint32_t t00 = g_expand[p00], t01 = g_expand[p01], t10 = g_expand[p10],
                         t11 = g_expand[p11];
                unsigned w00 = (unsigned)((256 - wx) * (256 - wy)), w01 = (unsigned)(wx * (256 - wy)),
                         w10 = (unsigned)((256 - wx) * wy), w11 = (unsigned)(wx * wy);
                unsigned ch, sum;

                c = 0;
                for (ch = 0; ch < 32; ch += 8) {
                    sum = ((t00 >> ch) & 0xff) * w00 + ((t01 >> ch) & 0xff) * w01 +
                          ((t10 >> ch) & 0xff) * w10 + ((t11 >> ch) & 0xff) * w11;
                    c |= (((sum + 32768) >> 16) & 0xff) << ch;
                }
            }
            if (c != 0)
                out[dx] = Over(out[dx], c);
        }
    }
}

/* ---- F12 ----------------------------------------------------------------- */

void Hud_Dump(const char *bmpPath)
{
    FILE *fp;
    unsigned char *row;
    int x, y;
    unsigned char hdr[54];
    unsigned pad, rowBytes, size;

    HgvkLog("F12: full-resolution HUD %dx%d scale %.3f: %u lines (%u straight), %u fills, "
            "%u bitmaps%s",
            g_w, g_h, (double)g_scale, g_nLine, g_nLineAxis, g_nFill, g_nBitmap,
            g_used ? "" : " (nothing drawn)");
    if (!g_used || g_px == NULL)
        return;
    fp = fopen(bmpPath, "wb");
    if (fp == NULL)
        return;
    rowBytes = (unsigned)g_w * 3;
    pad = (4 - (rowBytes & 3)) & 3;
    size = 54 + (rowBytes + pad) * (unsigned)g_h;
    row = (unsigned char *)malloc(rowBytes + pad);
    if (row == NULL) {
        fclose(fp);
        return;
    }
    memset(hdr, 0, sizeof(hdr));
    hdr[0] = 'B';
    hdr[1] = 'M';
    memcpy(hdr + 2, &size, 4);
    hdr[10] = 54;
    hdr[14] = 40;
    memcpy(hdr + 18, &g_w, 4);
    memcpy(hdr + 22, &g_h, 4); /* positive: rows bottom-up */
    hdr[26] = 1;
    hdr[28] = 24;
    fwrite(hdr, sizeof(hdr), 1, fp);
    memset(row, 0, rowBytes + pad);
    for (y = g_h - 1; y >= 0; y--) {
        for (x = 0; x < g_w; x++) {
            uint32_t p = g_px[(size_t)y * (size_t)g_w + (size_t)x]; /* premultiplied: over black */

            row[x * 3] = (unsigned char)(p >> 16);
            row[x * 3 + 1] = (unsigned char)(p >> 8);
            row[x * 3 + 2] = (unsigned char)p;
        }
        fwrite(row, rowBytes + pad, 1, fp);
    }
    free(row);
    fclose(fp);
    HgvkLog("F12: full-resolution HUD saved as %s", bmpPath);
}

static const HudApi g_api = {HUD_API_VERSION, ApiTarget, ApiLine, ApiFill, ApiBitmap};

const HudApi *Hud_Api(void)
{
    return &g_api;
}
