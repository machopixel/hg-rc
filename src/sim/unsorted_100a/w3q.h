/*
 * Round 3 (letter q) functions of HGSIM.DLL around 0x100a2000..0x100aa400.
 * Names carry the original address because the code alone does not say what
 * the fields mean. Floating point is done as the original's x87 did it
 * (see util/w3q.h).
 */
#ifndef HG_SIM_UNSORTED_100a_W3Q_H
#define HG_SIM_UNSORTED_100a_W3Q_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Orientation changes of an object that keeps two quaternions (floats x, y, z, w) at obj+0x3c
 * and obj+0x7c and a flag dword at obj+0x94. Each multiplies one of them in place
 * (Quat_Multiply), clears one flag bit and sets bit 0 of the flag dword:
 *   Sim_100aa1de: obj+0x3c = q * obj+0x3c, clears bit 14 (0x4000)
 *   Sim_100aa227: obj+0x3c = obj+0x3c * q, clears bit 14 (0x4000)
 *   Sim_100aa2be: obj+0x7c = q * obj+0x7c, clears bit 15 (0x8000)
 */
void __cdecl Sim_100aa1de(void *obj, const float *q);
void __cdecl Sim_100aa227(void *obj, const float *q);
void __cdecl Sim_100aa2be(void *obj, const float *q);

#endif
