/*
 * Test entry for the checker only (spec_w5b.py): 0x10047e1a returns a float in st(0). The C version is wrapped so
 * that the value is stored as a double at a fixed address (compared by the checker like all memory); the original
 * gets a machine-code trampoline that does the same.
 */
#include "sim/unsorted_1004/w5b.h"

#define OUT ((volatile double *)0x00AFF000)

int __cdecl Test_10047e1a(void *obj, float x)
{
    *OUT = Sim_10047e1a(obj, x);
    return 0;
}
