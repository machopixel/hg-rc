/*
 * Round 5, letter a: functions of 0x100203cf..0x1002feef that call helpers which are not converted yet.
 * Those helpers are called through their address (SIM_FN) with the prototypes below.
 */
#ifndef HG_SIM_UNSORTED_1002_W5A_H
#define HG_SIM_UNSORTED_1002_W5A_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Not converted: (pointer, pointer or 0, -1). */
#define W5A_CALL_1001f790 SIM_FN(int(__cdecl *)(void *, void *, int), 0x1001f790)
/* Not converted: one dword. */
#define W5A_CALL_10029b1f SIM_FN(int(__cdecl *)(uint32_t), 0x10029b1f)
/* Not converted: (object, six pointers into the 0x18-byte block of Sim_1002f9e0). */
#define W5A_CALL_1002fba5 SIM_FN(int(__cdecl *)(void *, void *, void *, void *, void *, void *, void *), 0x1002fba5)

/*
 * With sub = the object at +0x28 of the record of the current player (pointer table 0x109e8b40, index
 * [0x1017b8c8]) calls 0x1001f790(0x10133a10 + 0x420 * sub[0x613] + 0x18 * a,
 * b == -1 ? 0 : 0x10133950 + 0x60 * sub[0x613] + 0x18 * b, -1).
 */
void __cdecl Sim_100203cf(int a, int b);
/*
 * Entry i of the 0x14-byte entries at 0x109f09e0. When its dword +4 (an object) is not 0: 0x10029b1f(object),
 * the function at entry +0x10 (when not 0) called with the object, Sim_1002997e(object) and, unless that is -1,
 * Sim_1002990f(result); then the five dwords at 0x1015dc48 replace the entry.
 */
void __cdecl Sim_10029125(int i);
/*
 * After Sim_1002fa3f(obj): 1 when the float a lies within the first two floats of the block at obj + 0x98
 * and the float b within its floats +0x10 and +0x14 (limits included), otherwise 0 (0 without a block).
 * The third argument is not used.
 */
int __cdecl Sim_1002d1c0(void *obj, float a, int unused, float b);
/*
 * Gives the block at obj + 0x98 (when the object is not null and the word at +0 does not have bit 0x200)
 * its 0x18-byte block if it has none (Sim_1002f9e0), calls 0x1002fba5(obj, block, block + 4, +8, +0xc,
 * +0x10, +0x14) and sets bit 0x200 of the word at obj + 0.
 */
void __cdecl Sim_1002fa3f(void *obj);
/*
 * Frees the block at obj + 0x98 according to the dword obj + 0x34: 0 HeapFree, 5 Sim_10031917, 8
 * Sim_100305f8, any other value nothing; then clears obj + 0x98. Nothing for a null object or block.
 */
void __cdecl Sim_1002feef(void *obj);

#endif
