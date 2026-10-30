/*
 * Small helpers of HGSIM.DLL shared across the whole simulation: lookups in
 * the DLL's global tables, accessors of record fields, a ring of 128 table
 * values. Plain C with no knowledge of the hook layer. Every function keeps
 * the original's cdecl convention; names are Sim_<address> where the code
 * alone does not show what the function is for.
 *
 * The code targets the 32-bit DLL: pointers are 32 bits, so address
 * arithmetic with 32-bit offsets wraps exactly as the original's.
 */
#ifndef HG_SIM_UTIL_H
#define HG_SIM_UTIL_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* ---- lookups in global tables (util/lookup.c) ---- */

/* 0 when obj is null. Otherwise entry `index` of the 0x128-byte array at
 * *(obj+0x84) holds an index (+4) into the 0xd6-byte records at 0x10145508;
 * returns 1 when the dword at +0x86 of that record has any bit of `mask`. */
int __cdecl Sim_1000c749(void *obj, int index, unsigned mask);
/* Value of the first of `count` 8-byte (value, key) pairs whose key == key, else 0. */
uint32_t __cdecl Sim_10011b9e(short key, const void *pairs, short count);
/* Index i < dword 0x1015dc64 of the first record of pointer table 0x109e8b40
 * with +8 equal to that of the table entry selected by dword 0x1017b8c8 and
 * +0x10 == value; -1 if none (or that entry is null). */
int __cdecl Sim_10015c61(int value);
/* Group `index` of the 0x6c-byte table at 0x109b7d40 lists records of the pointer
 * table 0x109e8b40 (count at +4, indices from +0x1c). Copies those whose short at
 * +0x1c has none of the bits 6 (and 0x10 when flag is 0) to out[0..]; returns how
 * many. If dword +8 is set, list entry 4 is also tested and stored at out[4]. */
int __cdecl Sim_10016cb7(int index, uint32_t *out, int flag);
/* Value paired with `key` in the (key, value) pairs at *0x1017d214, searching
 * from the last of dword 0x1017d20c entries backwards; 0 if none. */
uint32_t __cdecl Sim_100b36d8(uint32_t key);
/* B[i] of the last i < dword 0x101c42a0 with A[i] == key, A = *0x1017d1f8, B = *0x1017d1f4; -1 if none. */
int __cdecl Sim_100b3839(uint32_t key);
/* Like Sim_100b3839 over dword 0x101a1168 entries, A = *0x1017d208, B = *0x1017d204; -1 for a negative key. */
int __cdecl Sim_100b38c5(int key);
/* Address of the 0x88-byte entry `index` of the table at 0x101a2128. */
void *__cdecl Sim_100b399c(int index);
/* obj has a count at +0x4f8 and a 0x128-byte array at +0x84. Returns 1 when
 * the count is non-zero and no entry has both +8 != -1 and +0x20 != 0.
 * obj must not be null (the original reads +0x4f8 regardless). */
int __cdecl Sim_100c3668(void *obj);
/* Reads the ushort at *0x10157590 + 2*(value & 0xff) (bits 16 and up of value
 * still take part), derives an index from it (adding value>>8 & 0xff to a
 * non-zero entry), stores 2 in dword 0x10157594 (1 when the entry is 0) and
 * returns the dword at base + (dword at base + 0x10 + 4*index). */
uint32_t __cdecl Sim_100f2cec(void *base, uint32_t value);

/* ---- ring of 128 table values (util/ring.c) ---- */

/* Next value of the table at 0x101cffc0, modulo m. Index in dword 0x1017cd38, wraps to 0 on reaching 127. */
int __cdecl Sim_1009c929(int m);
/* Next value of the table at 0x101cfce0. Index in dword 0x1017cd3c, wraps to 0 on reaching 127. */
uint32_t __cdecl Sim_1009c969(void);

/* ---- record fields (util/field.c) ---- */

void __cdecl Sim_10008c69(void *obj, uint32_t value);   /* obj+0x30 = value */
void __cdecl Sim_10009408(void *obj, unsigned short v); /* if obj: ushort obj+2 = v */
uint32_t __cdecl Sim_100096ee(void *obj);                /* return obj+0x28 */
void __cdecl Sim_100096f9(void *obj, uint32_t value);   /* obj+0x28 = value */
void __cdecl Sim_100a9b4c(void *obj, uint32_t value);   /* obj+0x8c = value, obj+0x94 |= 1 */
uint32_t __cdecl Sim_100a9b71(void *obj);                /* return obj+0x8c */
void __cdecl Sim_100a9cfb(void *obj, uint32_t *a, uint32_t *b, uint32_t *c); /* reads obj+0x70, +0x74, +0x78 */
void __cdecl Sim_100a9d21(void *obj, uint32_t *a, uint32_t *b, uint32_t *c); /* reads obj+0x30, +0x34, +0x38 */
void __cdecl Sim_100a9d47(void *obj, uint32_t a, uint32_t b, uint32_t c);    /* writes obj+0x30, +0x34, +0x38; obj+0x94 |= 1 */
void __cdecl Sim_100c274a(void *obj, unsigned char value); /* byte at (*(obj+0x54))+0x1e = value */
void __cdecl Sim_100c48c9(void *obj);                      /* clears dword +8 and byte +0x54 of *(obj+0x54) */
int __cdecl Sim_100cc8ba(int p);                          /* return p + 4 */
uint32_t __cdecl Sim_100cc6e0(void);                      /* return dword 0x1017e5b8 */

/* ---- append / write into tables (util/record.c) ---- */

void __cdecl Sim_1000711c(short index, unsigned short value);
void __cdecl Sim_10009136(void *obj, uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4,
                          uint32_t a5, uint32_t a6, uint32_t a7, uint32_t a8, uint32_t a9,
                          uint32_t a10, uint32_t a11);
void __cdecl Sim_10009278(void *obj, void *buf, unsigned char byte);
void __cdecl Sim_100a88db(void);
int __cdecl Sim_100ca350(uint32_t a, uint32_t b, uint32_t c, uint32_t d);

#endif
