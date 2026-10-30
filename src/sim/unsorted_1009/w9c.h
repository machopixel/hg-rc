/*
 * Round 9, letter c: the object tree builder 0x1009d90e (integer game code).
 */
#ifndef HG_SIM_UNSORTED_1009_W9C_H
#define HG_SIM_UNSORTED_1009_W9C_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* ---- w9c_tree.c ---- */
/*
 * Walks a tree of 0x28-byte nodes (+0 first child, +4 next sibling, +8 the object, +0xc the parent key, +0x10..
 * +0x18 and +0x1c..+0x24 two triples) and returns the object the node stands for. Unless `keep` is set and
 * except for objects whose name (Sim_100a9b7f) contains "WRF" or "WBZ", the object is detached: Sim_10029ab3(obj,
 * 0x100020e5) and Sim_1003b986(obj). Otherwise a new object Sim_100a9830(key, 10) takes its name, and a new entry
 * of the 0x7e-byte table (count 0x1015ea80, copied from the entry of the old object) points at it (+0x60; +0 =
 * index, +4 = -1, +0x48 = the entry number of the key, +0x50..+0x5c = 0); an object named "POF" is also written
 * to slot +0x5fb of the array +0x498 of the block +0x28 of PTRTAB(index), and the first child's +0xc gets it.
 * Then Sim_100a9d47(obj, triple 1), Sim_100aa0da(obj, triple 2, 0), obj+4 / obj+8 = the results for the first
 * child (keep = whether a new object was made) and the next sibling, obj+0 = the key. 0 for no node, no object or
 * a failed allocation.
 *
 * The original reads the block of PTRTAB(index) into a local only for 0 <= index < 0x19 and uses that local for
 * "POF" objects whatever the index; the only caller (0x1009d3f6) checks the range first, so the local is 0 here
 * for an index outside it.
 */
uint32_t __cdecl Sim_1009d90e(void *node, int32_t keep, int32_t index);

#endif
