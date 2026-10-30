/*
 * Wave 2g: leaf functions of HGSIM.DLL from 0x100071da..0x1000e510 (small
 * field setters and getters, a table clear). Names carry the original address
 * because the code alone does not say what the objects are.
 */
#ifndef HG_SIM_W2G_1000_H
#define HG_SIM_W2G_1000_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* The word at 0x10ac1b04 + 14 * (int16)index = (uint16)value. */
void __cdecl Sim_100071da(int index, int value);
/* The 16-bit word at +0x14 of the record obj[0x30] points to, zero extended; 0 when obj[0x30] is null. */
uint32_t __cdecl Sim_10008ef8(const void *obj);
/* The word at obj + 0x26 = (uint16)value. */
void __cdecl Sim_1000941e(void *obj, int value);
/*
 * obj[0x30] points to a block with a table of 0x28-byte records at block +
 * block[8]. Reads record `index`: *id = its word at +0, *count = its word at
 * +2, and copies min(count, max) bytes from record + record[4] to out[], each
 * widened to a dword. Any of id, count and out may be null. Does nothing when
 * obj[0x30] is null.
 */
void __cdecl Sim_10009613(const void *obj, int index, uint32_t *id, uint32_t *count, uint32_t *out, int max);
/* *word = (*word & 0xfef0) | (mask & 0x10f) | 0x8000. */
void __cdecl Sim_1000971b(void *word, uint32_t mask);
/* 1 when bit 0x200 of the 16-bit value is set and its low byte equals the dword at 0x1017b8c8, else 0. */
int __cdecl Sim_1000afe7(int value);
/* [0x10a38de0] = 0 and clears the dword at +0 and the byte at +4 of the 256 entries (0x14 bytes each) of the table at 0x10a39200. */
void __cdecl Sim_1000df50(void);
/* Address of the byte at +4 of entry `index` of the table at 0x10a39200 (entries are 0x14 bytes). */
uint8_t *__cdecl Sim_1000e510(int index);

#endif
