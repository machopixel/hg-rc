/*
 * Test entries for the checker only (spec_w7m.py): Sim_10047cf2 returns a float in st(0). The C version is wrapped
 * so that the value is stored as a double at a fixed address (compared by the checker like all memory); the
 * original gets a machine-code trampoline that does the same.
 */
#include "sim/unsorted_1004/w7m.h"

#define OUT ((volatile double *)0x00AFF000)

int __cdecl Test_10047cf2(uint32_t *obj)
{
    *OUT = Sim_10047cf2(obj);
    return 0;
}
