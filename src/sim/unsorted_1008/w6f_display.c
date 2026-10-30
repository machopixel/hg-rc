#include "w6f.h"
#include "w2i.h"
#include "w3q.h"
#include "w5y.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x10084b8c
double __cdecl Sim_10084b8c(float x, float y, float z)
{
    double a = x;
    double b = y;
    double c = z;

    return W6F_SQRT(a * a + b * b + c * c);
}

// FUNCTION: HGSIM 0x10086d8d
void __cdecl Display_SizeFromDriverName(const char *name)
{
    if (W6F_STRCMPI(name, SIM_PTR(const char, 0x1014b6cc)) == 0) {          /* "MCGA.DLL" */
        G32(0x1017b918) = 320;
        G32(0x1017b91c) = 200;
    } else if (W6F_STRCMPI(name, SIM_PTR(const char, 0x1014b6d8)) == 0) {   /* "VESA480.DLL" */
        G32(0x1017b918) = 640;
        G32(0x1017b91c) = 480;
    } else if (W6F_STRCMPI(name, SIM_PTR(const char, 0x1014b6e4)) == 0) {   /* "VESA768.DLL" */
        G32(0x1017b918) = 1024;
        G32(0x1017b91c) = 768;
    } else {
        G32(0x1017b918) = 320;
        G32(0x1017b91c) = 200;
    }
}

/*
 * The original's `cdq; shld edx, eax, 16; shl eax, 16; idiv d`: the 64-bit value x * 65536 divided by d,
 * truncated. x is a 16-bit quantity here (|x| <= 0xffff), so the dividend's magnitude fits 32 bits. Where
 * idiv raises a divide error (d == 0 or a quotient outside int32) the same error is raised.
 */
static int32_t FixedDiv16(int32_t x, int32_t d)
{
    uint32_t mag = (x < 0 ? 0u - (uint32_t)x : (uint32_t)x) << 16;
    uint32_t dmag = d < 0 ? 0u - (uint32_t)d : (uint32_t)d;
    uint32_t q;

    if (d == 0 || ((x < 0) == (d < 0) ? mag / dmag > 0x7fffffffu : mag / dmag > 0x80000000u)) {
        volatile int32_t n = INT32_MIN, m = -1;

        return n / m;       /* idiv overflow: the divide error the original raises */
    }
    q = mag / dmag;
    return (int32_t)((x < 0) == (d < 0) ? q : 0u - q);
}

// FUNCTION: HGSIM 0x1008d2d0
int32_t *__cdecl Sim_1008d2d0(const int32_t *rect, int32_t *out, void *table, int32_t index)
{
    int32_t size = W6F_CALL_100ed743(table, index);
    int32_t sx = size >> 16;
    int32_t sy = size & 0xffff;

    sx = FixedDiv16(sx, (int32_t)((uint32_t)rect[3] - (uint32_t)rect[1] + 1u));
    sy = FixedDiv16(sy, (int32_t)((uint32_t)rect[4] - (uint32_t)rect[2] + 1u));
    Rect_ScaleAroundCenter(rect, out, sx, sy);
    return out;
}

// FUNCTION: HGSIM 0x1008e02b
int32_t __cdecl Sim_1008e02b(void *bitmap)
{
    int32_t right, bottom;

    if ((int32_t)(G32(0x1015dc78) - G32(0x1017cc10)) >= 0x5a) {
        G32(0x1017cc10) = G32(0x1015dc78);
        G32(0x1014b9b0) = G32(0x1014b9b0) + G32(0x1017cc14);
        if (G32(0x1014b9b0) == 0xf0)
            G32(0x1017cc14) = 1;
        else if (G32(0x1014b9b0) == 0xf7)
            G32(0x1017cc14) = 0u - G32(0x1017cc14);
    }
    GS32(0x1017cc0c) = GS32(0x1017cc0c) % 2;
    GS32(0x1017cc0c) = GS32(0x1017cc0c) + 1;
    right = (int32_t)(U32(bitmap, 0xc) - G32(0x1017cc0c));
    bottom = (int32_t)(U32(bitmap, 0x10) - G32(0x1017cc0c));
    return W6F_CALL_100e90a3(bitmap, GS32(0x1017cc0c), GS32(0x1017cc0c), right, bottom, G32(0x1014b9b0));
}

// FUNCTION: HGSIM 0x1008e266
int32_t *__cdecl Sim_1008e266(const int32_t *rect, const int32_t *pt, int32_t *out)
{
    int32_t half[2];
    uint32_t mode = 0;
    int32_t dx, dy;
    /*
     * Sim_1008e211 leaves the slope unwritten when dx is about 0; the original then passes an unset local,
     * which Sim_1008e489 does not read in that case (modes 4 to 7 do not use it).
     */
    float slope = 0.0f;

    half[0] = (int32_t)((uint32_t)rect[3] - (uint32_t)rect[1] + 1u) >> 1;
    half[1] = (int32_t)((uint32_t)rect[4] - (uint32_t)rect[2] + 1u) >> 1;
    dx = (int32_t)((uint32_t)half[0] - (uint32_t)pt[0]);
    if (dx >= 0)
        mode |= 1;
    dy = (int32_t)((uint32_t)half[1] - (uint32_t)pt[1]);
    if (dy >= 0)
        mode |= 2;
    mode = (mode & ~4u) | (((uint32_t)Sim_1008e211((float)dx, (float)dy, &slope) & 1u) << 2);
    Sim_1008e489(half, mode, slope, out);
    return out;
}
