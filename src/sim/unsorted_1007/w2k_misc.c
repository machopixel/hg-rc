#include "w2k.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x10076689
uint32_t __cdecl Sim_10076689(void)
{
    uint32_t node = G32(0x1015DC14);
    uint32_t result = 0;

    while (node != 0) {
        if (S32(node, 4) == 1) {
            result = U32(node, 0xC);
            break;
        }
        node = U32(node, 0x14);
    }
    return result;
}

// FUNCTION: HGSIM 0x10079ecd
int __cdecl Sim_10079ecd(const uint8_t *id, int n)
{
    uint32_t index = id[0];
    uint32_t table = (uint32_t)id[1] << 8;
    uint32_t record;
    int result = 0;

    switch (table) {
    case 0x100:
        record = 0x109A8640u + index * 0x54u;
        if ((S16(GBYTES(record), 0x24) & 0x20) != 0 && ((1u << (n & 31)) & (uint32_t)(int32_t)S16(GBYTES(record), 0x26)) != 0)
            result = 1;
        break;
    case 0x200:
        record = PTRTAB(index);
        if ((S16(record, 0x1C) & 0x20) != 0 && ((1u << (n & 31)) & (uint32_t)(int32_t)S16(record, 0x1E)) != 0)
            result = 1;
        break;
    case 0x400:
        record = 0x109E9BA0u + index * 0x50u;
        if ((S16(GBYTES(record), 0) & 0x20) != 0 && ((1u << (n & 31)) & (uint32_t)(int32_t)S16(GBYTES(record), 6)) != 0)
            result = 1;
        break;
    }
    return result;
}
