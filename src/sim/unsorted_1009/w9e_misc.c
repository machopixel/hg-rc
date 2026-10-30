#include "w9e.h"
#include "../unsorted_1004/fns.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x1009b516
uint32_t __cdecl Sim_1009b516(const uint8_t *obj, int arg, uint32_t fallback)
{
    const uint8_t *world = BYTES(SIM_PTR(u32a, 0x1014c4c8)[G32(0x1015d1ac)]);
    const uint8_t *table, *rec;
    uint32_t result, kind;
    int idx;

    if (world == 0 || obj == 0 || arg == 0)
        return 0;
    kind = U16(obj, 0) & 0xf000u;
    if (kind == 0x8000 || kind == 0x9000)
        return kind;
    table = BYTES(U32(world, 0x74));
    result = 0;
    if (G32(0x1014cb28) != 0)
        result |= 0xf0;
    rec = BYTES(U32(obj, 0x24));
    switch (U16(rec, 2) & 0xf00u) {
    case 0x100:
        idx = Sim_10052be7(U16(rec, 0x24));
        return result | U32(table, (uint32_t)idx * 4);
    case 0x200:
        idx = Sim_10052c02(U16(rec, 0x24));
        return result | U32(table, 0xc + (uint32_t)idx * 4);
    case 0x400:
        return result | U32(table, 0x18);
    case 0x800:
        return result | 1;
    default:
        break;
    }
    switch (U16(rec, 2) & 0xf0u) {
    case 0x10:
    case 0x20:
    case 0x60:
        result = fallback;
        break;
    case 0x40:
        result |= U32(table, 0x1c);
        break;
    case 0x50:
        result |= U32(table, 0x20);
        break;
    case 0x80:
        result |= U32(table, 0x24);
        break;
    default:
        result |= 1;
        break;
    }
    return result;
}
