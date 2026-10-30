/*
 * Wave 2l: small functions around 0x10010124..0x1001db6a. Names carry the
 * original address because the code alone does not say what the objects are.
 * Plain C, original cdecl convention.
 */
#ifndef HG_SIM_UNSORTED_1001_W2L_H
#define HG_SIM_UNSORTED_1001_W2L_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* When neither is NULL: dword dst+0xc = dword src+8. */
void __cdecl Sim_10010124(const void *src, void *dst);
/*
 * Over the first [0x1015dc64] records of the pointer table 0x109e8b40: for each record whose dword +8 is `group`
 * and whose unsigned short +0x1aa has none of the bits 0xf000, has bit 0x200 and has 0xff-masked low byte equal to
 * `low`, that short becomes (value | 0x200) truncated to 16 bits.
 */
void __cdecl Sim_100128dd(int group, int low, unsigned value);
/* The original returns its first argument with the low 16 bits cleared; the other two arguments are not used. */
uint32_t __cdecl Sim_10013040(uint32_t value, uint16_t unused1, uint16_t unused2);
/* p = dword obj+0x28; dword obj+0x23e = dword p+0x588; dword p+0x58c = 2. */
void __cdecl Sim_10017aa9(void *obj);
/* Does nothing. */
void __cdecl Sim_1001db6a(void);

#endif
