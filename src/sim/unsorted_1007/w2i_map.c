#include "w2i.h"
#include "../util/access.h"

/* Rows of 0x13f bytes in maps of 0x4fc3a bytes; the address of row b of map a is 0x1047383a + a * 0x4fc3a + b * 0x13f. */
#define MAP_ROW(a, b) ((uint32_t)0x1047383Au + (uint32_t)(a) * 0x4fc3au + (uint32_t)(b) * 0x13fu)

// FUNCTION: HGSIM 0x100765a7
uint32_t __cdecl Sim_100765a7(void *stack)
{
    uint32_t result = 0;

    if (S32(stack, 0xc) > 0) {
        if (S32(stack, 0xc) == 1)
            G32(0x1017E5EC) = 0;
        U32(stack, 0xc) = U32(stack, 0xc) - 1u;
        result = U32(U32(stack, 8), U32(stack, 0xc) * 4u);
    }
    return result;
}

// FUNCTION: HGSIM 0x1007b42c
int __cdecl Sim_1007b42c(int a, int b, int i)
{
    uint32_t row = MAP_ROW(a, b);
    uint32_t cell = row + (uint32_t)i * 12u;
    uint32_t kind = U32(GBYTES(cell), 7);
    uint32_t cellRow = U32(GBYTES(cell), 0xb);
    uint32_t cellMap = U32(GBYTES(cell), 0xf) & 0xf;
    uint32_t value = U8(GBYTES(MAP_ROW(cellMap, cellRow)), 0);

    if ((kind == 1 && (value == 5 || value == 6)) || (kind == 2 && value == 5) || (kind == 3 && value == 6))
        return 1;
    return 0;
}

// FUNCTION: HGSIM 0x1007b4d6
int __cdecl Sim_1007b4d6(int a, int b)
{
    uint32_t row = MAP_ROW(a, b);
    uint32_t all = U8(GBYTES(row), 6);
    uint32_t result = 1;
    int i;

    if (U8(GBYTES(row), 0) == 5 || U8(GBYTES(row), 0) == 6)
        return 0;
    if (U32(GBYTES(row), 2) == 0x10 || U32(GBYTES(row), 2) == 0x8000 || U32(GBYTES(row), 2) == 0x8001
        || U32(GBYTES(row), 2) == 0x8002)
        return 1;
    for (i = 0; i < 8; i++) {
        int hit;

        if (U32(GBYTES(row), i * 12 + 7) == 0)
            break;
        hit = Sim_1007b42c(a, b, i);
        if (all != 0) {
            result &= (uint32_t)hit;
            if (result == 0)
                return 0;
        } else if (hit != 0) {
            return 1;
        }
    }
    return all != 0 ? 1 : 0;
}

// FUNCTION: HGSIM 0x1007caab
void __cdecl Sim_1007caab(int a)
{
    uint32_t map = 0x10473800u + (uint32_t)a * 0x4fc3au;
    int i;

    G32(0x1037305C) = (uint32_t)(GS32(0x1015DC78) / 0xb5);
    for (i = 0; i < S32(GBYTES(map), 0); i++) {
        uint32_t cell = map + (uint32_t)i * 0x13fu + 0x3au;

        if (U32(GBYTES(cell), 2) == 0x40000 && U8(GBYTES(cell), 0) == 5) {
            uint32_t value = U8(GBYTES(0x104738AEu + (uint32_t)(int32_t)S16(GBYTES(cell), 0xa9) * 0x4fc3au
                                       + (uint32_t)(int32_t)S16(GBYTES(cell), 0xab) * 0x13fu), 0);

            U8(GBYTES(0x104738AEu + (uint32_t)(int32_t)S16(GBYTES(cell), 0xa9) * 0x4fc3au
                       + (uint32_t)(int32_t)S16(GBYTES(cell), 0xab) * 0x13fu), 0) = (uint8_t)(value == 0);
        }
        U8(GBYTES(cell), 0) = 3;
        U32(GBYTES(cell), 0x67) = 0xFFFFFFFFu;
        U32(GBYTES(cell), 0x6b) = 0xFFFFFFFFu;
    }
    U8(GBYTES(map), 0x38) = 0;
    U32(GBYTES(map), 4) = G32(0x1037305C);
    U32(GBYTES(map), 8) = 0xFFFFFFFFu;
}

// FUNCTION: HGSIM 0x1007dbaa
int __cdecl Sim_1007dbaa(void)
{
    return 0;
}
