#include "w3q.h"
#include "../render/w2n.h"
#include "../util/w3q.h"

// FUNCTION: HGSIM 0x10052edc
int __cdecl Sim_10052edc(int x, const void *p)
{
    float a = F32A(p, 0x58);
    float b = F32A(p, 0x64);
    float c = F32A(p, 0x70);
    int dx = (int)((uint32_t)x - G32(0x10185bd4));
    double d = (double)dx * (double)a;

    d = d / (double)GF32(0x10185be4);
    d = -(d + (double)c);
    d = d * (double)GF32(0x10185be8);
    d = d / (double)b;
    return (int)(G32(0x10185bf4) - (uint32_t)Render_Ftol(d));
}

// FUNCTION: HGSIM 0x1005897b
int __cdecl Sim_1005897b(const void *poly, const void *verts, const void *box)
{
    float cx = F32A(box, 0x14);
    float cy = F32A(box, 0x18);
    float cz = F32A(box, 0x1c);
    float r = F32A(box, 0x3c);
    float maxX = (float)((double)cx + (double)r);
    float maxY = (float)((double)cy + (double)r);
    float maxZ = (float)((double)cz + (double)r);
    float minX = (float)((double)cx - (double)r);
    float minY = (float)((double)cy - (double)r);
    float minZ = (float)((double)cz - (double)r);
    int found = 0;
    int i;
    int count;

    if (poly == 0 || verts == 0)
        return 0;
    count = U16(poly, 2);
    for (i = 0; i < count && !found; i++) {
        const uint8_t *v = BYTES(verts) + (uint32_t)BYTES(poly)[U32(poly, 4) + (uint32_t)i] * 0x58;
        float x = F32A(v, 0xc);
        float y = F32A(v, 0x10);
        float z = F32A(v, 0x14);

        if (X87_ABOVE(x, maxX) || X87_BELOW(x, minX))
            continue;
        if (X87_ABOVE(y, maxY) || X87_BELOW(y, minY))
            continue;
        if (X87_ABOVE(z, maxZ) || X87_BELOW(z, minZ))
            continue;
        found = 1;
    }
    return found;
}

// FUNCTION: HGSIM 0x1005fc4f
void __cdecl Sim_1005fc4f(const float *m, float *a, float *b, float *c)
{
    float x = *a;
    float y = *b;
    float z = *c;

    *a = (float)((double)m[0] * (double)x + (double)m[1] * (double)y + (double)m[2] * (double)z);
    *b = (float)((double)m[3] * (double)x + (double)m[4] * (double)y + (double)m[5] * (double)z);
    *c = (float)((double)m[6] * (double)x + (double)m[7] * (double)y + (double)m[8] * (double)z);
}
