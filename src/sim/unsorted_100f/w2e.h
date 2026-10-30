/*
 * The C runtime string and memory routines that HGSIM.DLL carries as its own
 * copies (original 0x100fc690..0x100fe7f0, Visual C++ library code). They are
 * plain cdecl functions; the converted code calls these instead of the
 * originals. They behave like the standard routines of the same job, except
 * where noted (the comparisons return exactly -1, 0 or 1).
 */
#ifndef HG_SIM_UNSORTED_100F_W2E_H
#define HG_SIM_UNSORTED_100F_W2E_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* strcpy: copies the string with its terminator, returns dst. */
char *__cdecl Str_Copy(char *dst, const char *src);
/* strncpy: copies at most n bytes and pads with zeros up to n, returns dst. */
char *__cdecl Str_CopyN(char *dst, const char *src, uint32_t n);
/* strlen */
uint32_t __cdecl Str_Length(const char *s);
/* memset: every byte becomes the low byte of c, returns dst. */
void *__cdecl Mem_Fill(void *dst, int c, uint32_t n);
/* strcmp on unsigned bytes: -1, 0 or 1. */
int __cdecl Str_Compare(const char *a, const char *b);
/* memmove: the ranges may overlap, returns dst. */
void *__cdecl Mem_Move(void *dst, const void *src, uint32_t n);
/* strncmp on unsigned bytes: -1, 0 or 1; 0 when n is 0. */
int __cdecl Str_CompareN(const char *a, const char *b, uint32_t n);

#endif
