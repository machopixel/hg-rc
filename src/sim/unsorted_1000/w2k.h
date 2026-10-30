/*
 * Small leaf functions of HGSIM.DLL around 0x10008e8b..0x1000e060: accessors of
 * object fields and one scan of a tagged byte stream. Names carry the original
 * address because the code alone does not say what the fields mean.
 */
#ifndef HG_SIM_UNSORTED_1000_W2K_H
#define HG_SIM_UNSORTED_1000_W2K_H

#include <stddef.h>
#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Stores value through the pointer at obj+0x30; does nothing when that pointer is 0. */
void __cdecl Sim_10008e8b(void *obj, uint32_t value);
/* The word at obj, masked with 0x7ef0; 0 for a NULL obj. */
uint32_t __cdecl Sim_100093ea(const void *obj);
/* -1 for a NULL obj; 1 when bit 0x200 of obj+0xf0 is set and the record 0x109e8b40[low byte of that dword] starts with 9; otherwise 0. */
int __cdecl Sim_1000c627(const void *obj);
/*
 * Looks for the dword at 0x1012df58 (as a dword at any byte offset) in the first
 * *count bytes after *cursor. When found, *cursor ends after it and the dword
 * that follows, and *count receives that following dword; otherwise *cursor has
 * moved *count bytes on and *count becomes 0.
 */
void __cdecl Sim_1000e060(const uint8_t **cursor, int *count);

#endif
