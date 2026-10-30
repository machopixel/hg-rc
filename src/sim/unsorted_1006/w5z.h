/*
 * Round 5 (letter z): small functions of HGSIM.DLL at 0x10061b62 and 0x1006535f.
 * Calls of functions that are not converted yet go through their original
 * address (SIM_FN); the others call the converted C functions by name.
 */
#ifndef HG_SIM_UNSORTED_1006_W5Z_H
#define HG_SIM_UNSORTED_1006_W5Z_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Returns the original 0x100d3510(0x11, 0x16a, 1). */
uint32_t __cdecl Sim_10061b62(void);

/* Returns the original 0x1006519d(a + dword at p+4, b + dword at p+8, c). */
uint32_t __cdecl Sim_1006535f(uint32_t a, uint32_t b, uint32_t c, const void *p);

#endif
