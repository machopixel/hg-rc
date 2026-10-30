/*
 * Wave 2 (letter k) additions to unsorted_1004: leaf functions around
 * 0x10044311..0x1004e582. Names carry the original address because the code
 * alone does not say what the fields mean.
 */
#ifndef HG_SIM_UNSORTED_1004_W2K_H
#define HG_SIM_UNSORTED_1004_W2K_H

#include <stdint.h>
#include "fns.h"

/* Dword at 0x1015ee98. */
uint32_t __cdecl Sim_10044311(void);

/*
 * Request setter. kind 0..10 fills one of six records of the data at 0x1013af48
 * (kind 0: +0x0 = 1, +4 = a, +8 = b at 0x1013af48; kinds 1-3: 0x1013af60 = 1,
 * +4 = a, +8 = 0x32 / 0x3c / 0x50; kinds 4-6 the same at 0x1013af54; kind 7, 8,
 * 9, 10: = 1, a, b at 0x1013af6c, 0x1013af78, 0x1013af84, 0x1013af90).
 * Any other kind does nothing.
 */
void __cdecl Sim_10045f3b(unsigned kind, uint32_t a, uint32_t b);

/* Returns 1. */
int __cdecl Sim_1004c5b7(void);

/* Whether bit 2, 3, 6, 7, 9, 10, 11 of a flag word is set (the bits 0, 1, 4, 5 are Sim_1004cd8d.. in fns.h). */
int __cdecl Sim_1004cdaf(unsigned flags); /* 0x004 */
int __cdecl Sim_1004cdc0(unsigned flags); /* 0x008 */
int __cdecl Sim_1004cdf3(unsigned flags); /* 0x040 */
int __cdecl Sim_1004ce04(unsigned flags); /* 0x080 */
int __cdecl Sim_1004ce17(unsigned flags); /* 0x200 */
int __cdecl Sim_1004ce2a(unsigned flags); /* 0x400 */
int __cdecl Sim_1004ce3d(unsigned flags); /* 0x800 */

/*
 * The three functions below start from the dword at base + row * 0xaa (base
 * 0x101116be, 0x101116ba, 0x101116a6) and add one dword of the data for each
 * of the flag bits 0, 1, 2, 3, 4, 5, 6, 7, 9, 10, 11 that is set (bit 8 is not
 * looked at).
 */
int __cdecl Sim_1004ce64(int row, unsigned flags);
int __cdecl Sim_1004cfb5(int row, unsigned flags);
int __cdecl Sim_1004d106(int row, unsigned flags);

/* Number of bits of n - 1 (at least 1) for n >= 1; 0 for n == 0. */
int __cdecl Sim_1004db86(unsigned n);

/*
 * Hash lookup. key 0xffff returns byte & 0xff. Otherwise probes the head table
 * (dword pointer at 0x1015f35c, 9973 entries) from ((byte & 0xff) << 8 ^ key) %
 * 9973, stepping by 0x7f7 (mod 9973), up to an entry of 0xffff (returns 0xffff).
 * An entry is an index e: it is the answer when keys[e] (dword pointer at
 * 0x1015f358) == key and the byte at 0x1015f354 + e == byte & 0xff.
 */
unsigned __cdecl Sim_1004e582(unsigned key, unsigned byte);

#endif
