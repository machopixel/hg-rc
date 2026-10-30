#include "w3r.h"
#include "../unsorted_100f/w2e.h"
#include "../util/w3r.h"

#define RECORD(index) SIM_PTR(uint8_t, 0x10180a00u + (uint32_t)(index) * 0x80cu)
/* A 32-bit int stored as a float, as fild + fstp did. */
#define INT_AS_FLOAT(dst, src, off, srcoff) F32(dst, off) = (float)S32(src, srcoff)

// FUNCTION: HGSIM 0x100d867b
void __cdecl Sim_100d867b(const uint8_t *src, int index)
{
    uint8_t *rec;
    uint32_t flags;
    int k;

    if (index < 0 || index >= 9)
        return;
    rec = RECORD(index);
    flags = U32(src, 4);
    if (flags & 1)
        U32(rec, 8) = U32(src, 8);
    else
        U32(rec, 8) = 0;
    if (flags & 2) {
        INT_AS_FLOAT(rec, src, 0xc, 0xc);
        INT_AS_FLOAT(rec, src, 0x10, 0x10);
        INT_AS_FLOAT(rec, src, 0x14, 0x14);
    }
    if (flags & 4)
        Mem_Move(rec + 0x18, src + 0x18, 0x80);
    if (flags & 8)
        U32(rec, 0x98) = U32(src, 0x98);
    else
        U32(rec, 0x98) = 1;
    if (flags & 0x10)
        U32(rec, 0x9c) = U32(src, 0x9c);
    else
        U32(rec, 0x9c) = 1;
    if (flags & 0x20)
        U32(rec, 0xa8) = U32(src, 0xa8);
    else
        U32(rec, 0xa8) = 0;
    if (flags & 0x40) {
        INT_AS_FLOAT(rec, src, 0xac, 0xac);
        INT_AS_FLOAT(rec, src, 0xb0, 0xb0);
        INT_AS_FLOAT(rec, src, 0xb4, 0xb4);
    }
    if (flags & 0x80)
        INT_AS_FLOAT(rec, src, 0xb8, 0xb8);
    if (flags & 0x100) {
        U32(rec, 0xbc) = U32(src, 0xbc);
        for (k = 0xc0; k <= 0xd4; k += 4)
            INT_AS_FLOAT(rec, src, k, k);
    }
    if (flags & 0x200)
        for (k = 0xd8; k <= 0xec; k += 4)
            INT_AS_FLOAT(rec, src, k, k);
    if (flags & 0x400)
        U32(rec, 0xf0) = U32(src, 0xf0);
    else
        U32(rec, 0xf0) = 0;
}

// FUNCTION: HGSIM 0x100d8936
void __cdecl Sim_100d8936(const uint8_t *src)
{
    uint32_t flags = U32(src, 4);
    uint32_t defaultY;
    int i, k;

    if (U32(src, 0) != 2) {
        G32(0x1015245c) = 1;
        defaultY = 0xc61c4000u;
    } else {
        G32(0x1015245c) = 2;
        defaultY = 0xc59c4000u;
    }
    G32(0x1017e684) = 1;
    for (i = 0; i < 9; i++) {
        uint8_t *rec = RECORD(i);

        U32(rec, 0) = G32(0x1015245c);
        if (flags & 1)
            Mem_Move(rec + 0x18, src + 8, 0x80);
        if (flags & 2) {
            U32(rec, 0xbc) = U32(src, 0x88);
            for (k = 0; k < 6; k++)
                INT_AS_FLOAT(rec, src, 0xc0 + k * 4, 0x8c + k * 4);
        } else {
            for (k = 0xbc; k <= 0xd4; k += 4)
                U32(rec, k) = 0;
        }
        if (flags & 4) {
            INT_AS_FLOAT(rec, src, 0xb8, 0xb0);
            F32(rec, 0xf8) = (float)((double)F32(rec, 0xb8) / (double)GF32(0x1012bd64));
        } else {
            U32(rec, 0xb8) = 0x41600000u;
            F32(rec, 0xf8) = (float)((double)F32(rec, 0xb8) / (double)GF32(0x1012bd64));
        }
        if (flags & 8) {
            INT_AS_FLOAT(rec, src, 0xac, 0xa4);
            INT_AS_FLOAT(rec, src, 0xb0, 0xa8);
            INT_AS_FLOAT(rec, src, 0xb4, 0xac);
        } else {
            U32(rec, 0xac) = 0;
            U32(rec, 0xb0) = defaultY;
            U32(rec, 0xb4) = 0;
        }
    }
    for (k = 0; k < 9; k++)
        if (flags & (0x10000u << k))
            Sim_100d867b(src + 0xb4 + (uint32_t)k * 0xf4u, k);
}
