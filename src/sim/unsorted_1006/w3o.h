/*
 * Round 3 (letter o): 0x10068e1d.
 */
#ifndef HG_SIM_UNSORTED_1006_W3O_H
#define HG_SIM_UNSORTED_1006_W3O_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif


/* If the three output pointers are all non-NULL: *outX = ax + bx, *outY = ay + by, *outZ = az + bz (floats). */
void __cdecl Sim_10068e1d(float ax, float ay, float az, float bx, float by, float bz, float *outX, float *outY, float *outZ);

#endif
