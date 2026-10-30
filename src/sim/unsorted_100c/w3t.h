/*
 * Round 3 (letter t) functions of HGSIM.DLL in 0x100cxxxx. Plain C, original
 * cdecl convention.
 */
#ifndef HG_SIM_UNSORTED_100C_W3T_H
#define HG_SIM_UNSORTED_100C_W3T_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * With the divisor int 0x1015dc80 not zero: returns (float obj+0x268 - float
 * obj+0xd4) / divisor in st(0) and copies the dword obj+0xd4 to obj+0x268.
 * With a zero divisor returns 0.0 and changes nothing.
 */
float __cdecl Sim_100c3725(void *obj);

#endif
