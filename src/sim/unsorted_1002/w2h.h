/*
 * Wave 2 functions of HGSIM.DLL around 0x10028000..0x1002e000 (letter h).
 * Names carry the original address because the code alone does not say what
 * the fields mean.
 */
#ifndef HG_SIM_UNSORTED_1002_W2H_H
#define HG_SIM_UNSORTED_1002_W2H_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Does nothing. */
void __cdecl Sim_10028289(void);
/* Sets the 16-bit field obj+4 to 0. */
void __cdecl Sim_10028a92(void *obj);
/* Returns 0; all four arguments are ignored. */
int __cdecl Sim_1002d244(uint32_t a, uint32_t b, uint32_t c, uint32_t d);
/* Sets obj[0x34] = state, then calls Sim_1008a37f(obj) when state is 4 and Sim_1008a220(obj) otherwise. */
void __cdecl Sim_1002d890(void *obj, int32_t state);

#endif
