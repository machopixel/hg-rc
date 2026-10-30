/*
 * Round 3 (letter q) function of HGSIM.DLL at 0x1006112e. Floating point is
 * done as the original's x87 did it (see util/w3q.h).
 */
#ifndef HG_SIM_UNSORTED_1006_W3Q_H
#define HG_SIM_UNSORTED_1006_W3Q_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Quaternion product out = a * b, quaternions as four floats (x, y, z, w):
 *   w = a.w * b.w - a.xyz . b.xyz,   xyz = a.w * b.xyz + b.w * a.xyz + a.xyz x b.xyz.
 * Every output is computed before any is stored, so out may be a or b.
 */
void __cdecl Quat_Multiply(const float *a, const float *b, float *out);

#endif
