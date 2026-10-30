/*
 * Round 3 functions of HGSIM.DLL in 0x100bxxxx (letter p).
 */
#ifndef HG_SIM_UNSORTED_100B_W3P_H
#define HG_SIM_UNSORTED_100B_W3P_H

#include <stdint.h>

#include "../simmem.h"

/*
 * Stores nine floats at 0x10150800..0x10150820: the first three arguments, the next three times the float at
 * 0x1012babc (rounded as double, then to float), the last three. Integers convert with plain rounding to float.
 */
void __cdecl Sim_100b3fd5(int a, int b, int c, int d, int e, int f, int g, int h, int i);

/* Sorted array of dwords: dword +0 capacity, +4 count, +8 pointer to the dwords. */
typedef struct SortedList100b {
    int32_t capacity;
    int32_t count;
    int32_t *items;
} SortedList100b;

/*
 * Inserts `value` into the list keeping it ordered by compare(item, value) (negative when the item sorts first;
 * the position comes from a binary search, an equal item is inserted before the one found). Returns 0 when the list
 * is full, else 1.
 */
int __cdecl Sim_100bdc5d(SortedList100b *list, int32_t value, int(__cdecl *compare)(int32_t item, int32_t value));

#endif
