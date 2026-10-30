/*
 * Wave 3S, original 0x100335fd..0x10034036: distances to the 0x44-byte entries of the table at
 * 0x109e4cc4 (count at 0x109e4cc0; position floats at entry+0x20, +0x24, +0x28). Plain C,
 * original cdecl convention. The original leaves the unrounded x87 value in st(0).
 */
#ifndef HG_SIM_UNSORTED_1003_W3S_H
#define HG_SIM_UNSORTED_1003_W3S_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Squared distance from the entry's position to (x, y, z); the float at 0x1011131c for a NULL entry. Returned in st(0) as the 53-bit sum, callers round it to float where the original did. */
double __cdecl Entry_DistSq(const void *entry, float x, float y, float z);
/* 1 when (a - p) . (b - p) < the float at 0x1011131c (or is NaN), else 2, p being the entry's position. */
int __cdecl Sim_1003366e(const void *entry, float ax, float ay, float az, float bx, float by, float bz);
/*
 * Index of the entry (0..count) with the least Entry_DistSq: starts at the first entry for which
 * Sim_1003356a is 0 and then examines the following ones. Quirk kept: the index recorded for an
 * examined entry is one less than its real index, and the entry at index `count` is examined too.
 * -1 when the table is empty.
 */
int __cdecl Sim_10033d75(float x, float y, float z);
/*
 * Of the n entry numbers in list, the one with the least Entry_DistSq to (x, y, z): -1 for a NULL list or
 * n < 1, list[0] for n == 1. Quirk kept: the loop compares list[0..n-2]; the last number is never looked at.
 */
int __cdecl Sim_10034036(const int32_t *list, int n, float x, float y, float z);

#endif
