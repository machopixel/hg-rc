/* Round 3 functions of HGSIM.DLL in 0x100dxxxx (letter p). */
#ifndef HG_SIM_UNSORTED_100D_W3P_H
#define HG_SIM_UNSORTED_100D_W3P_H

#include <stdint.h>

#include "../simmem.h"

/*
 * With s = |x| + |y| + |z| (|v| is v when v > float 0x1012bd8c, else -v): when s > double 0x1012bd90,
 * stores trunc(x / t), trunc(y / t), trunc(z / t) in the three ints, with t = float(float(s) / double 0x1012bd98).
 * Nothing is stored otherwise.
 */
void __cdecl Sim_100db3f7(float x, float y, float z, int32_t *outX, int32_t *outY, int32_t *outZ);

/*
 * Cross product of the edges (p1 - p0) and (p2 - p0) of the triangle given as nine floats, passed on to
 * Sim_100db3f7 with the three out pointers. The z of the second edge is computed as z2 - y0, as in the original (not z2 - z0).
 */
void __cdecl Sim_100db5c6(float x0, float y0, float z0, float x1, float y1, float z1, float x2, float y2, float z2,
                          int32_t *outX, int32_t *outY, int32_t *outZ);

#endif
