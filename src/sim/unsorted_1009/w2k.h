/*
 * Wave 2 (letter k) additions to unsorted_1009: leaf functions around
 * 0x100987e6..0x1009e5b9 of HGSIM.DLL. Names carry the original address
 * because the code alone does not say what the data means.
 */
#ifndef HG_SIM_UNSORTED_1009_W2K_H
#define HG_SIM_UNSORTED_1009_W2K_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* The dword at +0x64 of the record the pointer at 0x1015d1b8 points to. The argument is not used. */
uint32_t __cdecl Sim_100987e6(uint32_t unused);
/* Does nothing. */
void __cdecl Sim_10098e39(void);
/* Returns -1. */
int __cdecl Sim_10098e3e(void);
/* Returns 0x400. */
int __cdecl Sim_1009ca28(void);
/* The dword at 0x1017d060. */
uint32_t __cdecl Sim_1009e5b9(void);

#endif
