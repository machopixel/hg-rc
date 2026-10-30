/*
 * Round 5, letter c: functions at 0x1005264d and 0x1005af06 that call helpers which are not converted yet.
 * Those helpers are called through their original address (SIM_FN) with the prototypes below.
 */
#ifndef HG_SIM_UNSORTED_1005_W5C_H
#define HG_SIM_UNSORTED_1005_W5C_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Not converted: takes the second argument of Sim_1005264d. */
#define SIM_CALL_100203b8 SIM_FN(void(__cdecl *)(int), 0x100203b8)

/* Records a (< 16) and b (< 32): stores b in the dword at 0x109b7d58 + a*0x6c and copies the 0x74-byte entry b of 0x109b68e0 to entry a of 0x109b8420 (calls 0x100203b8(b) first when a equals [0x1013bb40]). */
void __cdecl Sim_1005264d(int a, int b);
/* Depth-first search of the tree (child at +4, sibling at +8) for the node whose string at +0x90 equals the name; returns the node or NULL. */
void *__cdecl Sim_1005af06(void *node, const char *name);

#endif
