#include <string.h>
#include "view.h"
#include "../simmem.h"

/* Display size - 1, see view.h. */
#define MAX_X SIM_PTR(int32_t, 0x109e3e14)
#define MAX_Y SIM_PTR(int32_t, 0x109e3de0)

/* 16.16 multiply, rounded: the 64-bit product shifted right 16 plus the last bit shifted out (shrd / adc). */
static int32_t FixMul(int32_t a, int32_t b)
{
    int64_t p = (int64_t)a * b;

    return (int32_t)((uint32_t)(p >> 16) + (uint32_t)((p >> 15) & 1));
}

// FUNCTION: HGSIM 0x1008ca50
int32_t *__cdecl View_ScaleRect(void *unused, const int32_t *rect, int32_t *out)
{
    (void)unused;
    out[1] = FixMul(*MAX_X, rect[1]);
    out[2] = FixMul(*MAX_Y, rect[2]);
    out[3] = FixMul(*MAX_X, rect[3]);
    out[4] = FixMul(*MAX_Y, rect[4]);
    return out;
}

// FUNCTION: HGSIM 0x1008cafb
int32_t *__cdecl Rect_ScaleToRect(const int32_t *area, const int32_t *rect, int32_t *out)
{
    int32_t w = (int32_t)((uint32_t)area[3] - (uint32_t)area[1]);
    int32_t h = (int32_t)((uint32_t)area[4] - (uint32_t)area[2]);

    out[1] = FixMul(rect[1], w);
    out[2] = FixMul(rect[2], h);
    out[3] = FixMul(rect[3], w);
    out[4] = FixMul(rect[4], h);
    out[1] = (int32_t)((uint32_t)out[1] + (uint32_t)area[1]);
    out[2] = (int32_t)((uint32_t)out[2] + (uint32_t)area[2]);
    out[3] = (int32_t)((uint32_t)out[3] + (uint32_t)area[1]);
    out[4] = (int32_t)((uint32_t)out[4] + (uint32_t)area[2]);
    return out;
}

// FUNCTION: HGSIM 0x1008ccad
int32_t *__cdecl View_ScalePoint(void *unused, const int32_t *point, int32_t *out)
{
    (void)unused;
    out[0] = FixMul(*MAX_X, point[0]);
    out[1] = FixMul(*MAX_Y, point[1]);
    return out;
}

// FUNCTION: HGSIM 0x1008cdc9
int32_t *__cdecl Rect_ScalePoint(const int32_t *area, const int32_t *point, int32_t *out)
{
    int32_t w = (int32_t)((uint32_t)area[3] - (uint32_t)area[1]);
    int32_t h = (int32_t)((uint32_t)area[4] - (uint32_t)area[2]);

    out[0] = FixMul(w, point[0]);
    out[1] = FixMul(h, point[1]);
    return out;
}

// FUNCTION: HGSIM 0x1008d0da
int32_t *__cdecl View_CenterRect(void *unused, const int32_t *rect, int32_t *out)
{
    uint32_t r[5];
    int32_t offX, offY;

    (void)unused;
    memcpy(r, rect, sizeof r);
    r[3] -= r[1];
    r[1] = 0;
    r[4] -= r[2];
    r[2] = 0;
    /* width and height of the rect, counted in pixels (inclusive) */
    offX = (int32_t)((uint32_t)*MAX_X - (r[3] - r[1] + 1) - 1) / 2;
    offY = (int32_t)((uint32_t)*MAX_Y - (r[4] - r[2] + 1) - 1) / 2;
    out[1] = (int32_t)(r[1] + (uint32_t)offX);
    out[3] = (int32_t)(r[3] + (uint32_t)offX);
    out[2] = (int32_t)(r[2] + (uint32_t)offY);
    out[4] = (int32_t)(r[4] + (uint32_t)offY);
    return out;
}

// FUNCTION: HGSIM 0x1008f668
int32_t *__cdecl Rect_Interpolate(const int32_t *a, const int32_t *b, int32_t *out, int32_t t, int32_t skip)
{
    if (skip != 2) {
        out[1] = (int32_t)((uint32_t)a[1] + (uint32_t)FixMul(t, (int32_t)((uint32_t)b[1] - (uint32_t)a[1])));
        out[3] = (int32_t)((uint32_t)a[3] + (uint32_t)FixMul(t, (int32_t)((uint32_t)b[3] - (uint32_t)a[3])));
    }
    if (skip != 1) {
        out[2] = (int32_t)((uint32_t)a[2] + (uint32_t)FixMul(t, (int32_t)((uint32_t)b[2] - (uint32_t)a[2])));
        out[4] = (int32_t)((uint32_t)a[4] + (uint32_t)FixMul(t, (int32_t)((uint32_t)b[4] - (uint32_t)a[4])));
    }
    return out;
}
