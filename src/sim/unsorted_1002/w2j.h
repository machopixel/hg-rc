/*
 * Wave 2, letter j: leaf functions of HGSIM.DLL around 0x10020000..0x10030000.
 * A pool of nodes hashed into 0x3f1 buckets (pointer table in dword
 * 0x1015dc30, chains through +8) and a second list through the same nodes
 * (links +0xc next, +0x10 previous; head dword 0x109f13f0, tail dword 0x109f13f4).
 */
#ifndef HG_SIM_UNSORTED_1002_W2J_H
#define HG_SIM_UNSORTED_1002_W2J_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * When the word +2 of the node is non-zero: clears it and appends the node to
 * the second list.
 */
void __cdecl Sim_10027c70(void *node);
/*
 * Empties the second list, then appends (Sim_10027c70) every node of the hash
 * table whose word +2 is 0, bucket by bucket and along each chain.
 */
void __cdecl Sim_10027d77(void);
/*
 * Node of the hash table whose word +0 equals key and whose dword +4 equals the
 * dword at name; the bucket is (key + the four signed bytes at name) % 0x3f1
 * (signed remainder). 0 when none, or when key < 0.
 */
void *__cdecl Sim_10027f33(int key, const void *name);
/* Sim_10027c70 on the node Sim_10027f33(key, name) finds, if any. */
void __cdecl Sim_1002828e(int key, const void *name);
/* obj+0xf4 = value. */
void __cdecl Sim_10028a26(void *obj, uint32_t value);
/* Does nothing (an empty function). */
void __cdecl Sim_1002c10e(void);

#endif
