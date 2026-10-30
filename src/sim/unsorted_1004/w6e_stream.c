#include "w6e.h"
#include "w5c.h"
#include "w5z.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x1004dbc5
int __cdecl Sim_1004dbc5(const uint32_t *rec)
{
    uint8_t flags;

    if (W6E_CALL_1004dd2a(rec[0]) != 0)
        return 2;
    if (W6E_CALL_1004dd2a(rec[1]) != 0)
        return 2;
    /* the original packs the bit fields 7, 6..4, 3 and 2..0 of this byte again, which gives the same byte */
    flags = U8(rec, 8);
    if (Sim_1004dcaa(flags) != 0)
        return 2;
    if (Sim_1004dcaa(U8(rec, 9)) != 0)
        return 2;
    if (Sim_1004dcaa(U8(rec, 10)) != 0)
        return 2;
    return 0;
}

#define BIT_BUFFER GBYTES(0x1015f230)
#define BIT_INDEX G32(0x1015f340)
#define BITS_FREE G32(0x1015f348)

// FUNCTION: HGSIM 0x1004e24a
int __cdecl Sim_1004e24a(int32_t value, int32_t bits)
{
    uint32_t written = 0;
    uint32_t mask;

    do {
        if ((BIT_INDEX == 0xfe && BITS_FREE == 0) || (int32_t)BIT_INDEX > 0xfe) {
            if (Sim_1004dcaa(0xff) != 0)
                return -1;
            if (Sim_1004db5b(BIT_BUFFER, 0xff) != 0)
                return -1;
            BIT_INDEX = 0;
            BIT_BUFFER[(int32_t)BIT_INDEX] = 0;
            BITS_FREE = 8;
        }
        /* shift counts are taken modulo 32 like the x86 shift instructions */
        if (bits <= (int32_t)BITS_FREE) {
            mask = (1u << ((uint32_t)bits & 31)) - 1;
            BIT_BUFFER[(int32_t)BIT_INDEX] |= (uint8_t)(((uint32_t)value & mask) << ((8u - BITS_FREE) & 31));
            written += (uint32_t)bits;
            BITS_FREE = BITS_FREE - (uint32_t)bits;
            bits = 0;
        } else {
            mask = (1u << (BITS_FREE & 31)) - 1;
            BIT_BUFFER[(int32_t)BIT_INDEX] |= (uint8_t)(((uint32_t)value & mask) << ((8u - BITS_FREE) & 31));
            written += BITS_FREE;
            value >>= BITS_FREE & 31;
            bits = (int32_t)((uint32_t)bits - BITS_FREE);
            BIT_INDEX = BIT_INDEX + 1;
            BIT_BUFFER[(int32_t)BIT_INDEX] = 0;
            BITS_FREE = 8;
        }
    } while (bits != 0);
    return (int)written;
}
