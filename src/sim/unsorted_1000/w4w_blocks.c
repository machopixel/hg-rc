#include "w4w.h"
#include "../win/w4w_api.h"
#include "../util/access.h"
#include "../util/util.h"
#include "../unsorted_100f/w2e.h"

static float key_float(uint32_t bits)
{
    union {
        uint32_t u;
        float f;
    } v;

    v.u = bits;
    return v.f;
}

// FUNCTION: HGSIM 0x10008cbc
void *__cdecl Sim_10008cbc(void *owner, uint32_t key, int n, int m, uint32_t extra, uint32_t *end)
{
    uint32_t headerSize, tableSize, total;
    uint8_t *block;
    uint8_t *cur;

    if (owner == 0)
        return 0;
    if (m == 0)
        m = 1;
    headerSize = (uint32_t)n * 0x58u;
    tableSize = (uint32_t)m * 0x28u;
    total = headerSize + tableSize + 0x18u + extra;
    block = (uint8_t *)SIM_API(W4wHeapAllocFn, W4W_IAT_HEAPALLOC)(W4W_GAME_HEAP, 1, total);
    if (block == 0)
        return 0;
    Mem_Fill(block, 0, total);
    U32(block, 0) = key;
    U16(block, 4) = 0;
    U16(block, 6) = 0;
    U32(block, 0x10) = 0;
    U16(block, 0x14) = 0;
    U32(block, 8) = (headerSize + 0x18u) & 0xffffu;
    *end = (uint32_t)(uintptr_t)block + U32(block, 8) + tableSize;
    Sim_10008c69(owner, (uint32_t)(uintptr_t)block);
    if (U32(owner, 0x2c) == 0) {
        U32(owner, 0x2c) = (uint32_t)(uintptr_t)block;
        U32(block, 0xc) = 0;
        return block;
    }
    cur = BYTES(U32(owner, 0x2c));
    if (key_float(U32(cur, 0)) > key_float(key)) {
        U32(block, 0xc) = (uint32_t)(uintptr_t)cur;
        U32(owner, 0x2c) = (uint32_t)(uintptr_t)block;
        return block;
    }
    while (U32(cur, 0xc) != 0) {
        if (key_float(U32(BYTES(U32(cur, 0xc)), 0)) > key_float(key))
            break;
        cur = BYTES(U32(cur, 0xc));
    }
    U32(block, 0xc) = U32(cur, 0xc);
    U32(cur, 0xc) = (uint32_t)(uintptr_t)block;
    return block;
}

// FUNCTION: HGSIM 0x10008f1c
void *__cdecl Sim_10008f1c(int a, int b, uint32_t c, uint32_t *d)
{
    uint8_t *rec = (uint8_t *)SIM_API(W4wHeapAllocFn, W4W_IAT_HEAPALLOC)(W4W_GAME_HEAP, 1, 0xb4);

    if (rec == 0)
        return 0;
    U16(rec, 0) = (uint16_t)(G32(0x1015d09c) | 0x8000u);
    U32(rec, 0x28) = 0;
    U32(rec, 0x2c) = 0;
    U32(rec, 0x30) = 0;
    if (Sim_10008cbc(rec, 0, a, b, c, d) == 0) {
        SIM_API(W4wHeapFreeFn, W4W_IAT_HEAPFREE)(W4W_GAME_HEAP, 1, rec);
        return 0;
    }
    U16(rec, 2) = 0;
    U16(rec, 0x26) = 0;
    U16(rec, 0x24) = 0;
    U32(rec, 0x4c) = 0;
    U32(rec, 0x48) = 0;
    U32(rec, 0x44) = 0;
    U32(rec, 0x40) = 0;
    U32(rec, 0x3c) = 0;
    U32(rec, 0x38) = 0;
    U32(rec, 0x68) = 0;
    U32(rec, 0x64) = 0;
    U32(rec, 0x60) = 0;
    U32(rec, 0x5c) = 0;
    U32(rec, 0x58) = 0;
    U32(rec, 0x54) = 0;
    U32(rec, 0x84) = 0;
    U32(rec, 0x80) = 0;
    U32(rec, 0x7c) = 0;
    U32(rec, 0x78) = 0;
    U32(rec, 0x74) = 0;
    U32(rec, 0x70) = 0;
    U32(rec, 0x50) = 0;
    U32(rec, 0x6c) = 0;
    U32(rec, 0x88) = 0;
    U32(rec, 0x98) = 0;
    U32(rec, 8) = 0;
    U32(rec, 4) = 0;
    U32(rec, 0x10) = 0;
    U32(rec, 0xc) = 0;
    U32(rec, 0x18) = 0;
    U32(rec, 0x14) = 0;
    U32(rec, 0x20) = 0;
    U32(rec, 0x1c) = 0;
    U32(rec, 0x94) = 0;
    U32(rec, 0x34) = 4;
    U32(rec, 0x9c) = 0;
    U32(rec, 0xb0) = 0;
    return rec;
}
