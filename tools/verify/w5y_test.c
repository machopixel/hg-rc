/*
 * Test entry for the checker only (spec_w5y.py): Sim_100144bb returns a float in st(0); the wrapper stores it
 * as a float at a fixed address (compared by the checker like all memory); the original gets a machine-code
 * trampoline that does the same.
 */
#include "sim/unsorted_1001/w5y.h"

#define OUT ((volatile float *)0x00AFF000)

int __cdecl Test_100144bb(uint32_t unused, float f, int n)
{
    *OUT = Sim_100144bb(unused, f, n);
    return 0;
}
