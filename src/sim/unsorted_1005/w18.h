/*
 * Round 18: game functions of HGSIM.DLL around 0x10050000..0x1005ffff that read unset stack (DECISIONS.md, "Round 18").
 */
#ifndef HG_SIM_UNSORTED_1005_W18_H
#define HG_SIM_UNSORTED_1005_W18_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Walks the node tree of a gear design (GDF items: +0x54 next, +0x78 sub-tree, +0x7c six names of 0x20 bytes, word
 * +0x10, position +0x15c.., rows of 0x48 bytes from +0x168) from node with the name slot field (the next slot when
 * that name starts with 'g' / 'G'). A named node gets the index *counter (then counted up) and becomes a sub-unit:
 * Sim_1005aa45(name, index, parent, -1, word +0x10, 0x100 or 0x170 when its upper-cased copy (Sim_1010c9f0,
 * Crt_StrUpper) contains "POF_", "EYE_", "SOL_" or "DUM_", transform, field), the transform being 1, 1, 1, 0, 0, 0
 * and row `row` of table (+0x24..+0x2c) minus the node's position. Then the next node (same parent, row + 1) and
 * the sub-tree (parent = this index, table = node +0x168, row 0). A nameless node only counts *counter and
 * [0x101c42a0] when its first name is not empty, and walks its next node and its sub-tree the same way.
 *
 * Deviation (DECISIONS.md, round 18): the original hands a nameless node's sub-tree its never-written local
 * [ebp-0x10] as the parent. That value would reach Sim_1005aa45 only if a named node sits in that sub-tree's
 * next-chain (or below more nameless nodes); none of the 78 shipped gear designs has that shape
 * (tools/round18/gdfscan.py), so the C passes -1 (no parent) there.
 */
void __cdecl Sim_1005ab44(uint32_t parent, int32_t *counter, const uint8_t *table, int32_t row, uint8_t *node,
                          int32_t field);

#endif
