/*
 * Wave 2 functions of HGSIM.DLL around 0x10010000..0x10020000 (letter h).
 * Names carry the original address because the code alone does not say what
 * the fields mean.
 */
#ifndef HG_SIM_UNSORTED_1001_W2H_H
#define HG_SIM_UNSORTED_1001_W2H_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Dword table at 0x1012ff58 for i < 8 (signed), 7 otherwise. */
int32_t __cdecl Sim_10010063(int32_t i);
/* Sim_10052632 of the dword at 0x1013bb40; the argument is ignored. */
uint32_t __cdecl Sim_10010088(int32_t ignored);
/*
 * obj[0x170] points to an array of pointers, obj[0x174 + 4 * k] (k = 2, 1, 0)
 * to sets: word count at +0, pointer to (key, n) word pairs at +2, pointer to
 * 12-byte items at +6. For each set whose pair list has key == the signed word
 * obj[0x1a6], the first n items after the items of the earlier pairs (items stop at
 * a 0 first word) are appended to the pointer array unless an item with the same
 * words +0 and +2 is already in it. The array is ended by a 0 pointer after each
 * append, and its first entry is cleared first.
 * When a set has count 0 the original tests a leftover stack slot; this version
 * keeps the flag of the previous set instead (0 at the start).
 */
void __cdecl Sim_100119a7(void *obj);
/* a with its low 16 bits cleared; b and c are ignored. */
uint32_t __cdecl Sim_10013920(uint32_t a, int16_t b, int16_t c);
/* Returns 1; both arguments are ignored. */
int __cdecl Sim_10013e0a(int a, int b);
/*
 * Looks through the object table 0x109e8b40 (count in the signed dword
 * 0x1015dc64, index kept in 16 bits) for an object whose word +0x1c has none of
 * the bits 6 (mask 6), whose word +0x1aa is the signed word obj[4] with bit 0x200 set, for
 * which Sim_10052c60(object[4], that word) is 0 and whose word +0x1a6 is 3. Returns
 * object[4] | 0x200, or 0xffff. The caller uses only the low 16 bits; the high
 * half of the "not found" result is whatever the original left in eax and is
 * reproduced.
 */
uint32_t __cdecl Sim_10013f2a(const void *obj);
/*
 * a is a number (only the low byte is used), b = (kind << 8) | index with kind 0x100
 * (table at 0x109a8664, stride 0x54), 0x200 (object table 0x109e8b40, words +0x1c/+0x1e)
 * or 0x400 (table at 0x109e9ba0, stride 0x50), c a request code. Returns 1 or 0 as
 * decided by the bit masks in those records; with c == 0x4000 it tests and clears
 * flag 0x40000000 of dword +0x150 (kind 0x200) or +0x4c of the 0x109e9ba0 record
 * (kind 0x400). Codes other than 1, 2, 4, 8, 0x100, 0x1000, 0x4000 make the original
 * read an uninitialised stack word; this version treats it as 0.
 */
int __cdecl Sim_100152fc(unsigned a, unsigned b, int c);
/* Clears dword 0x10a38a40 and copies the 12 dwords at 0x101302e8 to 0x10a38a44; returns 1. */
int __cdecl Sim_10015bc9(void);
/*
 * Fills obj[0x174 + 4 * slot] (slot and sel are signed 16-bit): 0 when sel is -1,
 * otherwise dword 0x10a38a40[w] where w is the signed word in one of the word
 * tables at 0x10130cd8..0x10130e58 (64 bytes each, chosen by the dword obj[0], 1..10 with 7 doing
 * nothing) at sel * 4 + 2 * (obj[4] != other).
 */
void __cdecl Sim_10016a6b(void *obj, int16_t slot, int16_t sel, int16_t other);
/*
 * 0 for NULL or when obj[0] is not 1 or 9, when obj[0x14] is 1, or when obj[0x14]
 * is 3 and obj[0xc] equals [0x1017b8c8]. With r = obj[0x28]: 0 unless word r+0x562
 * has one of the bits 0x60e; 1 when it lacks bit 8; otherwise sets byte 0x55 of
 * obj[0x54] to 1 unless word r+0x560 has bit 0x20, and returns 0.
 */
int __cdecl Sim_10017a05(const void *obj);
/*
 * 1.2's replacement (1.2 0x10010cd2): may the unit act this tick? 1 for any unit that is not a gear (+0 1 or 9)
 * driven here (+0x14 2, or 3 with +0xc the player); for those 0 while its jets are cut (unit +0x560 bit 0x20) or it
 * is busy (+0x562 & 0x60e), and then, busy with bit 8 and the jets not cut, control byte +0x55 = 1. The tests
 * 0x10010bb4 (busy), 0x10010bf4 (jets cut), 0x10010c72 (bit 8) and the setter 0x10010cb0 are 1.2's own functions.
 */
int __cdecl Sim12_10010bb4(const void *obj);
int __cdecl Sim12_10010bf4(const void *obj, int unused);
int __cdecl Sim12_10010c72(const void *obj);
void __cdecl Sim12_10010cb0(const void *obj);
int __cdecl Sim12_10010cd2(const void *obj);
/* Returns 0x2c. */
int __cdecl Sim_10019d96(void);
/* Returns 0. */
int __cdecl Sim_1001f4dd(void);

#endif
