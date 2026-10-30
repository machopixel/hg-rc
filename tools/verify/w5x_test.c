/*
 * Test entry for the checker only (spec_w5x.py): 0x10084b57 returns a double in st(0). The C version is
 * wrapped so that the value is stored as a double at a fixed address (compared by the checker like all
 * memory); the original gets a machine-code trampoline that does the same.
 */
#include "sim/unsorted_1008/w5x.h"

#define OUT ((volatile double *)0x00AFF000)

int __cdecl Test_10084b57(float a, float b)
{
    *OUT = Sim_10084b57(a, b);
    return 0;
}
