/*
 * Wave 2, letter j: leaf functions of HGSIM.DLL around 0x100f9000..0x10100000.
 */
#ifndef HG_SIM_UNSORTED_100F_W2J_H
#define HG_SIM_UNSORTED_100F_W2J_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * The statically linked C runtime string routines (the 0x7efefeff word tricks of
 * Microsoft's strcat/strncat/strcspn/strrchr) and two fixed-point divisions. The
 * string routines behave as the standard ones; strings must not overlap.
 */

/* strcat: appends src to dst; returns dst. */
char *__cdecl Str_Cat(char *dst, const char *src);
/* strncat: appends at most count characters of src to dst and always a NUL (nothing at all when count is 0); returns dst. */
char *__cdecl Str_NCat(char *dst, const char *src, unsigned count);
/* strcspn: length of the leading part of s made of characters not in set. */
unsigned __cdecl Str_CSpan(const char *s, const char *set);
/* strrchr: last occurrence of the low byte of c in s (the NUL counts); 0 if none. */
char *__cdecl Str_FindLast(const char *s, int c);

/*
 * Rounded (|a| << 30) / |b| with the sign of a * b (the high dword of the
 * dividend is |a| >> 2 arithmetic, so a = INT_MIN is an unsigned 0x80000000
 * sign-extended). The remainder rounds the quotient up when it is at least
 * half of |b|. The original raises a divide error for b == 0 and for
 * quotients over 32 bits; this version does not (it returns the low 32 bits
 * and 0 for b == 0).
 */
int32_t __cdecl Sim_100f9f50(int32_t a, int32_t b);
/* Same with the dividend 0x4000 << 32 (the sign is that of b alone): rounded 2^46 / |b|. */
int32_t __cdecl Sim_100f9f88(int32_t b);

#endif
