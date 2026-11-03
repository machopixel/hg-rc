/*
 * Round 5, letter a: functions of 0x100a2b8c..0x100aacd5 that call helpers which are not converted yet.
 * Those helpers are called through their address (SIM_FN) with the prototypes below.
 */
#ifndef HG_SIM_UNSORTED_100A_W5A_H
#define HG_SIM_UNSORTED_100A_W5A_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Not converted: seven dwords (the first is the address 0x109e3e00). */
#define W5A_CALL_100ef727 SIM_FN(int(__cdecl *)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t), 0x100ef727)
#define W5A_CALL_100e86a1 SIM_FN(int(__cdecl *)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t), 0x100e86a1)
/* Not converted: (handle, buffer, count); returns the number of bytes read. */
#define W5A_CALL_10105b50 SIM_FN(int(__cdecl *)(uint32_t, uint32_t, uint32_t), 0x10105b50)
/* Not converted: one dword (a node). */
#define W5A_CALL_100aa64a SIM_FN(int(__cdecl *)(void *), 0x100aa64a)

/*
 * Calls 0x100ef727(0x109e3e00, a, b, [0x101cc2bc], [0x101ccc6c], 0, the word table 0x101659e0 [c]). Then [0x101cc2bc] =
 * a and [0x101ccc6c] = b.
 */
void __cdecl Sim_100a2b8c(uint32_t a, uint32_t b, uint32_t c);
/*
 * Calls 0x10105b50(handle, buffer, n) for chunks of at most 0x4000 bytes until count bytes are done (that
 * helper is the C runtime's low-level read: it calls ReadFile). Returns count, or -1 as soon as a call returns
 * another number than the chunk size.
 */
int __cdecl File_ReadChunked(uint32_t handle, uint32_t buffer, uint32_t count);
/*
 * Tree of nodes with the first child at +4 and the next sibling at +8. Clears the bits of mask in the word at
 * (dword node + 0x8c) + 2 (when that pointer is not null) for the node and, recursively, all its children.
 */
void __cdecl Sim_100aa5e8(void *node, uint32_t mask);
/*
 * Same tree: when bit 0 of the dword at +0x94 is set calls 0x100aa64a(node), otherwise handles every child
 * the same way.
 */
void __cdecl Sim_100aa742(void *node);
/*
 * Same tree: returns the first node (node itself, then the children depth first) whose pointer at +0x8c is
 * not null and whose word at (that pointer) + 0x26 equals id; NULL if there is none.
 */
void *__cdecl Sim_100aacd5(void *node, uint32_t id);

#endif
