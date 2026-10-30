/*
 * Test entries for the checker only (spec_w3t.py): the functions that return a float in st(0)
 * are wrapped so that the value is stored as a float at a fixed address (compared by the
 * checker like all memory); the original gets a machine-code trampoline that does the same.
 */
#include "sim/unsorted_1001/w3t.h"
#include "sim/unsorted_1004/w3t.h"
#include "sim/unsorted_1006/w3t.h"
#include "sim/unsorted_100c/w3t.h"

#define OUT ((volatile float *)0x00AFF000)

int __cdecl Test_10014061(int a)
{
    *OUT = Sim_10014061((int16_t)a);
    return 0;
}

int __cdecl Test_10015266(const void *obj)
{
    *OUT = Sim_10015266(obj);
    return 0;
}

int __cdecl Test_1004bc65(int a, int b)
{
    *OUT = Sim_1004bc65(a, b);
    return 0;
}

int __cdecl Test_10068e55(const float *p, int a, int b)
{
    *OUT = Sim_10068e55(p, a, b);
    return 0;
}

int __cdecl Test_100c3725(void *obj)
{
    *OUT = Sim_100c3725(obj);
    return 0;
}
