/*
 * Wave 2l: small table functions around 0x10020456..0x1002997e. Names carry
 * the original address because the code alone does not say what the tables
 * hold. Plain C, original cdecl convention.
 */
#ifndef HG_SIM_UNSORTED_1002_W2L_H
#define HG_SIM_UNSORTED_1002_W2L_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * n = number - 1, v = 0 for 0xffff, else value + 1 below 9 and value - 1 from 9 up. When n is 0..4, n is not 3
 * and v is 0..0x13: the dword at 0x10133700 + 8 * n = v.
 */
void __cdecl Sim_10020456(int number, int value);
/* The dword at 0x109f1484 + 8 * index for index 0..9, else 0. */
uint32_t __cdecl Sim_100272a0(int index);
/* dword p+0xc = value. */
void __cdecl Sim_10028973(void *p, uint32_t value);
/* 1 when none of the 0x80 entries of 0x30 bytes at 0x109eeb04 has a zero dword at +0 (no free entry), else 0. */
int __cdecl Sim_10028b50(void);
/* Index of the first of those 0x80 entries whose dword +0 is `key`, or -1. */
int __cdecl Sim_1002997e(int key);

#endif
