#include "w3r.h"
#include "../util/w3r.h"

// FUNCTION: HGSIM 0x10030228
void __cdecl Sim_10030228(uint8_t *obj, uint8_t *part, float *cx, float *cy, float *cz, uint32_t *minx, uint32_t *maxx,
                          uint32_t *miny, uint32_t *maxy, uint32_t *minz, uint32_t *maxz)
{
    uint32_t mn[3] = {0x7f7fc99eu, 0x7f7fc99eu, 0x7f7fc99eu};
    uint32_t mx[3] = {0xff7fc99eu, 0xff7fc99eu, 0xff7fc99eu};
    int i, k;

    if (obj == NULL || part == NULL || cx == NULL || cy == NULL || cz == NULL || minx == NULL || maxx == NULL ||
        miny == NULL || maxy == NULL || minz == NULL || maxz == NULL)
        return;
    for (i = 0; i < (int)U16(part, 2); i++) {
        uint8_t *vertex = obj + 0x18 + (uint32_t)U8(part, U32(part, 4) + (uint32_t)i) * 0x58u;

        for (k = 0; k < 3; k++) {
            float value = F32(vertex, 0xc + k * 4);

            if (!(value >= F32(&mn[k], 0)))
                mn[k] = U32(vertex, 0xc + k * 4);
            if (value > F32(&mx[k], 0))
                mx[k] = U32(vertex, 0xc + k * 4);
        }
    }
    *cx = (float)(((double)F32(&mn[0], 0) + (double)F32(&mx[0], 0)) * (double)GF32(0x101112dc));
    *cy = (float)(((double)F32(&mn[1], 0) + (double)F32(&mx[1], 0)) * (double)GF32(0x101112dc));
    *cz = (float)(((double)F32(&mn[2], 0) + (double)F32(&mx[2], 0)) * (double)GF32(0x101112dc));
    *minx = mn[0];
    *miny = mn[1];
    *minz = mn[2];
    *maxx = mx[0];
    *maxy = mx[1];
    *maxz = mx[2];
}

// FUNCTION: HGSIM 0x10033744
int __cdecl Sim_10033744(uint8_t *obj)
{
    float sum[3] = {0.0f, 0.0f, 0.0f};
    float scale;
    uint8_t *block;
    int32_t count, i;

    if (obj == NULL || U32(obj, 0x18) == 0)
        return 0;
    block = (uint8_t *)(uintptr_t)U32(U32(obj, 0x18), 0x30);
    if (block == NULL)
        return 0;
    count = S16(block, 4);
    for (i = 0; i < count; i++) {
        uint8_t *vertex = block + (uint32_t)i * 0x58u + 0x18;

        if (vertex == NULL)
            return 0;
        sum[0] = (float)((double)sum[0] + (double)F32(vertex, 0xc));
        sum[1] = (float)((double)sum[1] + (double)F32(vertex, 0x10));
        sum[2] = (float)((double)sum[2] + (double)F32(vertex, 0x14));
    }
    scale = (float)((double)GF32(0x10111320) / (double)count);
    F32(obj, 0x20) = (float)((double)sum[0] * (double)scale);
    F32(obj, 0x24) = (float)((double)sum[1] * (double)scale);
    F32(obj, 0x28) = (float)((double)sum[2] * (double)scale);
    F32(obj, 0x20) = (float)((double)F32(obj, 0x20) + (double)F32(obj, 4));
    F32(obj, 0x24) = (float)((double)F32(obj, 0x24) + (double)F32(obj, 8));
    F32(obj, 0x28) = (float)((double)F32(obj, 0x28) + (double)F32(obj, 0xc));
    return 1;
}

// FUNCTION: HGSIM 0x10033e6f
int __cdecl Sim_10033e6f(const uint8_t *a, const uint8_t *b)
{
    float x = F32(a, 4), y = F32(b, 4);

    if (!(x >= y))
        return -1;
    return x > y ? 1 : 0;
}
