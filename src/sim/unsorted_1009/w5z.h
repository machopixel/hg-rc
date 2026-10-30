/*
 * Round 5 (letter z): small functions of HGSIM.DLL at 0x10098710 and 0x10098a5d.
 * Calls of functions that are not converted yet go through their original
 * address (SIM_FN); the others call the converted C functions by name.
 */
#ifndef HG_SIM_UNSORTED_1009_W5Z_H
#define HG_SIM_UNSORTED_1009_W5Z_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Returns the original 0x10098721(0, 0). */
uint32_t __cdecl Sim_10098710(void);

/* Calls the original 0x1008a94d(v), then stores v at +0x20 of the block that dword 0x1015d1b8 points to. The first argument is not used. */
void __cdecl Sim_10098a5d(uint32_t unused, uint32_t v);

#endif
