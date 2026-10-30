#include "w3s.h"
#include "../render/w3s_fp.h"
#include "../unsorted_1001/w3s.h"
#include "../unsorted_1004/fns.h"

// FUNCTION: HGSIM 0x100526e3
int __cdecl Sim_100526e3(int index, int val)
{
    int result = 0;
    uint8_t *group = GBYTES(0x109b7d40) + (uint32_t)index * 0x6cu;

    if (val == -1) {
        S32(group, 0) = -1;
    } else if (val < GS32(0x1015dc64) && index < 0x10) {
        uint32_t rec = PTRTAB(val);
        uint8_t *first = GBYTES(0x109b8430) + (uint32_t)index * 0x74u;
        uint8_t *second = GBYTES(0x109b8450) + (uint32_t)index * 0x74u;
        float a = F32(first, (uint32_t)U32(rec, 0x10) * 4u);
        float b = F32(second, (uint32_t)U32(rec, 0x10) * 4u);
        uint16_t i;

        S32(group, 0) = val;
        for (i = 0; i < 8; i++) {
            F32(first, i * 4u) = (float)((double)F32(first, i * 4u) - (double)a);
            F32(second, i * 4u) = (float)((double)F32(second, i * 4u) - (double)b);
        }
        result = 1;
    }
    return result;
}

// FUNCTION: HGSIM 0x10052ad6
int __cdecl Sim_10052ad6(int index)
{
    uint8_t *group = GBYTES(0x109b7d40) + (uint32_t)index * 0x6cu;
    int16_t current = (int16_t)Sim_10052aaf(index);
    int16_t i;
    int16_t number;

    if (current == -1) {
        current = (int16_t)Sim_10012843(index);
        if (current == -1)
            return 0;
    }
    number = 1;
    for (i = 0; i < S32(group, 4); i++) {
        uint32_t rec = PTRTAB(U32(group, 0x1c + (int)i * 4));

        if (S32(rec, 0) == 1 && (S16(rec, 0x1c) & 6) == 0) {
            if (S32(rec, 4) == (int)current) {
                U32(rec, 0x10) = 0;
            } else {
                U32(rec, 0x10) = (uint32_t)(int)number;
                number = (int16_t)(number + 1);
            }
        }
    }
    if (U32(group, 8) != 0) {
        uint32_t rec = PTRTAB(U32(group, 0x2c));

        if ((S16(rec, 0x1c) & 6) == 0)
            U32(rec, 0x10) = 4;
    }
    return 1;
}
