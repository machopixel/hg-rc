/*
 * Round 6, letter h: function at 0x100e2776 (x87 floating point at the 53-bit precision the Windows loader sets);
 * it calls the C library fabs, which is not converted yet, through its original address.
 */
#ifndef HG_SIM_UNSORTED_100E_W6H_H
#define HG_SIM_UNSORTED_100E_W6H_H

#include <stdint.h>
#include "../simmem.h"
#include "../libm/hg_math.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Not converted: the C library fabs (a double on the stack, the result in st(0)). */
#define W6H_FABS_E hg_fabs

/* Stores x, y, z in the floats 0x1015264c, 0x10152650, 0x10152654; one whose fabs is below 1e-7f (0x1012bdc4), or NaN, becomes 1.0f. */
void __cdecl Sim_100e2776(float x, float y, float z);

#endif
