/*
 * Round 6, letter i: C runtime code of the DLL around 0x10100000..0x10102000 (heap check, binary search,
 * file length, the Pentium division check, text helpers of the number formatting). Helpers that are not
 * converted yet are called through their original address with SIM_FN.
 */
#ifndef HG_SIM_UNSORTED_1010_W6I_H
#define HG_SIM_UNSORTED_1010_W6I_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#define __stdcall
#endif

/* Not converted yet: the original helpers, called through their address. */
#define W6I_FN_10106E50 SIM_FN(int(__cdecl *)(void), 0x10106e50)
#define W6I_FN_10105FE0 SIM_FN(int32_t(__cdecl *)(uint32_t, int32_t, int), 0x10105fe0)
#define W6I_FN_101086C0 SIM_FN(uint32_t(__cdecl *)(uint32_t, uint32_t), 0x101086c0)
#define W6I_FN_10100BE0 SIM_FN(uint32_t(__cdecl *)(uint32_t, uint32_t, uint32_t, uint32_t), 0x10100be0)
#define W6I_FN_100FF030 SIM_FN(int(__cdecl *)(int, void *), 0x100ff030)

/*
 * Starts with -2; -4 when 0x10106e50() is negative. HeapValidate(heap 0x10ac3714, 0, NULL) failing turns
 * the result into -4, except when GetLastError() is 0x78: then dword 0x1017e88c = 0x78, dword 0x1017e888
 * = 0x28 and the result is kept.
 */
int __cdecl Heap_Check(void);
/*
 * Binary search for key among num elements of width bytes from base, sorted for compare(key, element)
 * (C library bsearch). Returns the matching element or NULL.
 */
void *__cdecl Search_Binary(const void *key, uint8_t *base, uint32_t num, uint32_t width,
                            int(__cdecl *compare)(const void *, const void *));
/*
 * For an open handle (below dword 0x10ac385c, bit 0 of byte +4 of its 8-byte entry in the tables
 * pointed to from 0x10ac3720, 32 per table): position = 0x10105fe0(fd, 0, 1), end = 0x10105fe0(fd, 0, 2),
 * then back to the position if it differs; returns the end, or -1 if the first call gave -1. Other
 * handles: dword 0x1017e888 = 9, dword 0x1017e88c = 0, returns -1.
 */
int32_t __cdecl File_GetLength(uint32_t fd);
/* Returns 0x101086c0(0x10000, 0x30000). */
uint32_t __cdecl Sim_101009f0(void);
/*
 * The Pentium division check: 1 when 4195835 - (4195835 / 3145727) * 3145727, computed in double at run
 * time, is above the double at 0x1012bf38 (1.0), else 0.
 */
int __cdecl Fpu_HasFdivBug(void);
/*
 * Returns IsProcessorFeaturePresent(0) when GetModuleHandleA("KERNEL32") and GetProcAddress find it
 * (the names at 0x1012bf5c and 0x1012bf40), otherwise Fpu_HasFdivBug().
 */
int __cdecl Fpu_CheckFdivBug(void);
/* Sets byte 0x1017e908 to 1 around 0x10100be0(a, b, c, d) and returns its result. */
uint32_t __cdecl Sim_10100ed0(uint32_t a, uint32_t b, uint32_t c, uint32_t d);
/* Moves the text at str (with its 0) up by count bytes (Mem_Copy); nothing for count 0. */
void __cdecl Str_MakeRoom(char *str, uint32_t count);
/*
 * Puts the low byte of c into the stream f (dword +0 = write pointer, +4 = room left; 0x100ff030 when
 * there is no room). *count becomes -1 if that returns -1, otherwise it is increased.
 */
void __cdecl Stream_PutCharCounted(int c, uint8_t *f, int *count);

#endif
