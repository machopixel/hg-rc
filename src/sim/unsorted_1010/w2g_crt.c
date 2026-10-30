#include "w2g.h"

// FUNCTION: HGSIM 0x101003d0
int __cdecl Math_AbsInt_101003d0(int value)
{
    return value < 0 ? (int)(0u - (unsigned)value) : value;
}

// FUNCTION: HGSIM 0x10101d48
uint32_t __cdecl Sim_10101d48(uint32_t low, uint32_t high)
{
    uint32_t exponent = high & 0x7FF00000u;

    (void)low;
    if (exponent != 0x7FF00000u)
        return exponent;
    return high;
}

// FUNCTION: HGSIM 0x10103830
int __cdecl Sim_10103830(uint32_t status)
{
    if (status & 0x20)
        return 5;
    if (status & 8)
        return 1;
    if (status & 4)
        return 2;
    if (status & 1)
        return 3;
    return (int)(((status & 0xFF) & 2) << 1);
}

// FUNCTION: HGSIM 0x1010b270
uint32_t __cdecl Str_LengthMax_1010b270(const char *s, uint32_t max)
{
    const char *p = s;
    uint32_t left = max;

    if (left != 0) {
        for (;;) {
            if (*p == 0)
                break;
            p++;
            if (left-- == 1)
                break;
        }
    }
    if (*p == 0)
        return (uint32_t)(p - s);
    return max;
}

// FUNCTION: HGSIM 0x1010b5a0
int __cdecl Wstr_Length_1010b5a0(const uint16_t *s)
{
    const uint16_t *p = s;

    while (*p != 0)
        p++;
    return (int)(p - s);
}
