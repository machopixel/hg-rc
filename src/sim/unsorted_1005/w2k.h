/*
 * Wave 2 (letter k) additions to unsorted_1005: leaf functions around
 * 0x100526a9..0x1005b923. Names carry the original address because the code
 * alone does not say what the data means.
 */
#ifndef HG_SIM_UNSORTED_1005_W2K_H
#define HG_SIM_UNSORTED_1005_W2K_H

#include <stddef.h>
#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* With index < [0x1015dc64] and value < 8: stores value in the dword at +0x10 of the record 0x109e8b40[index] and returns 1; otherwise returns 0. */
int __cdecl Sim_100526a9(unsigned index, unsigned value);
/* 2 if index >= 16, else the dword at 0x109b7d54 + 0x6c * (dword at 0x109a864c + 0x54 * index). */
int __cdecl Sim_10052c30(unsigned index);
/* Both return their argument. */
uint32_t __cdecl Sim_10052ff0(uint32_t value);
uint32_t __cdecl Sim_10052ffb(uint32_t value);
/* Dword at 0x1013f140 = 1. */
void __cdecl Sim_1005571d(void);
/* The byte at 0x10164bc8 + index (index sign-extended from 16 bits). The upper half of the result is 0xffff for a negative index, else 0. */
uint32_t __cdecl Sim_1005572c(int index);
/* Returns 0. */
int __cdecl Sim_10057a43(void);
/*
 * Unless the dword at 0x1015e65c has bit 0x1000000 set (that dword is the fourth
 * of the four 0x1015e650..0x1015e65f copied), copies the four dwords at
 * 0x1015e650 to out when out is not NULL. Returns 0. a and b are not used.
 */
int __cdecl Sim_10057a58(uint32_t a, uint32_t b, uint32_t *out);
/*
 * For i in 0..16 with d = dword 0x10138d70[i] and k = dword obj+0xbdb: count =
 * dword at 0x10138fe8 + k * 0x24c + 4 * d is stored at obj+0xb67+4*i, and the
 * first count dwords of 0x10138db8 + k * 0x24c + 0x50 * d are copied to
 * obj+0x617 + 0x50 * i.
 */
void __cdecl Sim_1005b7e0(void *obj);
/* 8 gives 7, 9-10 give 3, 11-12 give 4, 13-16 give 2, anything else 0. */
int __cdecl Sim_1005b923(unsigned kind);

#endif
