/*
 * Round 13: game functions of HGSIM.DLL around 0x100b0000..0x100bffff whose last original callee was converted in
 * round 12.
 */
#ifndef HG_SIM_UNSORTED_100B_W13_H
#define HG_SIM_UNSORTED_100B_W13_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Order step of a unit by its state word +0x1f8. Clears the flags [0x1017e2a8] / [0x1017e2ac] / [0x1017e2b0]. State
 * -1: +0x1f8 = Sim_100bf186(unit), and Sim_100bf28e(unit) when that is not -1; nothing else. States 0..0x1b run
 * their step (0x100c0670, 0x100c0965, 0x100c1c67, ...; 1: the formation slot move with Sim_1001403a / Sim_100c3d16 /
 * Sim_100c00c7 / Sim_100c07c5 and the slot counters at +0x270 of the leader; 3: also steers block +0xc by
 * +0x218 * 450 without a contact and flips +0x218 every 0x21f ticks; 14, 16, 17, 22, 23 set the result word +0x22c),
 * each deciding "done". Afterwards done is also set by a hit for a type-1 unit (Sim_100c3eb5 of the record, flag
 * 0x1017e2a8), by the timer +0x200 running out (flag 0x1017e2ac) and by a pending order +0x1fc != -1 (flag
 * 0x1017e2b0); a type-1 unit without one in a state other than 0, 0x10, 0x11, 0x18..0x1b picks one from the table
 * 0x10151b88 (Sim_1009c929(0x40) >> 4, kept in [0x1017e2a4]) when Sim_1009c929(0x61) < 3. Done: Sim_100bfa33(unit).
 */
void __cdecl Sim_100be0f0(uint8_t *unit, int16_t key);

#endif
