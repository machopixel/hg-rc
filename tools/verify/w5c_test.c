/*
 * Test entry for the checker only (spec_w5c.py): Sim_100144e9 returns a float in st(0). The C version is wrapped so that
 * the value is stored as a float at a fixed address (compared by the checker like all memory); the original gets a
 * machine-code trampoline that does the same.
 */
#include "sim/unsorted_1001/w5c.h"

#define OUT ((volatile float *)0x00AFF000)

int __cdecl Test_100144e9(const void *obj)
{
    *OUT = Sim_100144e9(obj);
    return 0;
}
