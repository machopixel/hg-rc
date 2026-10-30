#include "w3t.h"
#include "../util/w3t.h"
#include "../render/w2n.h"
#include "../unsorted_100f/w2e.h"

// FUNCTION: HGSIM 0x100dacf9
int __cdecl Sim_100dacf9(const void *packet)
{
    int32_t count = (int32_t)((U32(packet, 4) - 0x48u) / 0x1cu);
    int32_t slot, j;
    uint32_t base;

    if (GS32(0x1015d17c) >= 0x40 || count > 0x40)
        return 0;
    slot = GS32(0x1015d17c);
    G32(0x1015d17c) = (uint32_t)slot + 1u;
    base = 0x10a00040u + (uint32_t)slot * 0x744u;
    S32(GBYTES(base), 0x40) = count;
    Str_CopyN((char *)GBYTES(base), (const char *)BYTES(packet) + 8, 0x40);
    for (j = 0; j < count; j++) {
        const uint8_t *src = BYTES(packet) + (uint32_t)j * 0x1cu + 0x48u;
        uint8_t *dst = GBYTES(base) + 0x44 + (uint32_t)j * 0x1cu;

        FL32(dst, 0) = (float)S32(src, 0);
        FL32(dst, 4) = (float)S32(src, 4);
        FL32(dst, 8) = (float)S32(src, 8);
        FL32(dst, 0xc) = (float)S32(src, 0xc);
        FL32(dst, 0x10) = (float)S32(src, 0x10);
        FL32(dst, 0x14) = (float)S32(src, 0x14);
        U32(dst, 0x18) = U32(src, 0x18);
    }
    return 1;
}

static float Abs(float v)
{
    return v > 0.0f ? v : -v;
}

// FUNCTION: HGSIM 0x100db4c2
void __cdecl Plane_Quantize(float a, float b, float c, float d, int32_t *outA, int32_t *outB, int32_t *outC,
                            int32_t *outD)
{
    float sum = Abs(a);
    double total;
    float scale;

    sum = (float)((double)sum + (double)Abs(b));
    sum = (float)((double)sum + (double)Abs(c));
    total = (double)sum + (double)Abs(d);
    sum = (float)total;
    if (total > 0.0001) {
        scale = (float)((double)sum / 536870912.0);
        *outA = Render_Ftol((double)a / (double)scale);
        *outB = Render_Ftol((double)b / (double)scale);
        *outC = Render_Ftol((double)c / (double)scale);
        *outD = Render_Ftol((double)d / (double)scale);
    }
}

// FUNCTION: HGSIM 0x100db659
void __cdecl Plane_FromPoints(float x0, float y0, float z0, float x1, float y1, float z1, float x2, float y2,
                              float z2, int32_t *outA, int32_t *outB, int32_t *outC, int32_t *outD)
{
    float v1x = (float)((double)x1 - (double)x0);
    float v1y = (float)((double)y1 - (double)y0);
    float v1z = (float)((double)z1 - (double)z0);
    float v2x = (float)((double)x2 - (double)x0);
    float v2y = (float)((double)y2 - (double)y0);
    float v2z = (float)((double)z2 - (double)y0);
    float n0 = (float)((double)v1y * (double)v2z - (double)v2y * (double)v1z);
    float n1 = (float)((double)v1x * (double)v2z - (double)v2x * (double)v1z);
    float n2 = (float)((double)v1x * (double)v2y - (double)v2x * (double)v1y);
    float d = (float)-(((double)n0 * (double)x0 + (double)n1 * (double)y0) + (double)n2 * (double)z0);

    Plane_Quantize(n0, n1, n2, d, outA, outB, outC, outD);
}

// FUNCTION: HGSIM 0x100db70f
void __cdecl Plane_FromPointNormal(float x, float y, float z, float a, float b, float c, int32_t *outA,
                                   int32_t *outB, int32_t *outC, int32_t *outD)
{
    float n1 = -b;
    float d = (float)-(((double)a * (double)x + (double)n1 * (double)y) + (double)c * (double)z);

    Plane_Quantize(a, n1, c, d, outA, outB, outC, outD);
}
