/*
 * Round 3 (letter q) functions of HGSIM.DLL around 0x10008000..0x1000d000.
 * Names carry the original address because the code alone does not say what
 * the fields mean. Floating point is done as the original's x87 did it
 * (see util/w3q.h).
 */
#ifndef HG_SIM_UNSORTED_1000_W3Q_H
#define HG_SIM_UNSORTED_1000_W3Q_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Calls fn(node) for every node of the list whose first node is the dword at list+8; the
 * next pointer of a node is its dword +8 (read before fn is called). Does nothing when
 * list is null.
 */
void __cdecl Sim_1000951c(const void *list, void(__cdecl *fn)(void *node));
/*
 * Walks the list that starts at the dword obj+0x2c (next pointer at +0xc) to the first node
 * whose float at +0 is not above `limit` (an unordered pair counts as not above) and calls
 * Sim_10008c69(obj, node).
 */
void __cdecl Sim_10008e46(void *obj, float limit);
/*
 * Table of `count` (dword 0x1015d0a0) records of 0xa8 bytes at 0x10a3a600: dword +0 = kind,
 * 20 floats from +8 (ascending thresholds), 20 dwords from +0x58. Finds the record of `kind`
 * and the first threshold that is not below `value`; *out = the dword before that slot (the
 * first one when it is the first threshold; the last one when every threshold is below).
 * For a threshold hit, when the dword 0x10145518 + 0xd6 * kind is 0x45, *out is also
 * replaced by (float)Sim_1009c929(10) + *out, and by 45.0f when that is above the float
 * at 0x10111098. Returns 1. When there is no record for `kind` the value is not a clean 0: it is the count (if not positive) or the
 * address behind the last record, as the original leaves it.
 */
int __cdecl Sim_1000ca63(int kind, float value, float *out);

#endif
