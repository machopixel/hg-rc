/*
 * Wave 2 functions of HGSIM.DLL around 0x10007000..0x10009500 (letter h).
 * Names carry the original address because the code alone does not say what
 * the fields mean.
 */
#ifndef HG_SIM_UNSORTED_1000_W2H_H
#define HG_SIM_UNSORTED_1000_W2H_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Resets two record tables: 512 groups of 32 records of 0x18 bytes at 0x10a51ac0
 * (byte +0, word +0x10 and dword +0x14 cleared) and 512 records of 0xe bytes at
 * 0x10ac1b00 (word +0 = 0xffff, words +2/+4/+6 = 0, word +8 = 0xfffe, dword +0xa =
 * 0xffffffff), then clears the dwords 0x1012da30, 0x1015cfe0, 0x1015cfe4 and 0x1015cfe8.
 */
void __cdecl Sim_10007020(void);
/* Does nothing. */
void __cdecl Sim_100071f1(void);
/* Dword at 0x1015d09c. */
uint32_t __cdecl Sim_100089a0(void);
/* The 16-bit field at obj+0x24, zero-extended. */
uint32_t __cdecl Sim_1000945e(const void *obj);
/*
 * With link = dword obj+0x30 (nothing happens when 0): stores the signed 16-bit
 * link+4 in *a and the signed 16-bit link+6 in *b, each only when the pointer is not 0.
 */
void __cdecl Sim_100094df(const void *obj, int32_t *a, int32_t *b);

#endif
