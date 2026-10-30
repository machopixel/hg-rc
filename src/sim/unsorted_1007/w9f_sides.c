#include "w9f.h"
#include "../util/access.h"
#include "../unsorted_100f/w2e.h"

// FUNCTION: HGSIM 0x10078f60
void __cdecl Sim_10078f60(const uint8_t *p)
{
    uint32_t count = (U32(p, 4) - 0x26u) / 0x97u;
    uint32_t side = U32(p, 8);
    uint8_t *block = W9F_SIDE(side);
    int32_t i, c;

    if (U32(p, 0x26) != 0x10)
        W9F7_CALL_100bc280(0x4a, 0);
    block[0x39] = U8(GBYTES(0x109b7d4c), side * 0x6cu);
    U32(block, 0xc) = U32(p, 0xc);
    U32(block, 4) = 0xffffffffu;
    U32(block, 8) = 0xffffffffu;
    Str_Copy((char *)block + 0x18, (const char *)p + 0x12);
    Str_Copy((char *)block + 0x28, (const char *)p + 0x1d);
    for (i = 0; i < (int32_t)count; i++) {
        const uint8_t *m = p + 0x26 + (uint32_t)i * 0x97u;
        uint8_t *row = W9F_ROW(side, i);

        row[1] = m[0x2e];
        U32(row, 2) = U32(m, 0);
        U32(row, 0x6f) = U32(m, 0x2a);
        U32(row, 0x67) = 0xffffffffu;
        U32(row, 0x6b) = 0xffffffffu;
        U16(row, 0xa9) = U16(m, 0x53);
        U16(row, 0xab) = U16(m, 0x55);
        row[0x74] = m[4] == 'V';
        U32(row, 0x75) = m[0x2f] == 'M';
        U32(row, 0x79) = m[0x30];
        U32(row, 0x7d) = m[0x31];
        Str_Copy((char *)row + 0x89, (const char *)m + 0x34);
        Str_Copy((char *)row + 0x99, (const char *)m + 0x3f);
        Str_Copy((char *)row + 0xad, (const char *)m + 0x57);
        row[6] = m[6];
        for (c = 0; c < 8; c++) {
            uint32_t kind;

            switch (m[0xa + c * 4]) {
            case 'C':
                kind = 1;
                break;
            case 'S':
                kind = 2;
                break;
            case 'F':
                kind = 3;
                break;
            default:
                kind = 0;
                break;
            }
            U32(row, 7 + c * 0xc) = kind;
            U32(row, 0xb + c * 0xc) = m[0xb + c * 4];
            U32(row, 0xf + c * 0xc) = m[0xc + c * 4];
        }
        row[0] = 0;
        row[0xee] = 0;
        U16(row, 0xef) = 0x800;
    }
    SIM_PTR(u32a, 0x104727a0)[side] = 0;
    U32(block, 0) = count;
    if (GS32(0x104737e0) < (int32_t)(side + 1u))
        G32(0x104737e0) = side + 1u;
}

// FUNCTION: HGSIM 0x1007cbcb
uint32_t __cdecl Sim_1007cbcb(int side)
{
    uint8_t *block = W9F_SIDE(side);
    int32_t i;

    for (i = 0; i < S32(block, 0); i++) {
        uint32_t off = (uint32_t)i * 0x13fu;

        if (U32(block, off + 0x3c) == 0x20)
            return (off & 0xffff0000u) | U16(block, off + 0x129);
    }
    return ((uint32_t)i & 0xffff0000u) | U16(block, 0x129);
}
