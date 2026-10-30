/*
 * Wave 2 (batch f) functions of HGSIM.DLL in 0x1010xxxx: the statically linked
 * C runtime's number conversion code (integer arithmetic on 96-bit mantissas,
 * binary <-> decimal digits), a random number generator and a file-mode
 * setter. Plain C, original cdecl convention.
 *
 * Two layouts of 12 bytes ("ld12") are used. The mantissa helpers 0x10108880..
 * 0x10108b10 treat three dwords with word 0 as the most significant. The
 * helpers 0x1010b5c0.. 0x1010cd00 treat them with word 0 as the least
 * significant; there bytes 0..1 are extra low precision, bytes 2..9 the 64-bit
 * mantissa and the word at byte 10 the sign (0x8000) and biased exponent.
 */
#ifndef HG_SIM_UNSORTED_1010_W2F_H
#define HG_SIM_UNSORTED_1010_W2F_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* seed (dword 0x101596f8) = seed * 214013 + 2531011; returns bits 16..30 of it. */
int __cdecl Crt_Rand(void);
/* Maps x87 status bits of the low byte of flags to another flag word (bit0 -> 0x10, 2 -> 8, 3 -> 4, 4 -> 2, 5 -> 1, 1 -> 0x80000). */
uint32_t __cdecl Sim_10108840(uint32_t flags);
/* 1 when every bit after position n (bit 0 = top bit of word 0) of the 96-bit value is 0. */
int __cdecl Sim_10108880(const uint32_t *bits, int n);
/* Adds 1 at bit position n of the 96-bit value and propagates the carry towards word 0; returns the carry out of word 0. */
int __cdecl Sim_101088f0(uint32_t *bits, int n);
/*
 * Rounds the 96-bit value to its first n+1 bits (positions 0..n): clears the
 * rest, first adding 1 at position n-1 when bit n is set and some bit after
 * it is set. Returns the carry out of that addition (0 when none was done).
 */
int __cdecl Sim_10108960(uint32_t *bits, int n);
/* dst = src, 3 dwords. */
void __cdecl Sim_10108a00(uint32_t *dst, const uint32_t *src);
/* Zeroes 3 dwords. */
void __cdecl Sim_10108a20(uint32_t *p);
/* 1 when all 3 dwords are 0. */
int __cdecl Sim_10108a30(const uint32_t *p);
/* Shifts the 96-bit value right by n bits (word 0 most significant). */
void __cdecl Sim_10108a50(uint32_t *p, int n);
/*
 * Converts the 10-byte extended value src (mantissa dwords at +2 and +6, word
 * +0, sign/exponent word at +0xa) to a binary format described by fmt (six
 * ints: [0] largest exponent, [1] smallest, [2] mantissa bits kept, [3]
 * exponent bits, [4] total bits 0x20 or 0x40, [5] bias) and stores it at dst.
 * Returns 0 for a normal result, 1 for overflow (infinity), 2 for zero or
 * underflow.
 */
int __cdecl Sim_10108b10(const uint16_t *src, uint32_t *dst, const int32_t *fmt);
/* Sim_10108b10 with the format at 0x1015bfe8 (64 bits). */
int __cdecl Sim_10108ce0(const uint16_t *src, uint32_t *dst);
/* Sim_10108b10 with the format at 0x1015c000 (32 bits). */
int __cdecl Sim_10108d00(const uint16_t *src, uint32_t *dst);
/*
 * Turns the double given by value as two dwords into decimal digits and
 * returns the structure at 0x1017ec10: signed char of the sign ('-' or ' '),
 * decimal exponent, the result of Sim_1010bfa0 and a pointer to the digit
 * string at 0x1017ebf4. Uses 17 digits.
 */
void *__cdecl Sim_10108f00(uint32_t lo, uint32_t hi);
/* Converts the double at in to the 10-byte extended value at out (a dword pair and a word). */
void __cdecl Sim_10108f80(uint32_t *out, const uint32_t *in);
/* *out = a + b; returns the carry. */
int __cdecl Sim_1010b5c0(uint32_t a, uint32_t b, uint32_t *out);
/* a += b over 3 dwords, word 0 least significant; returns the carry out of word 2. */
int __cdecl Sim_1010b5f0(uint32_t *a, const uint32_t *b);
/* Shifts the 96-bit value (word 0 least significant) left by 1. */
void __cdecl Sim_1010b660(uint32_t *p);
/* Shifts the 96-bit value (word 0 least significant) right by 1. */
void __cdecl Sim_1010b690(uint32_t *p);
/* Builds an ld12 from count digit values (not ASCII) at digits. count 0 or an all-zero result never ends. */
void __cdecl Sim_1010b6c0(const char *digits, uint32_t count, uint32_t *out);
/*
 * Formats the extended value (lo, hi mantissa dwords; expw sign/exponent word)
 * as up to ndigits decimal digits: out+0 word decimal exponent, out+2 sign
 * char, out+3 digit count, out+4 the digits and a 0. Returns 1 for numbers,
 * 0 for the text of infinities and NaNs. When bit 0 of flag is set, ndigits is
 * counted after the decimal point.
 */
int __cdecl Sim_1010bfa0(uint32_t lo, uint32_t hi, uint32_t expw, int ndigits, uint32_t flag, char *out);
/* ld12 a *= b (round to nearest even, overflow gives infinity, underflow zero). */
void __cdecl Sim_1010ca40(void *a, const void *b);
/* ld12 a *= 10^n using the tables at 0x1015c170 / 0x1015c2d0; clears a's low word first when flag is 0. */
void __cdecl Sim_1010cd00(void *a, int n, int flag);
/* Sets the text (0x4000) or binary (0x8000) bit of file handle fd; returns the previous mode or -1 (error code in the dword 0x1017e888). */
int __cdecl Sim_1010cd90(uint32_t fd, int mode);

#endif
