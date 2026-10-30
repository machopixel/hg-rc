/*
 * Wave 2 functions of HGSIM.DLL around 0x10100000..0x10109000 (letter h).
 * Names carry the original address because the code alone does not say what
 * the fields mean.
 */
#ifndef HG_SIM_UNSORTED_1010_W2H_H
#define HG_SIM_UNSORTED_1010_W2H_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Dword 0x101596f8 = value. */
void __cdecl Sim_10100390(uint32_t value);
/*
 * The nodes of the circular list headed at 0x10159e38 (next pointer at +0) each
 * cover the addresses above their dword +0x10 and below their dword +0x14. For
 * an address addr inside one, a multiple of 16 and at least 0x100 into its 4 KB
 * page: stores that node in *node and the page start in *page, and returns
 * page + 8 + ((addr - page - 0x100) >> 4). Returns 0 otherwise.
 */
uint32_t __cdecl Sim_10106900(uint32_t addr, uint32_t *node, uint32_t *page);
/* Clears the 0x101 bytes at 0x1017eac0 and the dwords 0x1017ebc4, 0x1017ebc8, 0x1017ebd0, 0x1017ebd4, 0x1017ebd8; returns 0. */
int __cdecl Sim_10107fd0(void);
/*
 * Converts a flag word to another layout: bit 4 -> 0x1, 3 -> 0x4, 2 -> 0x8, 1 -> 0x10,
 * 0 -> 0x20, 19 -> 0x2; bits 8-9 (1, 2, 3) -> 0x400, 0x800, 0xc00; bits 16-17
 * (0) -> 0x300, (1) -> 0x200; bit 18 -> 0x1000.
 */
uint32_t __cdecl Sim_101087b0(uint32_t flags);

#endif
