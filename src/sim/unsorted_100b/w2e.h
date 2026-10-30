/* Wave 2e, original 0x100b6b87..0x100bea42: string comparisons, two path/text helpers and an object reset. */
#ifndef HG_SIM_UNSORTED_100B_W2E_H
#define HG_SIM_UNSORTED_100B_W2E_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Str_Compare(*a, *b) where a and b point to string pointers (a qsort-style comparison). */
int __cdecl Str_CompareIndirect(const char *const *a, const char *const *b);
/* Str_CompareN(a + 0x18, b, 4) */
int __cdecl Sim_100b971d(const char *a, const char *b);
/* Str_CompareN(a + 0x18, b + 0x18, 4) */
int __cdecl Sim_100b9db0(const char *a, const char *b);
/* Cuts trailing blanks, tabs, CR and LF off the string by writing the new terminator (at index strlen - trailing + 1; one past the string for an empty one). */
/*
 * list has a signed count at +4 and a pointer to an array of string pointers at +8.
 * Returns the first string whose first four bytes equal those of key, or 0.
 */
const char *__cdecl Sim_100bcb52(const char *key, const void *list);
/* Str_CompareN(a, b, 8) */
int __cdecl Sim_100bcba7(const char *a, const char *b);
/*
 * Looks for a '.' from the end of path towards (but not including) its first character; if
 * found, the text after it is replaced with ext (strcpy, no length check).
 */
void __cdecl Path_ReplaceExtension(char *path, const char *ext);
/*
 * Reinitialises an object: shorts +0x1f8 and +0x1fa = 0xFFFF, dwords +0x1fc and +0x23a = -1,
 * +0x24a, +0x25a and short +0x25e = 0, +0x260 = 1.0f, 8 bytes at +0x270 cleared, +0x278 = 0x3a98
 * when 0 and +0x27c = 0x10151ae0 + dword[+0] * 8 (0x10151b38 if the dword +0x538 of the
 * block at +0x28 is 1) when 0. The signed byte +0x1b7 is clamped to 1..6 and its value sets the
 * bits 0..6 of the dword +0x26c (bit 0, 1: <= 3; 2: <= 2; 3: dword[+0] == 1 and <= 5;
 * 4: <= 4; 5: dword[+0] == 1 and <= 1; 6: <= 5); when dword[+0] == 1 and bit 3 ended up 0
 * the dword +0x534 of the block at +0x28 is cleared.
 */
void __cdecl Sim_100bea42(void *obj);

#endif
