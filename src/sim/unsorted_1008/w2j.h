/*
 * Wave 2, letter j: leaf functions of HGSIM.DLL around 0x10080000..0x10090000.
 */
#ifndef HG_SIM_UNSORTED_1008_W2J_H
#define HG_SIM_UNSORTED_1008_W2J_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Does nothing (an empty function). */
void __cdecl Sim_100877a0(void);
/* Returns 0. */
int __cdecl Sim_100877b5(void);
/*
 * dst[0..3] = (src[0..3] << 16) / 319, 199, 319, 199 (signed, rounded toward
 * zero, each read just before it is written); returns dst. The original raises
 * a divide error when a quotient does not fit 32 bits (|src| >= 319 << 15 or
 * 199 << 15); this version does not.
 */
int32_t *__cdecl Sim_1008cfce(const int32_t *src, int32_t *dst);
/* Dwords +0 and +4 of the block that p[0] points to = 0. */
void __cdecl Sim_1008f64c(void *p);

#endif
