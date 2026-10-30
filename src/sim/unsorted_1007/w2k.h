/*
 * Wave 2 (letter k) additions to unsorted_1007: two leaf functions of
 * HGSIM.DLL (0x10076689, 0x10079ecd). Names carry the original address because
 * the code alone does not say what the data means.
 */
#ifndef HG_SIM_UNSORTED_1007_W2K_H
#define HG_SIM_UNSORTED_1007_W2K_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Walks the list from the pointer at 0x1015dc14 (next at +0x14) and returns the dword at +0xc of the first node whose dword at +4 is 1; 0 if there is none. */
uint32_t __cdecl Sim_10076689(void);
/*
 * id[0] is a record index and id[1] selects the table: 1 = 0x109a8640 (0x54-byte
 * records), 2 = the record pointers at 0x109e8b40, 4 = 0x109e9ba0 (0x50-byte
 * records). Returns 1 when the signed 16-bit word at record +0x24 (table 2: +0x1c,
 * table 4: +0) has bit 0x20 set and bit n (shift taken modulo 32) of the signed
 * 16-bit word at +0x26 (table 2: +0x1e, table 4: +6) is set; otherwise 0.
 */
int __cdecl Sim_10079ecd(const uint8_t *id, int n);

#endif
