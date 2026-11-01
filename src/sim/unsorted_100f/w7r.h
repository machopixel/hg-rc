/*
 * Round 7, letter r: C runtime code of the DLL around 0x100ff000 (character case mapping).
 * Callees that are not converted are called through their original address (SIM_FN):
 *   0x101053c0 (locale, flags, source, source length, destination, destination size, code page): the locale
 *              string mapping helper; the flag 0x100 that 0x100ff280 passes is LCMAP_LOWERCASE.
 */
#ifndef HG_SIM_UNSORTED_100F_W7R_H
#define HG_SIM_UNSORTED_100F_W7R_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

typedef int(__cdecl *W7rMapStringFn)(uint32_t locale, uint32_t flags, const uint8_t *src, int srcLen, uint8_t *dst,
                                     int dstSize, uint32_t codePage);

#define W7R_FN_101053C0 SIM_FN(W7rMapStringFn, 0x101053c0)

/*
 * Lower case of c. While the locale dword 0x1017e950 is 0 only 'A'..'Z' change. Otherwise a c below 0x100
 * that is not upper case (flag 1 in the table at [0x10159450], or Crt_IsCType(c, 1) while the dword
 * 0x1015965c is above 1) is returned as it is; anything else is mapped with 0x101053c0 as one byte, or as two
 * bytes (high byte first) when the table marks the high byte as a lead byte (flag 0x80 in its second byte).
 * The answer is c when the mapping gives 0 bytes, the first byte for 1, else first | second << 8.
 */
int __cdecl Crt_ToLower(int c);
/* 1.2 0x100ea880, _strlwr: A-Z to a-z in place (without a locale; with one through 0x101053c0). Returns s. */
char *__cdecl Crt12_StrLwr(char *s);

#endif
