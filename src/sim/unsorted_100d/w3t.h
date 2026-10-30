/*
 * Round 3 (letter t) functions of HGSIM.DLL in 0x100dxxxx. Plain C, original
 * cdecl convention.
 */
#ifndef HG_SIM_UNSORTED_100D_W3T_H
#define HG_SIM_UNSORTED_100D_W3T_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Copies a packet into slot [0x1015d17c] of the 64 slots of 0x744 bytes at
 * 0x10a00040 (and increments the counter). The packet has its size in the dword
 * at +4, a name at +8 (0x40 bytes) and (size - 0x48) / 0x1c entries of 7 dwords
 * at +0x48: the first six are stored as floats, the seventh as it is. Returns 0
 * when all slots are used or there are more than 0x40 entries, else 1.
 */
int __cdecl Sim_100dacf9(const void *packet);

/*
 * Scales the four plane coefficients so that their absolute values add up to
 * 2^29 and stores them truncated as ints; nothing is stored when the sum is
 * not above 0.0001.
 */
void __cdecl Plane_Quantize(float a, float b, float c, float d, int32_t *outA, int32_t *outB, int32_t *outC,
                            int32_t *outD);

/*
 * Plane through three points (x0 y0 z0 x1 y1 z1 x2 y2 z2) passed to
 * Plane_Quantize with the four output pointers. Kept as the original has it:
 * the third edge's z is z2 - y0 (not z2 - z0).
 */
void __cdecl Plane_FromPoints(float x0, float y0, float z0, float x1, float y1, float z1, float x2, float y2,
                              float z2, int32_t *outA, int32_t *outB, int32_t *outC, int32_t *outD);

/*
 * Plane through the point (x, y, z) with the coefficients (a, -b, c) (the
 * second one is negated), d = -(a*x + (-b)*y + c*z), passed to Plane_Quantize.
 */
void __cdecl Plane_FromPointNormal(float x, float y, float z, float a, float b, float c, int32_t *outA,
                                   int32_t *outB, int32_t *outC, int32_t *outD);

#endif
