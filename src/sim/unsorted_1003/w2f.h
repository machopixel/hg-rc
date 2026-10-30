/*
 * Wave 2 (batch f) functions of HGSIM.DLL in 0x1003xxxx. Plain C, original
 * cdecl convention.
 */
#ifndef HG_SIM_UNSORTED_1003_W2F_H
#define HG_SIM_UNSORTED_1003_W2F_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* G32(0x109e3d80) = b (the first argument is unused). */
void __cdecl Sim_10035599(uint32_t a, uint32_t b);
/* Does nothing. */
void __cdecl Sim_100386d0(void);
/* Of the 0x128-byte records at obj+0x84, sets the dwords +4 and +8 of the one numbered by the dword obj+0x5fb to -1. */
void __cdecl Sim_10038a9c(void *obj);
/* Calls Sim_100751e4 with the keys 1 to 8 in turn. */
void __cdecl Sim_1003cf81(void);

#endif
