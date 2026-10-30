/*
 * Wave 2l: small functions around 0x10073027..0x1007dc1f. Names carry the
 * original address because the code alone does not say what the objects are.
 * Plain C, original cdecl convention.
 */
#ifndef HG_SIM_UNSORTED_1007_W2L_H
#define HG_SIM_UNSORTED_1007_W2L_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* The dword at +0x508 of the object at obj+0x28. */
uint32_t __cdecl Sim_10073027(const void *obj);
/* 1 when the dword at p+0xc is 0. */
int __cdecl Sim_10076676(const void *p);
/*
 * Copies ten dwords: 0x10179d38 <- 0x101c49dc, 0x10179d3c <- 0x101c49e0, 0x10179d50 <- 0x101c459c, 0x10179d54 <-
 * 0x101c45a0, 0x10179d40 <- 0x101c5138, 0x10179d44 <- 0x101c513c, 0x10179d48 <- 0x101c5144, 0x10179d58 <-
 * 0x101c47dc, 0x10179d5c <- 0x101c47e0, 0x10179d60 <- 0x101c47e8.
 */
void __cdecl Sim_1007dc1f(void);

#endif
