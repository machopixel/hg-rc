/*
 * Round 5, letter x: 0x1008b757 and 0x1008b773 (built on Sim_1000e983) and 0x10084b57, which calls the C
 * library square root (not converted, called through its address, SIM_FN).
 */
#ifndef HG_SIM_UNSORTED_1008_W5X_H
#define HG_SIM_UNSORTED_1008_W5X_H

#include <stdint.h>
#include "../simmem.h"
#include "../libm/hg_math.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Not converted: the C library's sqrt (a double on the stack, the result in st(0)). */
#define SIM_CALL_100fca04 hg_sqrt

/* Sim_1000e983(0x1014b950, 0, 0, Sim_1000edce()). */
void __cdecl Sim_1008b757(void);
/* Sim_1000e983(0x1014b95c, 0, 0, Sim_1000edce()). */
void __cdecl Sim_1008b773(void);
/*
 * sqrt(a * a + b * b), computed in doubles. The original returns the double in st(0) without rounding it to a
 * float (it also stores a float copy in a local nobody reads), and its caller compares it against a double
 * constant, so this returns the double.
 */
double __cdecl Sim_10084b57(float a, float b);

#endif
