/*
 * Round 3 (letter o) functions around 0x10084a94..0x100881dd: the distance
 * estimate used throughout the collision code, the normalisation built on it,
 * and the step that lists the cell objects around a position.
 */
#ifndef HG_SIM_UNSORTED_1008_W3O_H
#define HG_SIM_UNSORTED_1008_W3O_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Estimate of the length of the vector (x, y, z): with the absolute values
 * sorted so that a is the largest of |x|, |z| and then of |y| (the other two
 * are not sorted against each other), the result is (a * K + b + c) / K, K the
 * float at 0x1012b8a8. The original leaves it in st(0) (unrounded), so it is
 * returned as a double; callers that store it to a float round it themselves.
 */
double __cdecl Sim_10084a94(float x, float y, float z);

/* Divides the three floats by their Sim_10084a94 length when that is greater than the float at 0x1012b870. */
void __cdecl Sim_10084c95(float *x, float *y, float *z);

/*
 * Fills the list of pointers at 0x1036f2a0 (terminated by a zero) with the
 * cell records of the 9 x 9 grid at 0x1017ba4c (0x18c bytes per column, 0x2c
 * per row) that lie in the up to four cells next to (x, z) and have a non-zero
 * dword at +8. Without the grid flag [0x1017c8c4] the list is the single entry
 * [0x1017c8cc]. Leaves its working values in 0x1036f248, 0x1036f3f8, 0x1036f400
 * and 0x1036f404.
 */
void __cdecl Sim_100881dd(float x, float z);

#endif
