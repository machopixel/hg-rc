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

/* Fetches the 55 functions of the network library (module [0x1015eb68]) by name (0x1013a794..) into
 * 0x1015ea8c..0x1015eb64 with GetProcAddress, in that order. */
void __cdecl Sim_1003be30(void);

#endif
