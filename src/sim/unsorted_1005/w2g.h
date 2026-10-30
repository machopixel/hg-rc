/*
 * Wave 2g: an entry pool of HGSIM.DLL around 0x100570f1..0x100585b3.
 *
 * Entries are 0x4c bytes, from 0x109b16e0, [0x109b0eac] of them. A doubly
 * linked list (first [0x101668d0], last [0x101668d4]; next at +0, previous at
 * +4) holds some of them. A stack of entry numbers at 0x109b12c0 has its top
 * index in [0x109b0eb0]: a push needs top != count - 1, a pop needs top >= 0.
 * Field +0xc of an entry is the number the stack functions return; +0x44 is a
 * limit (-1 for none) and +0x48 a count that Sim_10058493 advances by
 * [0x1015dc80].
 */
#ifndef HG_SIM_W2G_1005_H
#define HG_SIM_W2G_1005_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Returns 2. */
int __cdecl Sim_100570f1(void);
/* Clears the four dwords at 0x1015e650. */
void __cdecl Sim_10057f53(void);
/* Appends the entry at the end of the list (does nothing for null). */
void __cdecl Sim_100581a0(uint8_t *entry);
/* Unlinks the entry from the list and clears its two links (does nothing for null, or when either end of the list is empty). */
void __cdecl Sim_1005820d(uint8_t *entry);
/*
 * When 0 <= number < [0x109b0eac] and the stack is not full: pushes `number`,
 * unlinks the entry from the list (Sim_1005820d) and returns it; otherwise
 * returns null.
 */
uint8_t *__cdecl Sim_100582d2(int number);
/* When the stack is not empty: appends the entry of the number on top to the list (Sim_100581a0), pops it and returns it; otherwise null. */
uint8_t *__cdecl Sim_1005833d(void);
/*
 * Adds [0x1015dc80] to the count (+0x48) of every listed entry whose limit
 * (+0x44) is not -1. The numbers (+0xc) of the ones whose count then exceeds
 * the limit are collected in the order of the list (buffer at 0x109b0ec0), and
 * each is given to Sim_100585b3.
 */
void __cdecl Sim_10058493(void);
/* When [0x10143fd8] is set and Sim_1005833d returns an entry: clears its count (+0x48) and returns its number (+0xc). Otherwise -1. */
int __cdecl Sim_10058575(void);
/* Sim_100582d2(number). */
uint8_t *__cdecl Sim_100585b3(int number);

#endif
