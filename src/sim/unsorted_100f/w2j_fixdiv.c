#include "w2j.h"

/*
 * (hi:lo) / divisor by long division (no 64-bit division helper in the flat
 * test images). Returns the low 32 bits of the quotient; *rem is the remainder.
 */
static uint32_t Divide64(uint32_t hi, uint32_t lo, uint32_t divisor, uint32_t *rem)
{
    uint64_t numerator = ((uint64_t)hi << 32) | lo;
    uint64_t remainder = 0;
    uint32_t quotient = 0;
    int bit;

    if (divisor == 0) {
        *rem = 0;
        return 0;
    }
    for (bit = 63; bit >= 0; bit--) {
        remainder = (remainder << 1) | ((numerator >> bit) & 1);
        if (remainder >= divisor) {
            remainder -= divisor;
            if (bit < 32)
                quotient |= 1u << bit;
        }
    }
    *rem = (uint32_t)remainder;
    return quotient;
}

/* The quotient plus 1 when the remainder is at least half of the divisor (rounded up), negated for a negative sign. */
static int32_t DivideRounded(uint32_t hi, uint32_t lo, uint32_t divisor, int negative)
{
    uint32_t rem;
    uint32_t quotient = Divide64(hi, lo, divisor, &rem);

    if ((divisor >> 1) + (divisor & 1) - 1 < rem)
        quotient++;
    return (int32_t)(negative ? 0u - quotient : quotient);
}

// FUNCTION: HGSIM 0x100f9f50
int32_t __cdecl Sim_100f9f50(int32_t a, int32_t b)
{
    uint32_t magnitudeA = a < 0 ? 0u - (uint32_t)a : (uint32_t)a;
    uint32_t magnitudeB = b < 0 ? 0u - (uint32_t)b : (uint32_t)b;

    return DivideRounded((uint32_t)((int32_t)magnitudeA >> 2), magnitudeA << 30, magnitudeB, (a < 0) != (b < 0));
}

// FUNCTION: HGSIM 0x100f9f88
int32_t __cdecl Sim_100f9f88(int32_t b)
{
    uint32_t magnitude = b < 0 ? 0u - (uint32_t)b : (uint32_t)b;

    return DivideRounded(0x4000, 0, magnitude, b < 0);
}
