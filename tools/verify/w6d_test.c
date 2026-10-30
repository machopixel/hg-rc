/*
 * Test entries for the checker only (spec_w6d.py): these functions return a float in st(0). The C version is
 * wrapped so that the value is stored as a double at a fixed address (compared by the checker like all memory);
 * the original gets a machine-code trampoline that does the same.
 */
#include "sim/unsorted_1001/w6d.h"

#define OUT ((volatile double *)0x00AFF000)

int __cdecl Test_100127f0(const float *a, const float *b)
{
    *OUT = Sim_100127f0(a, b);
    return 0;
}

int __cdecl Test_1001448d(uint32_t unused, float f, int n)
{
    *OUT = Sim_1001448d(unused, f, n);
    return 0;
}

int __cdecl Test_1001655f(float x, int n, int flag)
{
    *OUT = Sim_1001655f(x, n, flag);
    return 0;
}
