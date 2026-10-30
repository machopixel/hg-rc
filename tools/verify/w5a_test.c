/*
 * Test entry for the checker only (spec_w5a.py): 0x100c34b7 returns a float in st(0). The C version is
 * wrapped so that the value is stored as a float at a fixed address (compared by the checker like all
 * memory); the original gets a machine-code trampoline that does the same.
 */
#include "sim/unsorted_100c/w5a.h"

#define OUT ((volatile float *)0x00AFF000)

float __cdecl Sim_100c34b7(void *obj, int16_t key);

int __cdecl Test_100c34b7(void *obj, int key)
{
    *OUT = Sim_100c34b7(obj, (int16_t)key);
    return 0;
}
