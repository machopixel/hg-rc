/*
 * Wave 2 (batch f) functions of HGSIM.DLL in 0x100axxxx: reads of the first
 * dword and of +8 of a structure. Plain C, original cdecl convention.
 */
#ifndef HG_SIM_UNSORTED_100A_W2F_H
#define HG_SIM_UNSORTED_100A_W2F_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Follows the pointer in the first dword until it is 0; returns the last node reached (p itself when its first dword is 0). */
void *__cdecl Sim_100aa7cd(void *p);
/* Returns the first dword of p. */
uint32_t __cdecl Sim_100aa7e7(const void *p);
/* Returns the dword at p+8. */
uint32_t __cdecl Sim_100aa7fc(const void *p);

#endif
