/*
 * Wave 2l: tiny functions around 0x100a78c3..0x100aa7f1. Names carry the
 * original address because the code alone does not say what the values are.
 * Plain C, original cdecl convention.
 */
#ifndef HG_SIM_UNSORTED_100A_W2L_H
#define HG_SIM_UNSORTED_100A_W2L_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* dword 0x1017d15c = a, dword 0x1017d160 = b. */
void __cdecl Sim_100a78c3(uint32_t a, uint32_t b);
/* Does nothing. */
void __cdecl Sim_100a9012(void);
/* The dword at p+4. */
uint32_t __cdecl Sim_100aa7f1(const void *p);

#endif
