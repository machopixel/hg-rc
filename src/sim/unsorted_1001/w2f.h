/*
 * Wave 2 (batch f) functions of HGSIM.DLL in 0x1001xxxx. Plain C, original
 * cdecl convention.
 */
#ifndef HG_SIM_UNSORTED_1001_W2F_H
#define HG_SIM_UNSORTED_1001_W2F_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* With the 0x13f-byte records at 0x1047383a + 0x4fc3a * [0x1013bb40], sets the dword at +2 of record n to 0x10000; does nothing for n == 0. */
void __cdecl Sim_10010804(int n);
/* Returns a with the low 16 bits cleared (the two other arguments are unused). */
uint32_t __cdecl Sim_10013902(uint32_t a, uint16_t b, uint16_t c);
/*
 * obj+0x180 = the two words at obj+0x1a6 (low half) and obj+0x1aa (high half);
 * obj+0x198 (a word, the number of values at +0x180) becomes 1 when it was 0.
 * Returns 1. The second argument is unused.
 */
int __cdecl Sim_10013e3c(void *obj, int unused);
/* 1 -> 2, 2 -> 1, anything else -> 0. */
int __cdecl Sim_10016a4a(int v);
/* Returns 0. */
int __cdecl Sim_1001f4e4(void);

#endif
