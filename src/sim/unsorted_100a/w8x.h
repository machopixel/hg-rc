/*
 * Round 8, letter x: the scene pass at 0x100a4845..0x100a4d8e. Objects (flags word +0, node +0x28 with children
 * from +4, next +8, object +0x8c) are offered to the display routine [0x1014cb40] (counted in [0x101cc2b4]) and,
 * when it declines them, drawn with Sim_100a4510 (counted in [0x101c7a9c], range [0x101c7aa0]).
 * Helpers that another hook file patches (or that are not converted yet) are called through their original
 * address with SIM_FN and the prototypes below.
 */
#ifndef HG_SIM_UNSORTED_100A_W8X_H
#define HG_SIM_UNSORTED_100A_W8X_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Patched by hooks_view.c: marks the visible cells for the camera. */
#define W8XA_CALL_10087d83 SIM_FN(void(__cdecl *)(void *), 0x10087d83)
/* Patched by hooks_view.c: sorts the draw list entries first .. last (8 bytes each) by depth. */
#define W8XA_CALL_100a4700 SIM_FN(void(__cdecl *)(void *, void *), 0x100a4700)
/* Not converted: draws one entry of the draw list. */
#define W8XA_CALL_100a539e SIM_FN(void(__cdecl *)(uint32_t), 0x100a539e)

/*
 * Offers obj (unless its flag 0x1000 is set) and then the objects of its node's children, depth first; returns 1
 * as soon as drawing stopped ([0x101c9ea4] cleared, or [0x1017d13c] reached [0x1017d140]), else 0.
 */
int __cdecl Sim_100a4845(void *obj);
/*
 * As Sim_100a4845 for a top-level object without its flag test; the player's own object (word +2 bit 0x100, word
 * +0x24 = [0x1017b8c8], [0x1015ef0c] set, mode [0x1015d1ac] not 4) instead offers the object of the player's
 * record +0x28 +0x490 with the near plane (camera +0x3c and [0x10185c50]) set to 8.0 for that time.
 */
int __cdecl Sim_100a4933(void *obj);
/*
 * The scene pass: draw list at [0x1017d168] + [0x1017d134] * 8 (Sim_100a88db first when [0x1017d134] is 0),
 * counters cleared, visible cells marked (0x10087d83), every list of the table 0x1036f2a0 (until a null entry)
 * offered object by object (+0x20 chain from +0x20) until one stops, the entries sorted (0x100a4700) when more
 * than one, [0x101c9ea0] = their count, and outside the high-colour mode ([0x1017b940] = 0) each drawn with
 * 0x100a539e. A list whose +8 is 0 or equal to its +4 is retried for ever, as in the original.
 */
void __cdecl Sim_100a4d8e(void);

#endif
