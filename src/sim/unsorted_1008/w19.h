/*
 * Round 19: the two node-tree walks of HGSIM.DLL that read an unset local (DECISIONS.md, "Round 19").
 */
#ifndef HG_SIM_UNSORTED_1008_W19_H
#define HG_SIM_UNSORTED_1008_W19_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * The centre of the object obj (node +0x28, children from node +4 linked at +8, each child's object at +0x8c) and its
 * sub-tree, into *x / *y / *z and obj +0x7c.. (Sim_10089066) or +0x60.. (Sim_1008950e): a leaf (no node or no
 * children) gives its own position +0x44..; otherwise the middle (times [0x1012b8e4]) of the box of its own position
 * (none for an object with the flag 0x1000 / 0x800 in word +0) and the centres of the children that answer 1, and
 * obj +0x70.. / +0x54.. gets that centre minus +0x44.. plus +0x38... A leaf with the flag answers 0 without output.
 * Returns 0 for no object or node (outputs 0), else whether the box's minimum x is not the marker 0x7f7fc99e.
 *
 * A leaf compares a minimum it never sets: in the original that is the [ebp-4] that the last non-leaf call at the
 * same depth of the recursion left (all calls at one depth share their frame). The C keeps that value per depth for
 * the walk (exactly as the original within one walk); a depth that had no non-leaf call yet counts as "not the
 * marker" (deviation, DECISIONS.md round 19).
 */
int __cdecl Sim_10089066(uint8_t *obj, uint32_t *x, uint32_t *y, uint32_t *z);
int __cdecl Sim_1008950e(uint8_t *obj, uint32_t *x, uint32_t *y, uint32_t *z);

#endif
