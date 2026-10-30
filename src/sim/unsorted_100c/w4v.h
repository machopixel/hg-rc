/*
 * Round 4, letter v: the callback list of HGSIM.DLL around 0x100cc6ea. The list is a pointer to the first node (the
 * list head dword); a node is 0x18 bytes: +0 callback, +4 heap block (freed with the node), +8 interval, +0xc tick of
 * the last successful run, +0x10 tick of the next run, +0x14 next node. The current tick is [0x1015dc78]; [0x1017e5b8]
 * holds the node being worked on. A callback is cdecl (command, arg, tick, interval) and returns 0 to be removed:
 * command 0 = on creation, 1 = run, 2 = on removal.
 */
#ifndef HG_SIM_UNSORTED_100C_W4V_H
#define HG_SIM_UNSORTED_100C_W4V_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

typedef int(__cdecl *TimerCallback)(int command, int arg, int tick, int interval);

/*
 * Adds a node at the head of `list` (callback, interval, last = next = the current tick) and, when the callback is not
 * NULL, calls callback(0, arg, tick, interval); when that returns 0 the node is removed again (Sim_100cc79a) and NULL
 * is returned. Returns the node, or NULL when the allocation fails. [0x1017e5b8] = the node.
 */
void *__cdecl Sim_100cc6ea(uint32_t *list, TimerCallback callback, uint32_t interval, uint32_t arg);
/*
 * Unlinks `node` from `list` (nothing at all for NULL), sets [0x1017e5b8] = node, calls its callback with
 * (2, 0, tick, interval) and frees the node's heap block (+4). The node itself is not freed.
 */
void __cdecl Sim_100cc79a(uint32_t *list, void *node);
/* Sim_100cc79a on every node of the list. */
void __cdecl Sim_100cc830(uint32_t *list);
/*
 * Runs the due nodes: for each node with a callback whose next-run tick (+0x10) is not after the current tick, sets
 * it to tick + interval and calls callback(1, 0, tick, interval); a result of 0 removes the node (Sim_100cc79a), else
 * its last-run tick (+0xc) becomes the (re-read) current tick. [0x1017e5b8] is the node being visited.
 */
void __cdecl Sim_100cc8c5(uint32_t *list);

#endif
