/*
 * Round 5 (letter z): small functions of HGSIM.DLL around 0x100203b8 to 0x1002cdb0.
 * Calls of functions that are not converted yet go through their original
 * address (SIM_FN); the others call the converted C functions by name.
 */
#ifndef HG_SIM_UNSORTED_1002_W5Z_H
#define HG_SIM_UNSORTED_1002_W5Z_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Returns the original 0x10020256(2, 0). The argument is not used. */
uint32_t __cdecl Sim_100203b8(uint32_t unused);

/* Returns the original 0x100246b3(a0, 0, a1, ... a7, 0, 0, 0). */
uint32_t __cdecl Sim_10024616(uint32_t a0, uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4, uint32_t a5, uint32_t a6, uint32_t a7);

/* Returns the original 0x100282d4(0, a, b, 0). */
uint32_t __cdecl Sim_1002851b(uint32_t a, uint32_t b);

/* When dword 0x109f13f0 is not 0, calls the original 0x10027fb8 with it and returns 1; else returns 0. */
int __cdecl Sim_10028534(void);

/*
 * Calls the original 0x1002ce45(obj, f1, f2, f3, &r, &b, &c) (f1 to f3 are float bit patterns) and
 * returns what it stored in r (the fifth argument).
 */
uint32_t __cdecl Sim_1002cdb0(uint32_t obj, uint32_t f1, uint32_t f2, uint32_t f3);

#endif
