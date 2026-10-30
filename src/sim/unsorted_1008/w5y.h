/*
 * Round 5 (letter y) functions of HGSIM.DLL in 0x1008xxxx. Plain C, original cdecl convention.
 * They call helpers that are not converted yet through their original address (SIM_FN), so the
 * original code still runs for those. Names carry the original address because the code alone
 * does not say what the data is for.
 */
#ifndef HG_SIM_UNSORTED_1008_W5Y_H
#define HG_SIM_UNSORTED_1008_W5Y_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* When the dword at 0x1015ecd0 is 0: Sim_100e46d0(0x80, 1) and Sim_1001f4ba(). */
void __cdecl Sim_1008b8de(void);
/*
 * s = sqrt(a) (Sim_100fcb70 on the double of a). When s is below 1.0e-7 (or not a number) returns 1; otherwise stores
 * b / a (computed in double, stored as float) at *out and returns 0.
 */
int __cdecl Sim_1008e211(float a, float b, float *out);

#endif
