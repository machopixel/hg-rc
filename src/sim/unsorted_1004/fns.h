/*
 * Small leaf functions of HGSIM.DLL found around 0x10044000..0x10053000:
 * a mode value with its setter and getter, a record copy, accessors that
 * unpack fields of one packed word (nibble fields and single flag bits) and
 * lookups into fixed-stride tables. None of them calls another function.
 * Their names carry the original address because the code alone does not
 * say what the fields mean.
 */
#ifndef HG_SIM_UNSORTED_1004_H
#define HG_SIM_UNSORTED_1004_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Mode value at 0x1015ef04. With [0x10179cd4] set it is taken from [0x1013aee0] (1 if that was -1). */
void __cdecl Sim_100448cd(int mode);
int __cdecl Sim_100448ff(void);
/* Copies six dwords from src to dst and sets dst[6] = 1; returns 0 if either is NULL. */
int __cdecl Sim_10044992(const uint32_t *src, uint32_t *dst);

/* Nibble fields of a packed word, each plus a constant. Bits 0-3, 4-7, 8-11, 12-15, 16-19. */
int __cdecl Sim_1004bf79(unsigned word); /* +6 */
int __cdecl Sim_1004bf87(int word);      /* +9 */
int __cdecl Sim_1004bf98(int word);      /* +0xb */
int __cdecl Sim_1004c30d(int word);      /* word >> 16, arithmetic */
int __cdecl Sim_1004c565(unsigned word); /* +0xe */
int __cdecl Sim_1004c573(int word);      /* +0xe */
int __cdecl Sim_1004c584(int word);      /* +0xe */
int __cdecl Sim_1004c595(int word);      /* +0x16 */
int __cdecl Sim_1004c5a6(int word);      /* +0x13 */
int __cdecl Sim_1004c909(unsigned word); /* +0x18 */
int __cdecl Sim_1004c917(int word);      /* +0x18 */
int __cdecl Sim_1004c928(int word);      /* +0x18 */
int __cdecl Sim_1004c939(int word);      /* +0x1d */
int __cdecl Sim_1004c94a(int word);      /* +0x20 */

/* Whether bit 0, 1, 4, 5 of a flag word is set. */
int __cdecl Sim_1004cd8d(unsigned flags);
int __cdecl Sim_1004cd9e(unsigned flags);
int __cdecl Sim_1004cdd1(unsigned flags);
int __cdecl Sim_1004cde2(unsigned flags);

/* Table lookups. */
int __cdecl Sim_1004d330(int index); /* dword at 0x101116b2 + index * 0xAA */
int __cdecl Sim_10052aaf(int index); /* dword at 0x109b7d40 + index * 0x6C, 0 if index >= 16 */
int __cdecl Sim_10052be7(int index); /* 0x109e8b40[index] points to a record; dword at 0x109b7d54 + record[+8] * 0x6C */
int __cdecl Sim_10052c02(int index); /* entry = dword at 0x109e9bb0 + index * 0x50; 2 if negative, else 0x109b8400[entry] */

#endif
