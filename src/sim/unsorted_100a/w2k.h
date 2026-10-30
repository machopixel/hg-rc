/*
 * Wave 2 (letter k): three functions of HGSIM.DLL around 0x100aa190..0x100abbe8:
 * a record copy and a doubly linked list with a head and a tail pointer.
 * Names carry the original address because the code alone does not say what
 * the data means.
 */
#ifndef HG_SIM_UNSORTED_100A_W2K_H
#define HG_SIM_UNSORTED_100A_W2K_H

#include <stddef.h>
#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Copies the four dwords at src to obj+0x3c, clears bit 0x4000 and sets bit 1 of the dword at obj+0x94. */
void __cdecl Sim_100aa190(void *obj, const uint32_t *src);
/*
 * Takes node out of the list whose head and tail pointers are *head and *tail
 * (previous link at +0xc, next link at +0x10) and clears the node's two links.
 * Does nothing when node, *head or *tail is 0.
 */
void __cdecl Sim_100abb1b(void *node, uint32_t *head, uint32_t *tail);
/*
 * With the dwords n = 0x1017d1c4 and limit = 0x1017d1c8: unless n == limit - 1 or
 * index is outside 0..limit-1, takes the 0x34-byte entry 0x101c57e0 + index * 0x34
 * out of the list (head 0x1017d1d0, tail 0x1017d1d4) with Sim_100abb1b, pushes index
 * on the stack of dwords at 0x101c56c0 (n is incremented first) and returns the
 * entry. Otherwise returns 0.
 */
void *__cdecl Sim_100abbe8(int index);

#endif
