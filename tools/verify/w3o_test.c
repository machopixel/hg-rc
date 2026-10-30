/*
 * Test entries for the checker only (spec_w3o.py): functions of round 3 (letter o) that leave their result in
 * st(0). The wrapper stores it as a double at 0xAF1000 (the checker's trampoline does the same for the original).
 */
#include "sim/unsorted_1000/w3o.h"
#include "sim/unsorted_1002/w3o.h"
#include "sim/unsorted_1008/w3o.h"
#include "sim/unsorted_1009/w3o.h"

#define RESULT (*(volatile double *)0xAF1000)

void __cdecl Test_1000946e(void *o, uint32_t *x, uint32_t *y, uint32_t *z) { RESULT = Sim_1000946e(o, x, y, z); }
void __cdecl Test_10009ca8(void *o, float x, float y, float z) { RESULT = Sim_10009ca8(o, x, y, z); }
void __cdecl Test_10009d7e(void *o, void *ray) { RESULT = Sim_10009d7e(o, ray); }
void __cdecl Test_10009eb3(void *o, void *ray) { RESULT = Sim_10009eb3(o, ray); }
void __cdecl Test_1002d961(float x, float y, float z) { RESULT = Sim_1002d961(x, y, z); }
void __cdecl Test_10084a94(float x, float y, float z) { RESULT = Sim_10084a94(x, y, z); }
void __cdecl Test_1009cbde(void *seg) { RESULT = Sim_1009cbde(seg); }
