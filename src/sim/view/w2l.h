/*
 * Wave 2l: object link maintenance and two rect animations around
 * 0x10088d74..0x1008f867. An object keeps three pairs of links that are
 * always changed together: the ObjList pair (next +0x14, prev +0x18, see
 * objlist.c), a second pair (prev +0x1c, next +0x20) and a third pair (prev +4,
 * next +8); the cell chain of cell_link.c (prev +0xc, next +0x10) is a fourth.
 * Plain C, original cdecl convention.
 */
#ifndef HG_SIM_VIEW_W2L_H
#define HG_SIM_VIEW_W2L_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Links node after `after` in the list whose links are prev +0x1c / next +0x20.
 * Does nothing when either is NULL, when node already has a link, or when the
 * object at node+0x28 has a non-zero first dword.
 */
void __cdecl ObjList2_InsertAfter(void *node, void *after);
/* Unlinks node from that list and clears both of its links. NULL is ignored. */
void __cdecl ObjList2_Unlink(void *node);

/*
 * Removes node from all three lists: the pair at +4 / +8 is relinked around it
 * and cleared, with [0x1017c8c4] set the dwords +0x8c and +0x90 become 9999,
 * then ObjList2_Unlink and ObjList_Unlink.
 */
void __cdecl Sim_1008a871(void *node);
/* Inserts node after `anchor` in the +4 / +8 pair, then ObjList2_InsertAfter and List_InsertBefore with the same two. */
void __cdecl Sim_1008a8f5(void *node, void *anchor);
/* Sim_1008a8f5(node, 0x1017ba00) for a non-NULL node. */
void __cdecl Sim_1008a14f(void *node);
/*
 * Unless node is NULL, the value of the dword [0x1017c8cc] or the address 0x1017b9d8: Sim_1008a871(node),
 * then the chain with prev +0xc / next +0x10 is relinked around the node and both of its links are cleared,
 * then ObjList_Unlink(node).
 */
void __cdecl Sim_1008a16d(void *node);
/* When [0x1017c8cc] is the address 0x1017b988 and node is not NULL: Sim_1008a871(node), Sim_1008a8f5(node, 0x1017c818). */
void __cdecl Sim_1008a1ea(void *node);
/*
 * Unless node is NULL or already has bit 0x1000 in its first word: ObjList_Unlink, sets that bit and, when
 * node+4 is non-zero, Sim_1008a871(node) and Sim_1008a8f5(node, 0x1017b9d8).
 */
void __cdecl Sim_1008a40a(void *node);

/* out[0] = (in[0] << 16) / 319 and out[1] = (in[1] << 16) / 199, signed, truncated toward zero. Returns out. The original raises a divide error when a quotient does not fit 32 bits; this version wraps. */
int32_t *__cdecl Sim_1008d07b(const int32_t *in, int32_t *out);
/*
 * Same animation as Sim_1008f747, in two phases: pair holds a state block N and an animation block O.
 * Only when N+0 is 1 does it work: out = O+0xc is the rect it writes (also the return value, 0 otherwise).
 * With flip 0 A = O+4 and B = O+8, otherwise they are swapped. When N+4 is 0, out = A and N+8 = 0. Otherwise
 * N+8 grows by the dword 0x1015dc80 and t = (N+8 << 16) / O+0. For t >= 0x10000 out = B, N+8 = 0 and N+0
 * becomes 0. For t <= 0x8000 out = A and Rect_Interpolate(A, B, out, 2t, 1) moves the x pair; above that
 * out = B and Rect_Interpolate(A, B, out, 2(t - 0x8000), 2) moves the y pair. Ends with N+4 = old N+0 and
 * N+0 = the new state. The original raises a divide error for O+0 == 0 or a quotient outside 32 bits; this
 * version does not.
 */
int32_t *__cdecl Sim_1008f867(int flip, const uint32_t *pair);

#endif
