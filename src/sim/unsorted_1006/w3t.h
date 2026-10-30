/*
 * Round 3 (letter t) functions of HGSIM.DLL in 0x1006xxxx. Plain C, original
 * cdecl convention.
 */
#ifndef HG_SIM_UNSORTED_1006_W3T_H
#define HG_SIM_UNSORTED_1006_W3T_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* The float at index row * 2 + column of the array p, or 0.0 when p is null. Returned in st(0). */
float __cdecl Sim_10068e55(const float *p, int row, int column);

#endif
