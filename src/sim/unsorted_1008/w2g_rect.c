#include "w2g.h"

/*
 * (value << 16) / divisor, the 64-bit signed division of the original (cdq,
 * shld, shl, idiv) done with 32-bit operations: with value = whole * divisor +
 * rest (both truncated toward zero, so of the same sign) the quotient is
 * whole * 65536 + rest * 65536 / divisor, and the second term fits easily.
 */
static int32_t Fraction(int32_t value, int32_t divisor)
{
    int32_t whole = value / divisor;
    int32_t rest = value % divisor;

    return (int32_t)((uint32_t)whole * 65536u + (uint32_t)(rest * 65536 / divisor));
}

// FUNCTION: HGSIM 0x1008cf1f
int32_t *__cdecl Sim_1008cf1f(const int32_t *src, int32_t *dst)
{
    dst[1] = Fraction(src[1], 0x13F);
    dst[2] = Fraction(src[2], 0xC7);
    dst[3] = Fraction(src[3], 0x13F);
    dst[4] = Fraction(src[4], 0xC7);
    return dst;
}
