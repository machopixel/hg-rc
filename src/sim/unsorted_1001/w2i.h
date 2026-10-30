/*
 * Wave 2 (letter i) additions to unsorted_1001: records and small tables around 0x10012000..0x1001e000.
 * Plain C, original cdecl convention. Names carry the original address
 * because the code alone does not say what the data is for.
 */
#ifndef HG_SIM_UNSORTED_1001_W2I_H
#define HG_SIM_UNSORTED_1001_W2I_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Returns (a & 0xFFFF0000) | (b & 0xFFFF); c is ignored. */
uint32_t __cdecl Sim_10012ad9(uint32_t a, uint32_t b, uint32_t c);
/*
 * Sets the pair of 16-bit words at obj + 0x1a0 to (a, b) when the first one is
 * still 0; d is ignored.
 */
void __cdecl Sim_10015cf8(void *obj, int16_t a, int16_t b, uint8_t d);
/*
 * With index = (short)Sim_10052aaf(obj[8]): unless that is -1 calls
 * Sim_10015cf8(table 0x109e8b40[index], word at rec, word at obj + 0xf0,
 * byte at rec + 4). Returns 0.
 */
int __cdecl Sim_10015c0e(void *obj, void *rec);
/* Dword at 0x1015d184. */
uint32_t __cdecl Sim_10019d8c(void);
/*
 * tab: dword count, then count entries of 0x39 bytes from tab + 4, each
 * starting with a signed dword key, sorted ascending. key = a % b (signed).
 * Returns the entry with that key; else the entry before the first one with a
 * larger key (the last entry when it is the first one); else the last entry
 * visited; NULL for a NULL tab or no entries.
 */
void *__cdecl Sim_1001c075(int a, int b, void *tab);
/*
 * Same table layout: the index after `index` (wrapping at count) of the first
 * entry whose dword at +0xd is 0 or 1; -1 when tab is NULL or index is not in
 * -1 .. count - 1.
 */
int __cdecl Sim_1001db13(void *tab, int index);

#endif
