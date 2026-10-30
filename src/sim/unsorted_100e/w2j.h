/*
 * Wave 2, letter j: leaf functions of HGSIM.DLL around 0x100e0000..0x100f0000.
 */
#ifndef HG_SIM_UNSORTED_100E_W2J_H
#define HG_SIM_UNSORTED_100E_W2J_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Does nothing (an empty function). */
void __cdecl Sim_100e122a(void);
/* Dword 0x1017e830 = a, dword 0x1017e854 = b. */
void __cdecl Sim_100e2740(uint32_t a, uint32_t b);

#endif
