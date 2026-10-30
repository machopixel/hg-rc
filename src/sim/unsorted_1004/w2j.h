/*
 * Wave 2, letter j: leaf functions of HGSIM.DLL around 0x10040000..0x10050000.
 */
#ifndef HG_SIM_UNSORTED_1004_W2J_H
#define HG_SIM_UNSORTED_1004_W2J_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * obj is a record with an id byte at +4 and a table-0x109e8b40 index byte at +2.
 * Unless the id equals dword 0x1017b8c8 and when that table entry exists and
 * has dword +0x14 == 3: entry+0xc = id, entry+0x18 = 0.
 */
void __cdecl Sim_100405e6(const void *obj);
/* Copies the 6 dwords at src to dst and returns 1; 0 when either is null or src+0x18 is 0. */
int __cdecl Sim_100449fc(void *dst, const void *src);
/*
 * p: +0 time of the last call, +4 target, +8 value, +0xc duration. Moves the
 * value toward the target by (target - value) * (time - p[0]) / duration (signed
 * division; the original raises a divide error for a duration of 0), reaching
 * the target when that step is at least the distance. p[0] = dword 0x1015dc78.
 * Returns the new value.
 */
int32_t __cdecl Sim_10047af2(void *p);
/* Returns 3. */
int __cdecl Sim_1004bc5b(void);
/* value >> 16, arithmetic. */
int __cdecl Sim_1004d27a(int value);
/* Dword 0x1015f340 = 0, byte 0x1015f230 = 0, dword 0x1015f348 = 8. */
void __cdecl Sim_1004e1a1(void);
/*
 * Adds the entry (prefix, byte) to a string table of at most 0x1000 entries:
 * dword 0x1015f350 is the count, 0x1015f35c a 0x26f5-dword hash table (0xffff =
 * empty, probing by 0x7f7), 0x1015f354 the bytes and 0x1015f358 the prefixes
 * of the entries. Returns the new entry's number, or 0xffff when the table is full.
 */
uint32_t __cdecl Sim_1004e4c2(uint32_t prefix, uint32_t byte);
/*
 * Empties that table (count 0, every slot 0xffff) and adds the entries
 * (0xffff, i) for i < (1 << bits) + 2.
 */
void __cdecl Sim_1004e61a(uint32_t bits);

#endif
