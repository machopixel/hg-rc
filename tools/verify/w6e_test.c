/*
 * Test entry for the checker only (spec_w6e.py): 0x10047bcd returns a float in st(0). The C version is wrapped so
 * that the value is stored as a float at a fixed address (compared by the checker like all memory); the original
 * gets a machine-code trampoline that does the same.
 */
#include "sim/unsorted_1004/w6e.h"

#define OUT ((volatile float *)0x00AFF000)

int __cdecl Test_10047bcd(uint32_t *obj)
{
    *OUT = Sim_10047bcd(obj);
    return 0;
}
