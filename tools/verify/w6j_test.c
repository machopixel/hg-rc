/*
 * Test entries for the checker only (spec_w6j.py). 0x10103870 returns a double in st(0) and 0x10101a50 a 64-bit
 * value in edx:eax; the checker compares only eax and memory, so these wrappers store the whole result at a
 * fixed address in argument memory (compared like all memory). The originals get machine-code trampolines in the
 * spec that do the same.
 */
#include "sim/unsorted_1010/w6j.h"

#define OUT_D ((volatile double *)0x00AFF000)
#define OUT_Q ((volatile uint32_t *)0x00AFF000)

int __cdecl Test_10103870(double x, int e)
{
    *OUT_D = Sim_10103870(x, e);
    return 0;
}

int __cdecl Test_10101a50(uint32_t *cursor)
{
    uint64_t v = Sim_10101a50(cursor);

    OUT_Q[0] = (uint32_t)v;
    OUT_Q[1] = (uint32_t)(v >> 32);
    return 0;
}
