/*
 * Wave 2 (letter k): two string functions of the C runtime that is linked into
 * HGSIM.DLL (0x100fd650, 0x100ff160). Both are the usual C library functions.
 */
#ifndef HG_SIM_UNSORTED_100F_W2K_H
#define HG_SIM_UNSORTED_100F_W2K_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* strchr: the first occurrence of the low byte of c in s (the terminating 0 counts), or NULL. */
char *__cdecl Crt_StrChr(const char *s, int c);
/* strspn: the length of the initial part of s made only of bytes that occur in set. */
unsigned __cdecl Crt_StrSpn(const char *s, const char *set);

#endif
