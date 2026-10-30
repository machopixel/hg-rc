/*
 * Wave 2 functions of HGSIM.DLL around 0x10076000..0x1007e000 (letter h).
 * Names carry the original address because the code alone does not say what
 * the fields mean.
 */
#ifndef HG_SIM_UNSORTED_1007_W2H_H
#define HG_SIM_UNSORTED_1007_W2H_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Walks the list at [0x1015dc14] (next pointer at +0x14) to the first node whose
 * dword +0 equals key; returns its dword +0xc, or 0 when there is none.
 */
uint32_t __cdecl Sim_1007655f(uint32_t key);
/*
 * a[0] is an index and a[1] << 8 a kind. Kind 0x200: the object 0x109e8b40[a[0]], kind 0x400:
 * the record 0x109e9ba0 + 0x50 * a[0]. In both, when its link dword (+0x150 or +0x4c
 * respectively) has neither bit 31 nor bit 30 and its low word (signed) is an index
 * in 0..0x18 below the signed dword [0x1015dc64] of a non-NULL object whose dword +8
 * equals b, sets bit 30 of the link dword and returns 1. Returns 0 otherwise.
 */
int __cdecl Sim_10079c86(const uint8_t *a, int32_t b);
/* Dword 0x1037305c = signed dword [0x1015dc78] / 0xb5. */
void __cdecl Sim_1007c884(void);
/* Returns 1. */
int __cdecl Sim_1007d8b0(void);
/* Returns 0. */
int __cdecl Sim_1007dba3(void);

#endif
