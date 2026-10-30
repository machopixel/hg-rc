/*
 * Round 11: game functions of HGSIM.DLL around 0x10030000..0x1003ffff, converted one at a time.
 * Calls of functions that are not converted yet go through their original address (SIM_FN); methods of the sound
 * objects are COM calls (__stdcall) through the object's table.
 */
#ifndef HG_SIM_UNSORTED_1003_W11_H
#define HG_SIM_UNSORTED_1003_W11_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#define __stdcall
#endif

/* Calls method +0x44 of the sound object obj with value (nothing when obj is null). */
void __cdecl Sim_10039708(uint32_t *obj, uint32_t value);

/* Sound shutdown: when *slot holds an object, its Release (method +8), the slot cleared and CoUninitialize;
 * returns 1. */
int __cdecl Sim_1003a548(uint32_t **slot);

/* With the sound system [0x1015d0a4] on (+0xc) and a unit: the block 0x109e3720 gets the unit's position +0x58..
 * and its record's (+0x28) velocity +0x548..; it goes to the 3D object idx (+0x114 + idx * 4) through method +0x30
 * (block, 1); a failure code is named by Sound_ErrorName. */
void __cdecl Sim_10039931(int idx, const uint8_t *unit);

/* With the sound system on: the listener block 0x109e36e0 gets the camera position ([0x1014cae0]), the own unit's
 * velocity (record +0x548..), the front direction (Sim_10084766 of the camera heading and of heading + 90) and
 * the up vector (0, 1, 0); it goes to the listener (+0xc) through method +0x28 (block, 1), then method +0x44
 * commits; failures are named by Sound_ErrorName. */
void __cdecl Sim_100397e7(void);

/* Fetches the 55 functions of the network library (module [0x1015eb68]) by name (0x1013a794..) into
 * 0x1015ea8c..0x1015eb64 with GetProcAddress, in that order. */
void __cdecl Sim_1003be30(void);

#endif
