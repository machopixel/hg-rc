/*
 * Test entries for the checker only (spec_w7l.py). Sim_1000cfe2 returns a float in st(0): the wrapper stores it
 * as a double at a fixed address (compared by the checker like all memory); the original gets a machine-code
 * trampoline that does the same. Sim_10015018 and Sim_10015f5b return a 16-bit value whose top half of eax no
 * caller reads: both sides return it zero-extended.
 */
#include "sim/unsorted_1000/w7l.h"
#include "sim/unsorted_1001/w7l.h"

#define OUT ((volatile double *)0x00AFF000)

int __cdecl Test_1000cfe2(void *obj)
{
    *OUT = Sim_1000cfe2(obj);
    return 0;
}

uint32_t __cdecl Test_10015018(void *obj, uint32_t sel, uint32_t key)
{
    return (uint16_t)Sim_10015018(obj, (int16_t)sel, (int16_t)key);
}

uint32_t __cdecl Test_10015f5b(void *a, void *obj)
{
    return (uint16_t)Sim_10015f5b(a, obj);
}
