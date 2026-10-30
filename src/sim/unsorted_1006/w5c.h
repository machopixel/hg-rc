/*
 * Round 5, letter c: function at 0x10068ce0 that calls helpers which are not converted yet (called through their address with SIM_FN).
 *
 * The floating-point part of the original keeps its intermediates in x87 registers at the 53-bit precision the Windows loader
 * sets, so every intermediate is a double here and only the stored results are floats.
 */
#ifndef HG_SIM_UNSORTED_1006_W5C_H
#define HG_SIM_UNSORTED_1006_W5C_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Not converted: takes a double on the stack, returns its absolute value in st(0). */
#define SIM_FABS_C SIM_FN(double(__cdecl *)(double), 0x100fcb70)
/* Not converted: takes a float, returns a double in st(0) (the sine of an angle in the units used here, see Sim_10068ce0). */
#define SIM_CALL_10084766_C SIM_FN(double(__cdecl *)(float), 0x10084766)

/*
 * Rotates the point (x, y) by `angle`: with s = 0x10084766(angle) and c = 0x10084766(angle + 90.0f) it stores
 * x * c - y * s in *outA and x * s + y * c in *outB, and `raw` unchanged in *outC. Does nothing when one of the three
 * pointers is NULL or fabs(angle) is below 1e-7f (0x1012b5f8), or NaN.
 */
void __cdecl Sim_10068ce0(float x, float y, uint32_t raw, float angle, float *outA, float *outB, uint32_t *outC);

#endif
