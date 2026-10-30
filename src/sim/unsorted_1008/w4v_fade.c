#include "w4v.h"
#include "../util/access.h"
#include "../win/w4v.h"

typedef void(__cdecl *DriverBlockFn)(uint32_t block);

#define PTR(v) ((void *)(uintptr_t)(v))

// FUNCTION: HGSIM 0x1008c550
int __cdecl Sim_1008c550(void *record, const uint8_t *src, uint32_t first, int32_t count)
{
    uint8_t *block;
    uint8_t *out;
    int32_t i;

    block = Heap_AllocRaw((uint32_t)count * 3u);
    if (block == 0)
        return 0;
    out = block;
    for (i = 0; i < (int32_t)((uint32_t)count * 3u); i++)
        *out++ = src[i];
    U32(record, 0) = (uint32_t)(uintptr_t)src;
    U32(record, 4) = (uint32_t)(uintptr_t)block;
    U8(record, 8) = (uint8_t)first;
    U32(record, 9) = (uint32_t)count;
    return 1;
}

// FUNCTION: HGSIM 0x1008c6f5
int __cdecl Sim_1008c6f5(void *record, const uint8_t *from, const uint8_t *to, uint32_t first, int32_t count,
                         uint32_t steps)
{
    uint8_t *block;
    uint32_t dst, delta, accum;
    int32_t i;

    block = Heap_AllocRaw((uint32_t)count * 12u);
    if (block == 0)
        return -1;
    U32(record, 0) = (uint32_t)(uintptr_t)block;
    U32(record, 4) = (uint32_t)(uintptr_t)block + 0x300;
    U32(record, 8) = (uint32_t)(uintptr_t)block + 0x600;
    delta = U32(record, 4);
    dst = U32(record, 0);
    accum = U32(record, 8);
    for (i = (int32_t)(first * 3u); i < (int32_t)(first * 3u + (uint32_t)count * 3u); i++) {
        U8(dst, 0) = from[i];
        dst++;
        U8(delta, 0) = (uint8_t)(to[i] - from[i]);
        delta++;
        U16(accum, 0) = 0;
        accum += 2;
    }
    U32(record, 0xc) = steps;
    U32(record, 0x10) = 0;
    U8(record, 0x14) = (uint8_t)first;
    U32(record, 0x15) = (uint32_t)count;
    return 1;
}

// FUNCTION: HGSIM 0x1008c803
void __cdecl Sim_1008c803(void *record)
{
    uint32_t accum, delta, colour;
    int32_t i;

    if (S32(record, 0x10) >= S32(record, 0xc))
        return;
    accum = U32(record, 8);
    delta = U32(record, 4);
    colour = U32(record, 0) + U8(record, 0x14);
    for (i = 0; i < (int32_t)((uint32_t)S32(record, 0x15) * 3u); i++) {
        S16(accum, 0) = (int16_t)(S16(accum, 0) + (int8_t)U8(delta, 0));
        U8(colour, 0) = (uint8_t)(U8(colour, 0) + (uint8_t)(S16(accum, 0) / S32(record, 0xc)));
        S16(accum, 0) = (int16_t)(S16(accum, 0) % S32(record, 0xc));
        accum += 2;
        delta++;
        colour++;
    }
    (*(DriverBlockFn *)(PTR(G32(0x1015e688) + 0x18)))(U32(record, 0));
    U32(record, 0x10) = U32(record, 0x10) + 1;
    if (S32(record, 0x10) >= S32(record, 0xc) && U32(record, 0) != 0) {
        Heap_FreeRaw(PTR(U32(record, 0)));
        U32(record, 0) = 0;
        if (U32(record, 4) != 0)
            U32(record, 4) = 0;
        if (U32(record, 8) != 0)
            U32(record, 8) = 0;
    }
}
