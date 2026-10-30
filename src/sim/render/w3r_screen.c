#include "w3r.h"
#include "w2n.h"
#include "../util/w3r.h"

// FUNCTION: HGSIM 0x10052e60
int __cdecl Sim_10052e60(int x, int y, const uint8_t *p)
{
    float fx = (float)(int32_t)((uint32_t)x - G32(0x10185bd4));
    float fy = (float)(int32_t)(G32(0x10185bf4) - (uint32_t)y);
    float a = F32(p, 0x58), b = F32(p, 0x64), c = F32(p, 0x70);
    double sum = (double)fx * (double)a / (double)GF32(0x10185be4) + (double)fy * (double)b / (double)GF32(0x10185be8);

    return sum > -(double)c;
}

// FUNCTION: HGSIM 0x10052f3a
int __cdecl Sim_10052f3a(int y, const uint8_t *p)
{
    int32_t fy = (int32_t)(G32(0x10185bf4) - (uint32_t)y);
    double v = (double)fy * (double)F32(p, 0x64) / (double)GF32(0x10185be8) + (double)F32(p, 0x70);

    v = -v * (double)GF32(0x10185be4) / (double)F32(p, 0x58);
    return (int)((uint32_t)Render_Ftol(v) + G32(0x10185bd4));
}

// FUNCTION: HGSIM 0x10090099
int __cdecl Sim_10090099(float *v)
{
    int result = 0;
    float dx = (float)((double)v[0] - (double)GF32(0x10185c48));
    float dy = (float)((double)v[1] - (double)GF32(0x10185c44));
    float dz = (float)((double)v[2] - (double)GF32(0x10185c4c));
    float rx = (float)((double)dx * (double)GF32(0x10185bfc) + (double)dy * (double)GF32(0x10185c00) + (double)dz * (double)GF32(0x10185c04));
    float ry = (float)((double)dx * (double)GF32(0x10185c08) + (double)dy * (double)GF32(0x10185c0c) + (double)dz * (double)GF32(0x10185c10));
    float rz = (float)((double)dx * (double)GF32(0x10185c14) + (double)dy * (double)GF32(0x10185c18) + (double)dz * (double)GF32(0x10185c1c));
    uint32_t sx, sy;

    sx = (uint32_t)Render_Ftol((double)rx / (double)GF32(0x1036f1d8)) + G32(0x10185bd4);
    v[0] = (float)(int32_t)sx;
    sy = G32(0x10185be0) - G32(0x10185bf0);
    sy -= (uint32_t)Render_Ftol((double)ry / (double)GF32(0x1036f1d8)) + G32(0x10185bf4);
    v[1] = (float)(int32_t)sy;
    F32(v, 8) = rz;
    if ((U32(&rz, 0) & 0x80000000u) == 0)
        result = (double)GS32(0x10185bd0) <= (double)v[0] && (double)v[0] <= (double)GS32(0x10185bec) &&
                 (double)GS32(0x10185bf0) <= (double)v[1] && (double)v[1] <= (double)GS32(0x10185be0);
    return result;
}
