#include "w2j.h"
#include "../util/access.h"

/* Low 32 bits of (value << 16) / divisor as the original's cdq/shld/idiv gives them (divisor > 0). */
static int32_t DivShifted16(int32_t value, uint32_t divisor)
{
    uint32_t magnitude = value < 0 ? 0u - (uint32_t)value : (uint32_t)value;
    uint64_t numerator = (uint64_t)magnitude << 16;
    uint64_t remainder = 0, quotient = 0;
    int bit;

    for (bit = 47; bit >= 0; bit--) {
        remainder = (remainder << 1) | ((numerator >> bit) & 1);
        if (remainder >= divisor) {
            remainder -= divisor;
            quotient |= (uint64_t)1 << bit;
        }
    }
    return value < 0 ? (int32_t)(0u - (uint32_t)quotient) : (int32_t)(uint32_t)quotient;
}

// FUNCTION: HGSIM 0x100877a0
void __cdecl Sim_100877a0(void)
{
}

// FUNCTION: HGSIM 0x100877b5
int __cdecl Sim_100877b5(void)
{
    return 0;
}

// FUNCTION: HGSIM 0x1008cfce
int32_t *__cdecl Sim_1008cfce(const int32_t *src, int32_t *dst)
{
    S32(dst, 0) = DivShifted16(S32(src, 0), 0x13f);
    S32(dst, 4) = DivShifted16(S32(src, 4), 0xc7);
    S32(dst, 8) = DivShifted16(S32(src, 8), 0x13f);
    S32(dst, 0xc) = DivShifted16(S32(src, 0xc), 0xc7);
    return dst;
}

// FUNCTION: HGSIM 0x1008f64c
void __cdecl Sim_1008f64c(void *p)
{
    U32(U32(p, 0), 0) = 0;
    U32(U32(p, 0), 4) = 0;
}
