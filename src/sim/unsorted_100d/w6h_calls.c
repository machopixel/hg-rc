#include "w6h.h"
#include "../fx/w32.h"
#include "fns.h"
#include "w2d.h"
#include "../unsorted_1005/w3q.h"
#include "../unsorted_1005/w5b.h"
#include "../unsorted_1006/w5y.h"
#include "../unsorted_1008/w3o.h"
#include "../unsorted_1008/w4v.h"
#include "../unsorted_1009/w2g.h"
#include "../unsorted_100a/w3t.h"
#include "../unsorted_100a/w5b.h"
#include "../unsorted_100a/w5x.h"
#include "../unsorted_100a/w5y.h"
#include "../unsorted_100a/w5z.h"
#include "../unsorted_100f/w2e.h"
#include "../util/util.h"
#include "../util/w3t.h"
#include "../win/w4a.h"

#define W6H_ADDR(a) ((uint32_t)(uintptr_t)SIM_PTR(uint8_t, a))
#define W6H_PTR(v) ((void *)(uintptr_t)(v))

/* The bits of a float (the originals store float results and pass them on as dwords). */
static uint32_t W6h_Bits(float f)
{
    union {
        float f;
        uint32_t u;
    } v;

    v.f = f;
    return v.u;
}

// FUNCTION: HGSIM 0x100d36f5
void __cdecl Sim_100d36f5(uint32_t first, int32_t count)
{
    uint8_t *data;

    if (GS32(0x1017e65c) > 0 || G32(0x1017e660) != 0)
        return;
    G32(0x1017e660) = 1;
    G32(0x1017e664) = W6H_ADDR(0x10185620) + (G32(0x10152420) << 4);
    data = W6H_CALL_100ba7d5(G32(0x1017e664), G32(0x1014ced4), 0);
    if (data == 0)
        return;
    Sim_1008c550(SIM_PTR(void, 0x1017e640), data, first, count);
    Mem_Move(W6H_PTR(G32(0x1017e644)), data, 0x300);
    *GBYTES(0x1017e648) = 0x80;
    U32(GBYTES(0x1017e649), 0) = 0x10;
    G32(0x1017e660) = 1;
}

// FUNCTION: HGSIM 0x100d62bd
void __cdecl Sim_100d62bd(void)
{
    uint32_t i;

    for (i = 0; i < 0x2e; i++) {
        uint8_t *entry = GBYTES(0x10145508 + i * 0xd6);

        if (entry[0xb2] != 0)
            Sim_100a77a7((uint32_t)(uintptr_t)(entry + 0xb2), G32(0x1014ceb4));
    }
    for (i = 0; i < 0x4d; i++) {
        if (GS32(0x10147b88 + i * 0x3c) > 0)
            Sim_100a77a7(W6H_ADDR(0x10147bac + i * 0x3c), G32(0x1014ceb4));
    }
}

// FUNCTION: HGSIM 0x100d6f92
void __cdecl Sim_100d6f92(void *obj, float *a, float *b, float *c)
{
    float matrix[12];
    float p, q, r;
    uint32_t s;

    U32(b, 0) = 0;
    U32(a, 0) = 0;
    U32(c, 0) = 0x3f800000;
    Sim_100a9ca3(W6H_PTR(U32(obj, 0x4c)), (uint32_t)(uintptr_t)&p, (uint32_t)(uintptr_t)&q, (uint32_t)(uintptr_t)&r);
    s = U32(obj, 0x28);
    if ((U16(s, 0x562) & 4) != 0 || (U16(s, 0x562) & 0x400) != 0) {
        p = (float)((double)p + (double)FL32(s, 0x1c));
        if (U32(U32(s, 0), 0) == 1 || U32(U32(s, 0), 0) == 9)
            q = (float)((double)FL32(s, 0xc) / (double)GFL32(0x1012bd3c) + (double)q);
    } else {
        if (U32(U32(s, 0), 0) == 1 || U32(U32(s, 0), 0) == 9)
            q = (float)((double)FL32(s, 0xc) / (double)GFL32(0x1012bd3c) + (double)q);
        U32(&p, 0) = U32(s, 0x1c);
    }
    Sim_10060656((uint32_t)(uintptr_t)matrix, W6h_Bits(p), W6h_Bits(q), W6h_Bits(r), 0, 0, 0);
    Sim_1005fc4f(matrix, a, b, c);
}

// FUNCTION: HGSIM 0x100d7111
void __cdecl Sim_100d7111(void *obj, void *dst)
{
    uint32_t s = U32(obj, 0x28);
    uint8_t *e = BYTES(U32(s, 0x84) + U32(s, 0x4fc) * 0x128u);
    uint32_t node = U32(s, U32(e, 0x11c) * 4 + 0x498);
    uint32_t x, y, z;

    Sim_100d709d(obj, &x, &y, &z, e);
    W6H_CALL_100a9e1a(dst, Sim_100aa863(W6H_PTR(node)));
    Sim_100a9d47(dst, x, y, z);
}

/* Sim_100d8d0a and Sim_100d9306: the same code with another limit and state. */
static int W6h_MoveOrDrop(void *obj, uint32_t limit_at, uint32_t state)
{
    uint32_t rec = U32(obj, W32_REC_ARG);
    uint32_t px, py, pz;

    Sim_100a9cfb(W6H_PTR(U32(obj, W32_REC_OBJ)), &px, &py, &pz);
    if (BitsToFloat(py) > GFL32(limit_at)) {
        double scale = (double)GS32(0x1015dc80);
        float dx, dy, dz;
        int n;

        dx = (float)(((double)FL32(obj, W32_REC_VEL) + (double)FL32(rec, 0x14)) * scale / (double)GFL32(0x1012bd64));
        n = Sim_1009c929(10);
        dy = (float)((((double)FL32(obj, W32_REC_VEL + 4) - (double)n) + (double)FL32(rec, 0x18)) * scale
                     / (double)GFL32(0x1012bd64));
        dz = (float)(((double)FL32(obj, W32_REC_VEL + 8) + (double)FL32(rec, 0x1c)) * scale / (double)GFL32(0x1012bd64));
        Sim_100a9d7c(W6H_PTR(U32(obj, W32_REC_OBJ)), dx, dy, dz);
        Sim_100aa799(W6H_PTR(U32(obj, W32_REC_OBJ)));
        U32(obj, W32_REC_STATE) = state;
        return 1;
    }
    U16(obj, 0x10) &= 0xfffe;
    Sim_100aa351(W6H_PTR(U32(obj, W32_REC_OBJ)));
    SlotPool_Release(W6H_PTR(U32(rec, 0x20)), (int)U32(rec, 0));
    return 0;
}

// FUNCTION: HGSIM 0x100d8d0a
int __cdecl Sim_100d8d0a(void *obj)
{
    return W6h_MoveOrDrop(obj, 0x1012bd68, 0x5e);
}

// FUNCTION: HGSIM 0x100d9306
int __cdecl Sim_100d9306(void *obj)
{
    return W6h_MoveOrDrop(obj, 0x1012bd60, 0xdf);
}

// FUNCTION: HGSIM 0x100d9064
void __cdecl Sim_100d9064(float x, float y, float z, float vx, float vy, float vz, void *owner)
{
    uint32_t cam = G32(0x1014cae0);
    float dx = (float)((double)x - (double)FL32(cam, 0));
    float dy = (float)((double)y - (double)FL32(G32(0x1014cae0), 4));
    float dz = (float)((double)z - (double)FL32(G32(0x1014cae0), 8));
    uint8_t *slot;
    uint8_t *o;
    uint32_t node;
    float sx, sy, sz;
    int i;

    if (Sim_10084a94(dx, dy, dz) > (double)FL32(G32(0x1014cae0), 0x40))
        return;
    slot = SlotPool_Take(owner);
    if (slot == 0)
        return;
    o = Sim_10090c7d(W6H_ADDR(0x10001659), W6H_ADDR(0x1000320b), (uint32_t)(uintptr_t)slot, 0);
    if (o == 0) {
        SlotPool_Release(owner, (int)U32(slot, 0));
        return;
    }
    U32(slot, 4) = U32(o, 8);
    U32(slot, 0x1c) = 0;
    U32(slot, 0x18) = 0;
    U32(slot, 0x14) = 0;
    U16(o, 0x10) = 1;
    node = U32(U32(o, W32_REC_OBJ), 0x8c);
    FL32(node, 0x50) = (float)Sim_10084a94(vx, vy, vz);
    FL32(node, 0x44) = x;
    FL32(node, 0x48) = y;
    FL32(node, 0x4c) = z;
    U32(node, 0x38) = 0;
    U32(node, 0x3c) = 0;
    U32(node, 0x40) = 0;
    FL32(o, W32_REC_VEL) = vx;
    FL32(o, W32_REC_VEL + 4) = vy;
    FL32(o, W32_REC_VEL + 8) = vz;
    sx = (float)((double)vx / (double)GFL32(0x1012bd6c));
    sy = (float)((double)vy / (double)GFL32(0x1012bd6c));
    sz = (float)((double)vz / (double)GFL32(0x1012bd6c));
    dx = sx;
    dy = sy;
    dz = sz;
    Sim_100aa39f(W6H_PTR(U32(o, W32_REC_OBJ)));
    Sim_100a9d47(W6H_PTR(U32(o, W32_REC_OBJ)), W6h_Bits(x), W6h_Bits(y), W6h_Bits(z));
    Sim_100aa799(W6H_PTR(U32(o, W32_REC_OBJ)));
    U32(o, W32_REC_TIME) = G32(0x1015dc78);
    /* 1.2 (0x100be504): 16 points of 0x32 bytes in its particle layout (1.0: 32 of 0x2a) */
#ifdef W10W_MUTATE /* the checker's negative check only: 1.0's 32 points */
    for (i = 0; i < 0x20; i++) {
#else
    for (i = 0; i < W32_REC_NPT; i++) {
#endif
        uint8_t *part = o + i * W32_REC_PT + 0x12;

        U16(part, 0x18) = 1;
        dx = (float)((double)dx + (double)sx);
        dy = (float)((double)dy + (double)sy);
        dz = (float)((double)dz + (double)sz);
        FL32(part, 0) = (float)(((double)Sim_1009c929(10) + (double)dx) - (double)GFL32(0x1012bd70));
        FL32(part, 4) = (float)(((double)Sim_1009c929(0x32) + (double)dy) - (double)GFL32(0x1012bd74));
        FL32(part, 8) = (float)(((double)Sim_1009c929(10) + (double)dz) - (double)GFL32(0x1012bd70));
    }
}

// FUNCTION: HGSIM 0x100da098
int __cdecl Sim_100da098(const void *data)
{
    if (G32(0x1017e688) != 0)
        Heap_Free(W6H_PTR(G32(0x1017e688)));
    G32(0x1017e688) = 0;
    G32(0x1017f118) = 0;
    G32(0x1017e690) = 0;
    G32(0x1017e688) = (uint32_t)(uintptr_t)Heap_AllocZero(U32(data, 4));
    if (G32(0x1017e688) == 0) {
        W6H_CALL_100bc280(0x3f, 0);
        return 0;
    }
    Mem_Move(W6H_PTR(G32(0x1017e688)), data, U32(data, 4));
    G32(0x1017f118) = (U32(G32(0x1017e688), 4) - 8u) / 12u;
    return 1;
}

// FUNCTION: HGSIM 0x100dac0a
int __cdecl Sim_100dac0a(const void *data)
{
    uint32_t slot = U32(data, 8);
    u32a *blocks = SIM_PTR(u32a, 0x1017ee40);
    u32a *counts = SIM_PTR(u32a, 0x1017ee80);

    if (blocks[slot] != 0)
        Heap_Free(W6H_PTR(blocks[slot]));
    blocks[slot] = 0;
    counts[slot] = 0;
    blocks[slot] = (uint32_t)(uintptr_t)Heap_AllocZero(U32(data, 4));
    if (blocks[slot] == 0) {
        W6H_CALL_100bc280(0x40, 0);
        return 0;
    }
    Mem_Move(W6H_PTR(blocks[slot]), data, U32(data, 4));
    counts[slot] = (U32(blocks[slot], 4) - 8u) / 0x97u;
    W6H_CALL_10078f60(W6H_PTR(blocks[slot]));
    return 1;
}

// FUNCTION: HGSIM 0x100db0ee
void __cdecl Sim_100db0ee(const void *data)
{
    int32_t count = (int32_t)((U32(data, 4) - 8u) / 0x18u);
    int32_t i;

    for (i = 0; i < count; i++) {
        const uint8_t *entry = BYTES(data) + i * 0x18;
        uint32_t record = 0x109b7d4c + (uint32_t)i * 0x6c;

        G32(record) = U32(entry, 8);
        G32(record + 8) = U32(entry, 0xc);
        SIM_PTR(u32a, 0x109b8400)[U32(entry, 8)] = U32(entry, 0xc);
        if (i == GS32(0x1013bb40) && G32(0x1017e6b0) != 0)
            Sim_1005259e(i, (const char *)W6H_PTR(G32(0x1017e6b0)));
        else if (i != GS32(0x1013bb40) && G32(0x1017e6b4) != 0)
            Sim_1005259e(i, (const char *)W6H_PTR(G32(0x1017e6b4)));
        else
            Sim_1005259e(i, (const char *)(entry + 0x10));
        G32(record + 4) = 0;
    }
}
