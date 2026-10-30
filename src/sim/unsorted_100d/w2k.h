/*
 * Wave 2 (letter k) additions to unsorted_100d: two leaf functions of
 * HGSIM.DLL at 0x100d1648 and 0x100d2c4f. Names carry the original address
 * because the code alone does not say what the data means.
 */
#ifndef HG_SIM_UNSORTED_100D_W2K_H
#define HG_SIM_UNSORTED_100D_W2K_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Moves a chain of nodes into the doubly linked list owner (first node at +0xc,
 * last at +0x10; nodes link to the previous one at +4 and to the next at +8).
 * chain is any node of the chain: the walk goes back through +4 to its first node
 * and forward through +8 to its last node. Every node from the first one
 * onwards gets owner in its +0x14; the chain is put in front of the owner's list
 * (owner+0xc = first node, the old first node follows the last one; owner+0x10
 * is set to the last node when it was 0). Returns the first node of the chain.
 */
void *__cdecl Sim_100d1648(void *owner, void *chain);
/* Whether bit 0x1000 of the word at obj is set. */
int __cdecl Sim_100d2c4f(const void *obj);

#endif
