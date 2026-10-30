/*
 * Round 3 functions of HGSIM.DLL in 0x1006xxxx (letter p): clamping a point to a box.
 * The box is 6 floats: lo and hi of x, of y, then of z.
 */
#ifndef HG_SIM_UNSORTED_1006_W3P_H
#define HG_SIM_UNSORTED_1006_W3P_H

#include <stdint.h>

#include "../simmem.h"

/*
 * Returns lo when value < lo (or either is NaN), hi when value > hi, else value. The original returns it in st(0), as a float.
 */
float __cdecl Math_ClampFloat_100686ea(float value, float lo, float hi);

/* With box, outX, outY, outZ all non-NULL: stores each coordinate clamped to the box and returns 1, else 0. */
int __cdecl Sim_10068716(const float *box, float x, float y, float z, float *outX, float *outY, float *outZ);

/* 1 when the point is inside the box, that is when clamping leaves all three coordinates unchanged (NaN counts as unchanged); 0 when box is NULL. */
int __cdecl Sim_10068b10(const float *box, float x, float y, float z);

#endif
