#include "w4v.h"
#include "../util/access.h"
#include "../win/w4v.h"
#include "../unsorted_100f/w2e.h"

// FUNCTION: HGSIM 0x10048112
uint32_t __cdecl Sim_10048112(uint32_t a, uint32_t b, int32_t kind, const int16_t *d)
{
    uint8_t *record;
    uint32_t index;

    if (GS32(0x1015f1c8) >= 7)
        return 0xffff;
    if (kind != 1)
        return 0xffff;
    if (d[0] < 1)
        return (uint32_t)(uintptr_t)d | 0xffffu;
    record = Heap_AllocRaw(0x1c);
    if (record == 0)
        return 0xffff;
    Mem_Fill(record, 0, 0x1c);
    G32(0x109b94e0 + G32(0x1015f1c8) * 4u) = (uint32_t)(uintptr_t)record;
    U32(record, 0) = 0;
    U32(record, 0xc) = 0;
    U32(record, 0x10) = a;
    U32(record, 8) = b;
    U32(record, 0x14) = (uint32_t)(int32_t)d[0];
    index = G32(0x1015f1c8);
    G32(0x1015f1c8) = index + 1;
    return index;
}
