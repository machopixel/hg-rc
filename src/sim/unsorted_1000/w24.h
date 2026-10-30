/*
 * Round 24: the AI line-of-fire check 0x1000aa9e (DECISIONS.md, "Round 24").
 */
#ifndef HG_SIM_UNSORTED_1000_W24_H
#define HG_SIM_UNSORTED_1000_W24_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * The two values the original 0x1000aa9e reads from its never-written [ebp-4] / [ebp-0x64]. A watched original run
 * (round 24, 223 of 223 reads) showed who leaves them: the unit's own target check just before, Sim_100cb4fb's call
 * of Sim_100cada2, whose y difference ([ebp-0x3c]) lands on [ebp-4] and whose Sim_100cbc6b call's first angle
 * ([ebp-0x10] there) lands on [ebp-0x64]. Sim_100cada2 keeps both of a call that got that far (g_w24TargetSet), and
 * Sim_100cb4fb hands them on to g_w24AimDz / g_w24AimDy, which 0x1000aa9e uses. 0 until the first target check.
 */
extern float g_w24TargetDy, g_w24TargetAngle;
extern int g_w24TargetSet;
extern float g_w24AimDz, g_w24AimDy;

/*
 * Whether unit has a clear shot at target: Sim_100cbc6b(target z - unit z, g_w24AimDy, g_w24AimDz) for the yaw and
 * the range, a = fmod(target[0x68] + target[0x74] - yaw + f[0x10111030], 360); with check set and
 * f[0x10111038] < a < f[0x10111034] returns 0. Builds the segment of target (Sim_100d6f16, cut to the range by
 * Sim_1009cd36) and looks for a hit (Sim_1002dda9, else Sim_100c2851 with target[4]): no hit gives whether target's
 * [0xf0] is unit's id | 0x200; a hit node with bit 0x100 in word +2 (or bit 0x200 with bit 0x400 in the id) gives
 * whether its word +0x24 is the id's low byte; otherwise 0.
 */
int __cdecl Sim_1000aa9e(void *target, void *unit, int check);

#endif
