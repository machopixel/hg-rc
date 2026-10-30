/*
 * Test entries for the checker only (spec_w8t.py). Sim_1001412d returns a float in st(0): the wrapper stores it
 * as a double at a fixed address (compared by the checker like all memory); the original gets a machine-code
 * trampoline that does the same. The 16-bit results (only ax is read by the callers) are returned zero-extended
 * on both sides.
 */
#include "sim/unsorted_1001/w8t.h"

#define OUT ((volatile double *)0x00AFF000)

int __cdecl Test_1001412d(void *obj, uint32_t limit)
{
    union { uint32_t u; float f; } v;

    v.u = limit;
    *OUT = Sim_1001412d(obj, v.f);
    return 0;
}

uint32_t __cdecl Test_1001305e(void *obj, uint32_t key, uint32_t sel)
{
    return Sim_1001305e(obj, (int16_t)key, (int16_t)sel);
}

uint32_t __cdecl Test_10013297(void *obj, uint32_t key, uint32_t sel)
{
    return Sim_10013297(obj, (int16_t)key, (int16_t)sel);
}

uint32_t __cdecl Test_10013372(void *obj, uint32_t key, uint32_t sel)
{
    return Sim_10013372(obj, (int16_t)key, (int16_t)sel);
}

uint32_t __cdecl Test_10013848(void *obj, uint32_t key, uint32_t sel)
{
    return Sim_10013848(obj, (int16_t)key, (int16_t)sel);
}

uint32_t __cdecl Test_10015d30(void *obj, uint32_t key, uint32_t sel, uint32_t other)
{
    return Sim_10015d30(obj, (int16_t)key, (int16_t)sel, (int16_t)other);
}
