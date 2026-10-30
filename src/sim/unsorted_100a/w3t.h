/*
 * Round 3 (letter t) functions of HGSIM.DLL in 0x100axxxx. Plain C, original
 * cdecl convention.
 */
#ifndef HG_SIM_UNSORTED_100A_W3T_H
#define HG_SIM_UNSORTED_100A_W3T_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Adds dx, dy, dz to the floats at obj+0x30, +0x34, +0x38 and sets bit 0 of the dword obj+0x94. */
void __cdecl Sim_100a9d7c(void *obj, float dx, float dy, float dz);

/*
 * size is {width, height}: stores height * 1.3333334 / width in the float
 * 0x101c64e0 and in the float at +0x44 of the camera (pointer at 0x1014cae0).
 */
void __cdecl Camera_SetAspect(const int32_t *size);

#endif
