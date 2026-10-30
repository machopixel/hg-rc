/*
 * Wave 2 (batch f) functions of HGSIM.DLL in 0x1009xxxx. Plain C, original
 * cdecl convention.
 */
#ifndef HG_SIM_UNSORTED_1009_W2F_H
#define HG_SIM_UNSORTED_1009_W2F_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Returns Sim_100585c4(-1), the dword 0x10143fd8 (the argument is unused). */
int __cdecl Sim_10098849(int unused);
/* Calls Sim_100585c4(b), then stores b at +0x2c of the structure pointed to by the dword 0x1015d1b8. Returns b. The first argument is unused. */
int __cdecl Sim_1009885e(int unused, int b);
/* 1 when the dword 0x1015d1ac is 4, else 0. */
int __cdecl Sim_1009b2eb(void);

#endif
