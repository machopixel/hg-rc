/*
 * Round 5 (letter z): small functions of HGSIM.DLL between 0x100b388a and 0x100bc4f7.
 * Calls of functions that are not converted yet go through their original
 * address (SIM_FN); the others call the converted C functions by name.
 */
#ifndef HG_SIM_UNSORTED_100B_W5Z_H
#define HG_SIM_UNSORTED_100B_W5Z_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * When dword 0x1019a07c is set: sets dword 0x1017d228 to 1, calls the original 0x100b469d, clears both dwords.
 * Then calls the original 0x1003b534 and Sim_100b4213.
 */
void __cdecl Sim_100b388a(void);

/*
 * Decrements dword 0x1017d224. When the result is below 0 calls the original 0x100bc280(0x22, 0), else stores the
 * dword at index 0x1017d224 of the table at 0x101c4220 in dword 0x101507f8. The argument is not used.
 */
void __cdecl Sim_100b3f94(uint32_t unused);

/* Calls the original 0x100b48de(i, dword at +0x10 of record i) for each record i (0x88 bytes each, from 0x101a2100) below dword 0x1017d218. */
void __cdecl Sim_100b418e(void);

/*
 * Looks key up with the original 0x10100620(&key, table[1], table[0], 8, compare function 0x100b6b87) and returns the dword at +4
 * of the entry it finds. Returns key when there is no table or no entry.
 */
uint32_t __cdecl Sim_100b787f(const void *table, uint32_t key);

/*
 * Formats the arguments with the original 0x100fc780 (format string at 0x10151484) into the buffer at 0x1017da18, passes that
 * to the original 0x1005f9aa and to Mono_PrintText, and returns the buffer.
 */
char *__cdecl Sim_100bc4f7(uint32_t a, uint32_t b);

#endif
