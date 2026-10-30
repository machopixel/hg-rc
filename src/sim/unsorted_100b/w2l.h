/*
 * Wave 2l: functions around 0x100b534b..0x100bf23f: releasing the object of a
 * 0x88-byte table entry, a string cut, and the choice of a value from a
 * 0x12-byte-record table with Sim_1009c929. Names carry the original address
 * because the code alone does not say what the records are. Plain C, original
 * cdecl convention.
 */
#ifndef HG_SIM_UNSORTED_100B_W2L_H
#define HG_SIM_UNSORTED_100B_W2L_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Entry `index` of the 0x88-byte table at 0x101a2100: when its dword +0x28 is non-zero, Sim_10009787 of it and the dword is cleared. */
void __cdecl Sim_100b534b(int index);
/* Returns the dword 0x1015084c. The original takes one argument and ignores it. */
uint32_t __cdecl Sim_100b5bbb(uint32_t unused);
/* Ends the string at the first space, tab or NUL: writes a NUL there. */
/*
 * table: short count at +0, dword pointer at +4 to that many records of 0x12 bytes whose first short is a key.
 * Index of the first record whose key equals the short `key`, or 0xffff. The original returns a 16-bit result and
 * leaves the upper half of eax as it happens to be: the upper half of the record pointer when found, the sign of
 * the count (0 or 0xffff) when not; this version reproduces it.
 */
uint32_t __cdecl Sim_100bf23f(const void *table, int key);
/*
 * table = dword obj+0x27c, with a short count at +0 and a pointer at +4 to records of 0x12 bytes (a key short at +0,
 * a short n at +2, n shorts from +4). With the short obj+0x1fa equal to -1: the key short of record
 * Sim_1009c929(count - short table+2), taken as a 16-bit index. Otherwise the record is the one that
 * Sim_100bf23f(table, short obj+0x1fa) finds (record 0 if none) and the result is its short number
 * Sim_1009c929(n) from +4. The original returns a 16-bit value; the upper half of eax is whatever it left
 * (the upper half of the table pointer in the first case, that of the Sim_1009c929 result in the second) and
 * is reproduced here.
 */
uint32_t __cdecl Sim_100bf0b8(const void *obj);

#endif
