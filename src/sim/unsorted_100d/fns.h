/*
 * Converted leaf functions of HGSIM.DLL from the code block around
 * 0x100d0000..0x100e8000. Despite the neighbouring loader strings, the
 * functions converted here are small helpers: list and pool bookkeeping,
 * flag tests and setters/copies of the globals that hold the current
 * camera/view state. Plain C; src/hooks/conv_100d.inc puts each one in
 * place of the original. Every function keeps the original cdecl convention.
 * Functions that cannot be named from the code keep the Sim_<address> name.
 */
#ifndef HG_SIM_UNSORTED_100D_H
#define HG_SIM_UNSORTED_100D_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* nodes.c */
/*
 * Removes a node from the doubly linked list of its owner (node+0x14).
 * Node: +4 next, +8 previous, +0x14 owner. Owner: +0x10 first, +0xc last.
 * Does nothing for a node without an owner; clears the node's links.
 */
void __cdecl List_Unlink(void *node);
/*
 * A pool of 0x24-byte entries at owner+0xfc with a stack of entry numbers at
 * owner+0x758 and the stack depth at owner+0x754. Take returns the next
 * entry (0 when the pool is exhausted at 0x2d entries or owner is 0);
 * Release puts entry `index` back, marking its +4 field -1.
 */
void *__cdecl SlotPool_Take(void *owner);
void __cdecl SlotPool_Release(void *owner, int index);
/* Reads a block whose header holds the size at +4 and two values at +8/+0xc; the payload starts at +0x10. Each output may be 0. */
void __cdecl Sim_100da020(void *block, uint32_t *out8, uint32_t *outC, uint32_t *payload);

/* flags.c */
int __cdecl Sim_100d5c9b(int value);
uint32_t __cdecl Sim_100dcbe0(uint32_t flags);
int __cdecl Sim_100d2c6f(void);
int __cdecl Sim_100d1df9(void);

/* state.c */
void __cdecl Sim_100d1cce(void);
void __cdecl Sim_100d252c(void *object, uint32_t value);
void __cdecl Sim_100d2545(void *object, uint32_t value);
void __cdecl Sim_100d32e5(int preset);
void __cdecl Sim_100d4500(void *object);
void __cdecl Sim_100d6392(uint32_t value);
int __cdecl Sim_100db770(int value);
void __cdecl Sim_100db77e(int value);
void __cdecl Sim_100e282a(uint32_t value);

#endif
