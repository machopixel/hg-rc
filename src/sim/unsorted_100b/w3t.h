/*
 * Round 3 (letter t) functions of HGSIM.DLL in 0x100bxxxx. Plain C, original
 * cdecl convention.
 *
 * A sorted array: the struct is {int capacity, int count, int *items}. The
 * insertion does a binary search with a comparison function of the caller
 * that takes (item, value) and returns a negative number when the item sorts
 * before value, zero when they are equal and a positive number after it.
 */
#ifndef HG_SIM_UNSORTED_100B_W3T_H
#define HG_SIM_UNSORTED_100B_W3T_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

typedef struct SortedArray {
    int32_t capacity;
    int32_t count;
    uint32_t *items;
} SortedArray;

/*
 * Inserts value in front of the first item the comparison function (which
 * returns a float in st(0)) finds equal, or after the last item that sorts
 * before it. Returns 0 when the array is full, else 1.
 */
int __cdecl Sim_100bdb28(SortedArray *array, uint32_t value, double(__cdecl *compare)(uint32_t item, uint32_t value));

/*
 * Same search with an int comparison function. Returns 0 when the array is
 * full, the item that compared equal (nothing is inserted) or value after
 * inserting it at the last position the search looked at (not corrected after
 * the search as in Sim_100bdb28).
 */
uint32_t __cdecl Sim_100bdd77(SortedArray *array, uint32_t value, int(__cdecl *compare)(uint32_t item, uint32_t value));

#endif
