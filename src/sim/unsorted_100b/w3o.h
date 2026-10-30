/*
 * Round 3 (letter o): a sorted pointer table search and the lookup built on it (0x100b42d5, 0x100baf3f, 0x100bda9f).
 */
#ifndef HG_SIM_UNSORTED_100B_W3O_H
#define HG_SIM_UNSORTED_100B_W3O_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif


/* Sim_100cc887 on the list head at 0x101a2184 + 0x88 * n. */
void __cdecl Sim_100b42d5(int n);

/*
 * Looks the two-dword key up in the table found with Sim_100bda9f(G[0x1017d604], arg, comparator 0x100b971d):
 * hash bucket (key[0] ^ key[1]) % table[0] of the pointer array at table+0x10, chain through +0x1c, entries
 * whose first 8 bytes equal the key. On a match: +8 += 1, +0x10/+0x14 = the 64-bit value returned by the
 * function pointer at 0x1017d60c (no arguments), *out = entry+0xc, returns entry+0x18. Otherwise 0.
 */
uint32_t __cdecl Sim_100baf3f(uint32_t *key, uint32_t arg, uint32_t *out);

/*
 * Binary search in the sorted pointer array: table+4 = count, table+8 = array. cmp(element, key)
 * returns <0 when the element sorts before the key, 0 on a match (the element is returned), else >0.
 * Returns 0 when not found.
 */
uint32_t __cdecl Sim_100bda9f(void *table, uint32_t key, int(__cdecl *cmp)(uint32_t element, uint32_t key));

#endif
