/*
 * Round 4, letter v: functions of HGSIM.DLL around 0x10034000..0x10039000 that call Windows (the heap).
 */
#ifndef HG_SIM_UNSORTED_1003_W4V_H
#define HG_SIM_UNSORTED_1003_W4V_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Allocates the zero-filled 0x18-byte record at [0x109e3da4] (dwords: [0x1017b918], [0x1017b91c], 1, 0x100, 1, 0)
 * and sets the dwords 0x109e3de4 (product of the two), 0x109e3de8, 0x109e3da8, 0x109e3e14 and 0x109e3de0 (the two
 * minus one), 0x109e3e18 and 0x109e3e1c (the two halved, signed division). Returns 1, 0 when the allocation fails.
 */
int __cdecl Sim_10034b30(void);
/*
 * Sim_10034b30, then Sim_100ab89a and Camera_SetAspect on its record, builds the 5-dword rectangle at 0x109e3e00
 * (pointer 0x109e3e20, 0, 0, width - 1, height - 1) and copies it to 0x1015e518, then Sim_100d3142 on it and
 * Sim_10047fe0, and sets [0x1015e538] = 1. Returns 1, or 0 when Sim_10034b30 failed (nothing else touched).
 */
int __cdecl Sim_10034c11(void);
/*
 * Sets owner[0x28] = 0, allocates a record of Sim_10072ff2() bytes (not zeroed) and, when that works, fills its
 * header (+0x84/+0x88/+0x8c point into it at +0xc51, +0x1591, +0x198d; +0 = owner; the ten dwords from +0x498 = 0)
 * and the eight 0x128-byte entries at +0x84's block, links it as owner[0x28] and sets owner[0x2c] = 0xc51.
 * Returns 1, or 0 when the allocation fails. The first argument is unused.
 */
int __cdecl Sim_1003730b(uint32_t unused, void *owner);
/*
 * Looks for a node with key `key` in the list headed by [0x109e3760] (link at node+0x690); when there is none,
 * allocates one (0x698 bytes, not zeroed), fills it (key, key[1], value, the three dwords of Sim_100a9cfb(key),
 * rec[0x5ff], a 0x128-byte copy of `src`, and a 0x24-byte copy of each record src[0x3c + i*4] of the table at
 * rec[0x8c] for i from src[0x108] up to src[0x38], clearing the word at +2 of each source record) and appends
 * it after the last node (the old head's +0x694 and [0x109e3760] now point to it). The head must not be NULL.
 */
void __cdecl Sim_10038fac(uint32_t value, uint32_t key, const void *rec, const void *src);

#endif
