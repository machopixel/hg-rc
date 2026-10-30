/*
 * Converted functions of HGSIM.DLL from around 0x10010000..0x10015000 that
 * only call functions already converted. Their names carry the original
 * address because the code alone does not say what the fields mean.
 */
#ifndef HG_SIM_UNSORTED_1001_H
#define HG_SIM_UNSORTED_1001_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * 1 + (the signed word at +0xa of the block at +0x19c of the table-0x109e8b40
 * record that Sim_10015c61(value) finds); 0 when value >= 8 or nothing is found.
 */
int32_t __cdecl Sim_100100ca(int32_t value);
/*
 * The record of table 0x109e8b40 that Sim_10015c61(arg[0xc]) finds supplies a
 * word at +0x1aa: its low byte indexes, its bits 0xf00 pick the table. 0x100:
 * 0x54 byte entries at 0x109a8668, 0x400: 0x50 byte entries at 0x109e9bb8,
 * 0x200: the block at +0x124 of table entry [index]. Returns 0 when nothing is
 * found, for another table bits, or when the word at +0x1a6 is 5. The other
 * three arguments are not used.
 */
void *__cdecl Sim_10010870(uint32_t unused1, const void *arg, uint32_t unused3, uint32_t unused4, uint32_t unused5);
/* Bit 0 of the 16-bit word at +0x562 of the record that obj[0x28] of the table-0x109e8b40 entry Sim_10052aaf(obj[8]) points to. */
int32_t __cdecl Sim_100140df(const void *obj);

#endif
