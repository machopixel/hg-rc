/*
 * Round 6 (letter j): functions of the C runtime that is linked into HGSIM.DLL (0x10101xxx..0x10106xxx):
 * output and argument helpers of the formatter, bit-level double helpers, stream (FILE) helpers and the
 * heap set-up. Stream records are used as offsets: +0 next byte, +4 bytes left, +8 buffer, +0xc flags,
 * +0x10 file handle, +0x18 buffer size (the offsets every function below reads and writes together).
 *
 * Callees that are not converted are called through their original address (SIM_FN):
 *   0x10101960 (character, stream, counter pointer)   0x100fd9a0 (character, mask)
 *   0x10105a60 (stream)                                0x101095f0 (character, stream)
 *   0x100fefe0 (block)                                 0x100fef30 (size -> block)
 *   0x100fe290 (stream)                                0x10109890 (file handle -> Windows handle)
 *   0x10106660 (no arguments)
 */
#ifndef HG_SIM_UNSORTED_1010_W6J_H
#define HG_SIM_UNSORTED_1010_W6J_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

typedef int(__cdecl *W6jPutCharFn)(int ch, void *stream, int *count);
typedef int(__cdecl *W6jCharMaskFn)(int ch, int mask);
typedef int(__cdecl *W6jStreamFn)(void *stream);
typedef int(__cdecl *W6jCharStreamFn)(int ch, void *stream);
typedef void(__cdecl *W6jFreeFn)(void *block);
typedef void *(__cdecl *W6jMallocFn)(uint32_t size);
typedef uint32_t(__cdecl *W6jOsHandleFn)(uint32_t fd);
typedef int(__cdecl *W6jVoidFn)(void);
typedef int(__cdecl *W6jCallbackFn)(uint32_t value);

#define W6J_PUTCHAR SIM_FN(W6jPutCharFn, 0x10101960)
#define W6J_CHARMASK SIM_FN(W6jCharMaskFn, 0x100fd9a0)
#define W6J_FILL SIM_FN(W6jStreamFn, 0x10105a60)
#define W6J_PUTBACK SIM_FN(W6jCharStreamFn, 0x101095f0)
#define W6J_FREE SIM_FN(W6jFreeFn, 0x100fefe0)
#define W6J_MALLOC SIM_FN(W6jMallocFn, 0x100fef30)
#define W6J_FLUSH SIM_FN(W6jStreamFn, 0x100fe290)
#define W6J_OSHANDLE SIM_FN(W6jOsHandleFn, 0x10109890)
#define W6J_REGION_NEW SIM_FN(W6jVoidFn, 0x10106660)

/* ---- w6j_format.c ---- */
/* Calls 0x10101960(ch, stream, count) up to n times (n signed); stops early once *count is -1. */
void __cdecl Sim_101019b0(int ch, int n, void *stream, int *count);
/* Calls 0x10101960(s[i] sign-extended, stream, count) for i = 0..n-1 (n signed); stops early once *count is -1. */
void __cdecl Sim_101019f0(const char *s, int n, void *stream, int *count);
/* Advances the argument cursor by 4 and returns the dword it passed. */
uint32_t __cdecl Sim_10101a30(uint32_t *cursor);
/* Advances the argument cursor by 8 and returns the two dwords it passed as one 64-bit value (edx:eax). */
uint64_t __cdecl Sim_10101a50(uint32_t *cursor);

/* ---- w6j_double.c ---- */
/* Stores 0x21 (type 1) or 0x22 (types 2 and 3) in the error dword 0x1017e888; other types change nothing. */
void __cdecl Sim_101037d0(int type);
/*
 * Returns x with its exponent field replaced: the top 16 bits become (top & 0x800f) | ((e + 0x3fe) << 4),
 * truncated to 16 bits (so an out-of-range e also reaches the sign bit). Returned in st(0).
 */
double __cdecl Sim_10103870(double x, int e);
/* 1 for +infinity, 2 for -infinity, 3 for a NaN with the top mantissa bit set, 4 for one without it, else 0. */
int __cdecl Sim_10103940(double x);

/* ---- w6j_stream.c ---- */
/*
 * When the flag byte for ch in the table at [0x10159450] (2 bytes per character), or 0x100fd9a0(ch, 4) while
 * the dword 0x1015965c is above 1, does not have the bit 4: returns (ch & ~0x20) - 7; otherwise ch.
 */
int __cdecl Sim_101048e0(int ch);
/* Takes the next byte of the stream (zero-extended); when no bytes are left returns 0x10105a60(stream). */
int __cdecl Sim_10104920(void *stream);
/* Unless ch is -1 returns 0x101095f0(ch, stream); otherwise -1. */
int __cdecl Sim_10104950(int ch, void *stream);
/* When the flags have one of 0x83 and also 8: frees the buffer, clears flags 0x408 and the pointers and count. */
void __cdecl Sim_10104aa0(void *stream);
/*
 * For the two streams at 0x10159b68 (slot 0) and 0x10159b88 (slot 1) whose handle has flag 0x40
 * (Sim_10107080): counts in the dword 0x1017e964 and, unless flags 0x10c are set, gives the stream the
 * 0x1000-byte buffer kept in the slot dword at 0x1017e930 (allocated on first use) and sets flags 0x1102.
 * Returns 1 when it did, else 0.
 */
int __cdecl Sim_10104ae0(void *stream);
/*
 * When the stream has flag 0x1000 runs 0x100fe290 on it; with a non-zero `release` it then also clears
 * flags 0x1100, the buffer size, next-byte and buffer pointers.
 */
void __cdecl Sim_10104b80(int release, void *stream);
/*
 * For an open file handle (flag 1 in the handle table 0x10ac3720, count 0x10ac385c): FlushFileBuffers on its
 * Windows handle; returns 0 on success. Otherwise (or when the handle is not open) stores the GetLastError code
 * in the dword 0x1017e88c (only on a failed flush), 9 in 0x1017e888, and returns -1.
 */
int __cdecl Sim_101056f0(uint32_t fd);
/* Frees every non-null block in the 64 dwords at 0x10ac3720 (the handle table) and clears them. */
void __cdecl Sim_10105fb0(void);

/* ---- w6j_heap.c ---- */
/* Calls the function stored in the dword 0x1017e96c (when set) with value; 1 if it answers non-zero, else 0. */
int __cdecl Sim_10106590(uint32_t value);
/* HeapCreate(1, 0x1000, 0) into the dword 0x10ac3714; then 0x10106660, destroying the heap when that fails. 1 on success. */
int __cdecl Sim_101065b0(void);
/*
 * VirtualFree(p, 0, 0x8000) for the dword +0x10 of every record of the ring that starts at 0x10159e38 (link at
 * +0) when it is non-zero, then HeapDestroy of the heap in the dword 0x10ac3714.
 */
void __cdecl Sim_101065f0(void);

#endif
