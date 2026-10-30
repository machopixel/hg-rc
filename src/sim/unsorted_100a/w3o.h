/*
 * Round 3 (letter o): 0x100a2c08 and 0x100a78eb.
 */
#ifndef HG_SIM_UNSORTED_100A_W3O_H
#define HG_SIM_UNSORTED_100A_W3O_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif


/*
 * Interpolates between the 6-dword points a and b (dwords 0, 1, 2, 3, 4, 5 = integers) at the
 * coordinate `value` of axis `mode` (1: dword 0, 2: dword 1) into out: the axis dword becomes
 * value, the other of the first two is a[k] + trunc((b[k] - a[k]) * t + the float at 0x1012ba54), likewise
 * dwords 2..5, with t = (value - a[axis]) / (b[axis] - a[axis]) as a float. For another mode t is
 * an uninitialised stack float in the original (0.0f here) and dwords 0 and 1 are left alone.
 */
void __cdecl Sim_100a2c08(int mode, int value, const int *a, const int *b, int *out);

/* Calls the function pointer stored at 0x1017d160 with arg and returns its result. */
int __cdecl Sim_100a78eb(uint32_t arg);

#endif
