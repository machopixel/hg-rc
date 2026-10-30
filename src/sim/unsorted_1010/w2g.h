/*
 * Wave 2g: C runtime helpers of HGSIM.DLL (0x101003d0..0x1010b5a0), converted
 * with their cdecl convention. The names carry the original address because
 * the DLL holds several copies of such helpers.
 */
#ifndef HG_SIM_W2G_1010_H
#define HG_SIM_W2G_1010_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* |value|; same as Math_AbsInt_100fd100. */
int __cdecl Math_AbsInt_101003d0(int value);
/*
 * Takes a double as its two halves. Returns the high half when its exponent
 * bits (0x7ff00000) are all set, otherwise just the exponent bits.
 */
uint32_t __cdecl Sim_10101d48(uint32_t low, uint32_t high);
/* Maps the low byte of a status word: bit 0x20 -> 5, 8 -> 1, 4 -> 2, 1 -> 3 (in that order), else 4 when the mask 2 is set, else 0. */
int __cdecl Sim_10103830(uint32_t status);
/* strnlen: length of the string, but at most `max`. */
uint32_t __cdecl Str_LengthMax_1010b270(const char *s, uint32_t max);
/* wcslen: number of 16-bit characters before the first 0. */
int __cdecl Wstr_Length_1010b5a0(const uint16_t *s);

#endif
