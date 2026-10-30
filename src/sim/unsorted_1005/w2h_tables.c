#include <string.h>
#include "w2h.h"
#include "../unsorted_1004/fns.h"
#include "../simmem.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x10052530
void __cdecl Sim_10052530(void)
{
    uint32_t i;

    for (i = 0; i < 0x10; i++) {
        G32(0x109b7d40 + i * 0x6c) = 0;
        G32(0x109b7d58 + i * 0x6c) = 0;
        G32(0x109b7d48 + i * 0x6c) = 0;
        memcpy(GBYTES(0x109b8420 + i * 0x74), GBYTES(0x109b68e0), 0x74);
    }
}

// FUNCTION: HGSIM 0x10052632
uint32_t __cdecl Sim_10052632(int32_t n)
{
    if (n >= 0x10)
        return 0;
    return G32(0x109b7d58 + (uint32_t)n * 0x6c);
}

// FUNCTION: HGSIM 0x10052c60
int __cdecl Sim_10052c60(int a, int b)
{
    int first = Sim_10052be7(a);

    return first == Sim_10052be7(b);
}

// FUNCTION: HGSIM 0x1005436d
int __cdecl Sim_1005436d(int32_t n, const uint32_t *entries)
{
    int32_t i;

    for (i = 0; i < n; i++) {
        const uint8_t *e = (const uint8_t *)entries + (uint32_t)i * 12;
        uint32_t bits = U32(U32(e, 0), 0) & U32(e, 4);

        if (U32(e, 8) == 1) {
            if (bits != 0)
                return 0;
        } else if (bits == 0) {
            return 0;
        }
    }
    return 1;
}

// FUNCTION: HGSIM 0x10057aa9
int __cdecl Sim_10057aa9(uint16_t *out)
{
    if (G32(0x10166868) == G32(0x1016686c)) {
        *out = 0;
    } else {
        *out = U16(GBYTES(0x10165ee0), G32(0x10166868) * 2);
        G32(0x10166868) = G32(0x10166868) + 1;
        if (G32(0x10166868) == 0x40)
            G32(0x10166868) = 0;
    }
    return 0;
}

// FUNCTION: HGSIM 0x100585e1
void __cdecl Sim_100585e1(int32_t index, uint32_t mask, const uint32_t *src)
{
    uint8_t *rec;
    uint8_t *d;

    if (src == 0 || index < 0 || index >= GS32(0x109b0eac))
        return;
    rec = GBYTES(0x109b16e0 + (uint32_t)index * 0x4c);
    d = rec + 0x10;
    if (mask & 1) {
        U32(d, 4) = src[1];
        U32(d, 8) = src[2];
        U32(d, 0xc) = src[3];
    }
    if (mask & 2)
        U32(d, 0) = src[0];
    if (mask & 4)
        U32(d, 0x2c) = U32(src, 0x2c);
    if (mask & 8)
        U32(d, 0x34) = U32(src, 0x34);
    else
        U32(d, 0x34) = 0xffffffffu;
    if (mask & 0x10)
        U32(d, 0x30) = U32(src, 0x30);
    if (mask & 0x20) {
        U32(d, 0x1c) = U32(src, 0x1c);
        U32(d, 0x20) = U32(src, 0x20);
        U32(d, 0x24) = U32(src, 0x24);
    }
    if (mask & 0x40)
        U32(d, 0x28) = U32(src, 0x28);
    U32(rec, 8) = (mask & 0x80) ? 1 : 0;
}
