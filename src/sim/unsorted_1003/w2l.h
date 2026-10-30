/*
 * Wave 2l: functions around 0x10037670..0x1003db30. Names carry the original
 * address because the code alone does not say what the objects are. Plain C,
 * original cdecl convention.
 */
#ifndef HG_SIM_UNSORTED_1003_W2L_H
#define HG_SIM_UNSORTED_1003_W2L_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Does nothing. */
void __cdecl Sim_10037670(void);
/*
 * Four masks at p+0 .. p+0xc: for each mask, p+0x10.. = number of set bits and p+0x20.. = number of clear
 * bits below the lowest set bit (0x20 for an empty mask).
 */
void __cdecl Sim_1003775f(uint32_t *p);
/* p[0..3] = a, b, c, d and then Sim_1003775f(p). */
void __cdecl Sim_1003772b(uint32_t *p, uint32_t a, uint32_t b, uint32_t c, uint32_t d);
/*
 * Finds the first of the [0x1015ea80] 0x7e-byte records at 0x109b9e20 whose dword +0x60 is `key`: releases each
 * non-zero object pointer in its four dwords from +0x50 with Sim_10009787 and clears them, sets dword +4 to -1 and
 * the short +0x7c to 1.
 */
void __cdecl Sim_1003b8e3(int key);
/* Reads the dword at +0x28 of the record pointer-table entry `index` when that entry is not NULL and does nothing with it. */
void __cdecl Sim_1003db30(int index);

#endif
