/*
 * Test entries for the checker only (spec_w7q.py). Crt_AToI64 returns a 64-bit value in edx:eax; the checker
 * compares only eax and memory, so this wrapper stores the whole result at a fixed address in argument memory
 * (compared like all memory). The original gets a machine-code trampoline in the spec that does the same.
 */
#include "sim/unsorted_100f/w7q.h"

#define OUT_Q ((volatile uint32_t *)0x00AFF000)

int __cdecl Test_100fd1c0(const char *s)
{
    int64_t v = Crt_AToI64(s);

    OUT_Q[0] = (uint32_t)(uint64_t)v;
    OUT_Q[1] = (uint32_t)((uint64_t)v >> 32);
    return 0;
}
