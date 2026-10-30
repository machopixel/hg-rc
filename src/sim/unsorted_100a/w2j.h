/*
 * Wave 2, letter j: leaf functions of HGSIM.DLL around 0x100a0000..0x100b0000.
 */
#ifndef HG_SIM_UNSORTED_100A_W2J_H
#define HG_SIM_UNSORTED_100A_W2J_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * obj+4 is the first node of a chain (next at +8). Returns the node `n` places
 * before the last one (0 = the last), or 0 when the list is empty, obj is null
 * or the chain is not longer than n.
 */
void *__cdecl Sim_100aac3d(const void *obj, int n);
/*
 * Appends node to the list whose first and last node are held in *head and
 * *tail (node+0xc = previous, node+0x10 = next). Does nothing for a null node.
 */
void __cdecl Sim_100abab0(void *node, void *head, void *tail);
/*
 * When the signed dword 0x1017d1c4 is >= 0 takes the node with that index from
 * the table of dword indexes at 0x101c56c0 (0x34-byte nodes at 0x101c57e0),
 * appends it with Sim_100abab0 to the list at dwords 0x1017d1d0 (first) and
 * 0x1017d1d4 (last), and decrements the dword 0x1017d1c4. Returns the node, or 0.
 */
void *__cdecl Sim_100abc5d(void);

#endif
