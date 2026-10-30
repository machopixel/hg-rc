#include "w3t.h"
#include "../util/w3t.h"

#define EMPTY_MIN 0x7f7fc99eu
#define EMPTY_MAX 0xff7fc99eu

/* min = v when v is below it or unordered; max = v when v is above it. */
static void Widen(uint32_t *lo, uint32_t *hi, const uint8_t *vertex, uint32_t off)
{
    float v = FL32(vertex, off);

    if (!(v >= BitsToFloat(*lo)))
        *lo = U32(vertex, off);
    if (v > BitsToFloat(*hi))
        *hi = U32(vertex, off);
}

// FUNCTION: HGSIM 0x100303ea
void __cdecl Sim_100303ea(const void *list, float *cx, float *cy, float *cz, uint32_t *minX, uint32_t *maxX,
                          uint32_t *minY, uint32_t *maxY, uint32_t *minZ, uint32_t *maxZ)
{
    uint32_t loX = EMPTY_MIN, hiX = EMPTY_MAX, loY = EMPTY_MIN, hiY = EMPTY_MAX, loZ = EMPTY_MIN, hiZ = EMPTY_MAX;
    int i, count;

    if (!list || !cx || !cy || !cz || !minX || !maxX || !minY || !maxY || !minZ || !maxZ)
        return;
    count = S16(list, 4);
    for (i = 0; i < count; i++) {
        const uint8_t *vertex = BYTES(list) + (uint32_t)i * 0x58u + 0x18u;

        Widen(&loX, &hiX, vertex, 0xc);
        Widen(&loY, &hiY, vertex, 0x10);
        Widen(&loZ, &hiZ, vertex, 0x14);
    }
    *cx = (float)(((double)BitsToFloat(loX) + (double)BitsToFloat(hiX)) * 0.5);
    *cy = (float)(((double)BitsToFloat(loY) + (double)BitsToFloat(hiY)) * 0.5);
    *cz = (float)(((double)BitsToFloat(loZ) + (double)BitsToFloat(hiZ)) * 0.5);
    *minX = loX;
    *minY = loY;
    *minZ = loZ;
    *maxX = hiX;
    *maxY = hiY;
    *maxZ = hiZ;
}

// FUNCTION: HGSIM 0x10030672
void __cdecl Sim_10030672(const void *obj)
{
    uint32_t block;
    int i, j;

    if (obj == 0 || U32(obj, 0x98) == 0)
        return;
    block = U32(obj, 0x98);
    Sim_100303ea(BYTES(U32(obj, 0x2c)), (float *)(BYTES(block) + 0x18), (float *)(BYTES(block) + 0x1c),
                 (float *)(BYTES(block) + 0x20), (uint32_t *)BYTES(block), (uint32_t *)(BYTES(block) + 4),
                 (uint32_t *)(BYTES(block) + 8), (uint32_t *)(BYTES(block) + 0xc),
                 (uint32_t *)(BYTES(block) + 0x10), (uint32_t *)(BYTES(block) + 0x14));
    for (i = 0; i < 7; i++) {
        for (j = 0; j < 7; j++) {
            uint8_t *cell = BYTES(block) + (uint32_t)i * 0xe0u + 0x24u + (uint32_t)j * 0x20u;

            U32(cell, 0x10) = EMPTY_MIN;
            U32(cell, 8) = EMPTY_MIN;
            U32(cell, 0) = EMPTY_MIN;
            U32(cell, 0x14) = EMPTY_MAX;
            U32(cell, 0xc) = EMPTY_MAX;
            U32(cell, 4) = EMPTY_MAX;
            U32(cell, 0x1c) = 0;
            U32(cell, 0x18) = 0;
        }
    }
}
