/*
 * Round 7 (letter s): more of the C runtime linked into HGSIM.DLL
 * (0x10104970..0x1010d180): stream set-up and refill, the handle table
 * start-up, the small-block heap allocator, locale wrappers around the
 * Windows string functions, environment and command line set-up, the
 * run-time error message writer and the special-operand helper of the power
 * function.
 * Names carry the original address where the code alone does not say what
 * the data means. Helpers that are not converted are called through their
 * original address (SIM_FN), Windows through the DLL's import slots.
 */
#ifndef HG_SIM_UNSORTED_1010_W7S_H
#define HG_SIM_UNSORTED_1010_W7S_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#define __stdcall
#endif

/* ---- unconverted helpers, called through their original address ---- */
/* 0x101099a0: (file name, open flags, share flag, 0x1a4) -> handle number, negative on failure. */
#define W7S_FN_101099a0 SIM_FN(int32_t (__cdecl *)(const char *, uint32_t, uint32_t, uint32_t), 0x101099a0)
/* 0x10100580: (2, count) -> block or 0. */
#define W7S_FN_10100580 SIM_FN(void *(__cdecl *)(uint32_t, uint32_t), 0x10100580)
/* 0x101055e0: (text, max) -> length of the text, at most max. */
#define W7S_FN_101055e0 SIM_FN(int32_t (__cdecl *)(const char *, int32_t), 0x101055e0)
/* 0x10105b50: (handle number, buffer, size) -> bytes read, 0 or -1. */
#define W7S_FN_10105b50 SIM_FN(int32_t (__cdecl *)(uint32_t, uint32_t, uint32_t), 0x10105b50)
/* 0x101009b0: one number; does not come back in the game (calls the exit routine). */
#define W7S_FN_101009b0 SIM_FN(void (__cdecl *)(uint32_t), 0x101009b0)
/* 0x1010b2a0: ("NAME=value" text, 0). */
#define W7S_FN_1010b2a0 SIM_FN(int32_t (__cdecl *)(char *, uint32_t), 0x1010b2a0)
/* 0x1010c6a0: (16-bit text, max) -> number of characters up to and with its 0, or max. */
#define W7S_FN_1010c6a0 SIM_FN(uint32_t (__cdecl *)(const uint16_t *, uint32_t), 0x1010c6a0)
/* 0x101064e0: (double y) -> 0 when y is not an integer, 1 or 2 (x87 code that halves y). */
#define W7S_FN_101064e0 SIM_FN(int32_t (__cdecl *)(double), 0x101064e0)
/* 0x10108680: (new, mask) -> sets the x87 control word bits of mask to new; returns the new value. */
#define W7S_FN_10108680 SIM_FN(uint32_t (__cdecl *)(uint32_t, uint32_t), 0x10108680)

/* ---- w7s_stdio.c: streams (+0 pointer, +4 count, +8 buffer, +0xc flags, +0x10 handle number,
 * +0x18 buffer size, +0x1c) and the handle table (pointers to blocks of 32 eight-byte slots at
 * 0x10ac3720, slot count in 0x10ac385c) ---- */
/* Counts *count up and reads characters from the stream with Sim_10104920 while Sim_101093f0 says
 * they are class 8; returns the first other one. */
int32_t __cdecl Sim_10104970(int32_t *count, void *stream);
/* Parses the mode text ("r", "w", "a", then + b t c n S R T D), opens name with 0x101099a0 and sets up
 * the stream; returns it, or 0. */
uint8_t *__cdecl Sim_10104be0(const char *name, const char *mode, uint32_t share, uint8_t *stream);
/* Returns the first stream of the table at [0x10ac3860] (0x10ac4ba0 entries) with none of the flags 0x83,
 * making a new 0x20-byte one in an empty slot; cleared with handle -1. 0 when none. */
uint8_t *__cdecl Sim_10104db0(void);
/* Stores the Windows error code in 0x1017e88c and its mapped number (table 0x101599e0, ranges
 * 0x13..0x24 -> 0xd, 0xbc..0xca -> 8, else 0x16) in 0x1017e888. */
void __cdecl Sim_10105100(uint32_t oserr);
/* Refills the stream buffer with 0x10105b50 and returns the next byte, or -1 (flags 0x10 / 0x20). */
int32_t __cdecl Sim_10105a60(uint8_t *stream);
/* Builds the handle table: the first block, the handles passed in the start-up information, and the
 * three standard handles; then SetHandleCount. */
uint32_t __cdecl Sim_10105db0(void);
/* The handle stored in slot fh when its flag bit 1 is set; else -1 with 9 in 0x1017e888. */
uint32_t __cdecl Sim_10109890(uint32_t fh);

/* ---- w7s_heap.c ---- */
/* Small-block heap: n map bytes from the current region (0x1015be58), the other regions, the
 * uncommitted pages of a region, or a new region (Sim_10106660); returns the address or 0. */
uint32_t __cdecl Sim_101069c0(int32_t n);

/* ---- w7s_locale.c: wrappers around the Windows string functions ---- */
/* GetStringTypeA, or GetStringTypeW on the text converted with MultiByteToWideChar (the choice is
 * made once, 0x1017e93c); 0 on failure. */
int32_t __cdecl Sim_10104fd0(uint32_t infoType, const char *src, int32_t count, uint16_t *types,
                             uint32_t codePage, uint32_t locale);
/* LCMapStringA, or LCMapStringW on the converted text and back (the choice is made once,
 * 0x1017e944); returns the length, or 0. */
int32_t __cdecl Sim_101053c0(uint32_t locale, uint32_t flags, const char *src, int32_t count, char *dst,
                             int32_t dstCount, uint32_t codePage);
/* Sets the multibyte code page (Sim_10107f20 of cp): the lead-byte table at 0x1017eac1 from the known
 * pages at 0x1015be68 or from GetCPInfo. 0, or -1 when the page is unknown. */
int32_t __cdecl Sim_10107d30(uint32_t cp);
/* Sim_10107d30(-3). */
int32_t __cdecl Sim_10108010(void);
/* Converts one multibyte character to 16 bits (lead bytes flagged 0x8000 in the table at
 * [0x10159450]); returns its length, 0, or -1 with 0x2a in 0x1017e888. */
int32_t __cdecl Sim_10109200(uint16_t *pwc, const uint8_t *s, uint32_t n);
/* Character class 8 of c (Crt_IsCType when the dword 0x1015965c is above 1, else the table at
 * [0x10159450]). */
int32_t __cdecl Sim_101093f0(int32_t c);
/* CompareStringA, or CompareStringW on the converted texts (the choice is made once, 0x1017ed14),
 * with the empty-text cases decided by the lead bytes of GetCPInfo; 0 on failure. */
int32_t __cdecl Sim_1010afa0(uint32_t locale, uint32_t flags, const char *s1, int32_t n1, const char *s2,
                             int32_t n2, uint32_t codePage);
/* Converts the 16-bit text to multibyte text (at most n bytes into s; only counts when s is 0);
 * returns the byte count without the 0, or -1 with 0x2a in 0x1017e888. */
int32_t __cdecl Sim_1010c4b0(char *s, const uint16_t *pwcs, uint32_t n);
/* Upper case in place: ASCII a..z when the dword 0x1017e950 is 0, else through Sim_101053c0 with
 * LCMAP_UPPERCASE (0x200). Returns s. */
char *__cdecl Crt_StrUpper(char *s);

/* ---- w7s_env.c: environment, command line, run-time messages ---- */
/* Converts each 16-bit "NAME=value" text of the list at [0x1017e8b8] and passes it to 0x1010b2a0;
 * 0, or -1 when a conversion or a block fails. */
int32_t __cdecl Sim_10107910(void);
/* Copies the texts of the environment block [0x1017e8f4] that do not start with '=' into new blocks,
 * listed in a new pointer array (0x1017e8b0); frees the block. */
void __cdecl Sim_10107990(void);
/* Takes the module name into 0x1017e9b8 (pointer in 0x1017e8c0), splits the command line [0x10ac4ba4]
 * (or the module name) with Sim_10107b20 into one new block (0x1017e8a8); returns the word count in
 * 0x1017e8a4. */
int32_t __cdecl Sim_10107a80(void);
/* A copy of the environment strings (16-bit ones converted, the choice made once at 0x1017ebe4); 0 on
 * failure. */
char *__cdecl Sim_10108220(void);
/* Writes the run-time message for code (table 0x1015bf58) to the error handle or shows it with
 * Sim_10107840 after "Runtime Error!" and the program name. */
void __cdecl Sim_10108430(uint32_t code);
/* A new pointer array with Sim_1010c9f0 copies of the texts of the 0-ended list; 0 for list 0. */
char **__cdecl Sim_1010b530(char **list);

/* ---- w7s_math.c: special operands of the power function (x87 compares in the original; bit exact) ---- */
/* For y = +-infinity or x = +-infinity writes the result of x to the power y to res (0, 1, +-infinity,
 * -0 or the NaN at 0x10159990) and returns 1 only for |x| = 1 with infinite y; other operands: 0 and
 * res untouched. */
int32_t __cdecl Sim_101062b0(double x, double y, double *res);

/* ---- w7s_misc.c ---- */
/* 0x10108680(newv, mask & ~0x80000). */
uint32_t __cdecl Sim_101086c0(uint32_t newv, uint32_t mask);
/* Daylight saving test of the time structure (year +0x14, day of year +0x1c, hour +8, minute +4,
 * second +0) against the start and end computed by Sim_1010a310; 0 when 0x1015c024 is 0. */
int32_t __cdecl Sim_1010a0a0(const uint8_t *tm);

#endif
