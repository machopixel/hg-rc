/* Round 3 functions of HGSIM.DLL in 0x100cxxxx (letter p). */
#ifndef HG_SIM_UNSORTED_100C_W3P_H
#define HG_SIM_UNSORTED_100C_W3P_H

#include <stdint.h>

#include "../simmem.h"

/*
 * 5000.0f when obj is NULL or its block (+0x28) has dword +0x4fc == -1; else, with rec = the 0x128-byte record
 * number block[0x4fc] of the array at block+0x84 and k = rec[4], (float at 0x10145508 + 0xd6 * k + 0x50 plus the float at +0x4c) * float 0x1012bb28.
 * The original returns it in st(0).
 */
float __cdecl Sim_100c05cb(const void *obj);

/* x limited to [-limit, limit] (x when it is NaN; limit is returned when x > limit, -limit when -limit > x). The original returns it in st(0). */
float __cdecl Math_ClampSymmetric_100c351c(float x, float limit);

#endif
