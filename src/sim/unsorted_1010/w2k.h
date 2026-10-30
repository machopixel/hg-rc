/*
 * Wave 2 (letter k): functions of the C runtime that is linked into HGSIM.DLL
 * (0x10107080, 0x101077f0, 0x1010c940). Names carry the original address where
 * the code alone does not say what the data means.
 */
#ifndef HG_SIM_UNSORTED_1010_W2K_H
#define HG_SIM_UNSORTED_1010_W2K_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * File handle table lookup: 0 when handle >= the dword at 0x10ac385c; otherwise
 * the flag byte at +4 of the 8-byte slot (handle & 31) in block (handle >> 5) of
 * the pointers at 0x10ac3720, masked with 0x40.
 */
uint32_t __cdecl Sim_10107080(unsigned handle);
/*
 * Searches the 12-byte entries at 0x1015c148 (count in the dword at 0x1015c1c8)
 * for one whose dword at +4 equals key; returns it or NULL.
 */
void *__cdecl Sim_101077f0(uint32_t key);
/*
 * Like Crt_StrChr for multibyte strings: while the dword at 0x1017ebc4 is 0 it
 * is Crt_StrChr(s, c). Otherwise a byte whose flag 4 is set in the table at
 * 0x1017eac1 starts a two-byte character and c is compared as a 16-bit value;
 * returns the pointer to the match or NULL. A 0 in c finds the end of the string.
 */
char *__cdecl Crt_MbsChr(const char *s, unsigned c);

#endif
