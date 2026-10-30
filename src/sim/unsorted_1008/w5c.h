/*
 * Round 5, letter c: functions at 0x1008d36c.. that call helpers which are not converted yet.
 * Those helpers are called through their original address (SIM_FN) with the prototypes below.
 */
#ifndef HG_SIM_UNSORTED_1008_W5C_H
#define HG_SIM_UNSORTED_1008_W5C_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Not converted: returns two signed 16-bit halves (high half = first, low half = second). */
#define SIM_CALL_100ed70b SIM_FN(int32_t(__cdecl *)(uint32_t), 0x100ed70b)

/* Not converted: x87 square root of a double (the C library sqrt). */
#define SIM_SQRT_C SIM_FN(double(__cdecl *)(double), 0x100fca04)

/*
 * Fills two tables of 0x320 floats: with t = i / 16.0 (the qword at 0x1012b898), inv = 1.0 / sqrt(t * t + 1.0) (the qword at 0x1012b8a0)
 * is stored at 0x10370d20 + 4*i and t * inv at 0x1036f860 + 4*i. The intermediates are doubles (x87 at the 53-bit precision of the loader).
 * Returns 1.
 */
int __cdecl Sim_10084a17(void);
/*
 * Scales the rectangle around its center (Rect_ScaleAroundCenter) by the two halves of 0x100ed70b(arg), each turned
 * into a 16.16 fraction of the rectangle's width / height (right-left+1, bottom-top+1). Returns `out`.
 * The original divides with a 64-bit idiv that traps when the quotient does not fit in 32 bits; this version does not trap there.
 */
int32_t *__cdecl Sim_1008d36c(const int32_t *rect, int32_t *out, uint32_t arg);

#endif
