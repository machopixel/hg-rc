/*
 * Test entries for the checker only (spec_w8w.py). These functions return a float in st(0): the wrapper stores it
 * as a double at a fixed address (compared by the checker like all memory); the original gets a machine-code
 * trampoline that does the same.
 */
#include "sim/unsorted_1008/w8w.h"

#define OUT ((volatile double *)0x00AFF000)

int __cdecl Test_10084766(float a)
{
    *OUT = Sim_10084766(a);
    return 0;
}

int __cdecl Test_1008486e(float x, float y)
{
    *OUT = Sim_1008486e(x, y);
    return 0;
}

int __cdecl Test_10088e37(uint8_t *obj)
{
    *OUT = Sim_10088e37(obj);
    return 0;
}

int __cdecl Test_10089325(uint8_t *obj)
{
    *OUT = Sim_10089325(obj);
    return 0;
}
