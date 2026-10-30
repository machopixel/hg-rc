/*
 * Round 5 (letter z): small functions of HGSIM.DLL around 0x100100a2 to 0x1001661f.
 * Calls of functions that are not converted yet go through their original
 * address (SIM_FN); the others call the converted C functions by name.
 */
#ifndef HG_SIM_UNSORTED_1001_W5Z_H
#define HG_SIM_UNSORTED_1001_W5Z_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Clears the dword at 0x1012ff58, then returns the original 0x1005264d(dword 0x1013bb40, a). The second argument is not used. */
uint32_t __cdecl Sim_100100a2(uint32_t a, uint32_t unused);

/* Returns 0 when Sim_1000fe60(4) is 0, else the result of the original 0x10016117(4, 2). */
int __cdecl Sim_10010acd(void);

/* Takes the mode that Sim_100109e7(dword 0x1015d100) gives; for a mode of 0 or more returns the original 0x10010699(mode, 0), else the mode (-1). */
int __cdecl Sim_10010b0b(void);

/* Calls Mono_Clear when dword 0x10a3ade8 is set, then Sim_10052ad6(i, 0) and Sim_1001661f(i) for i below dword 0x104737e0. */
void __cdecl Sim_10010f70(void);

/* Returns the original 0x10016d83(a, 0). */
uint32_t __cdecl Sim_1001661f(uint32_t a);

#endif
