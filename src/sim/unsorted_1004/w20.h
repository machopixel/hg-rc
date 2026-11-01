/*
 * Round 20: game functions of HGSIM.DLL around 0x10040000..0x1004ffff left "not attempted" (DECISIONS.md, "Round 20").
 */
#ifndef HG_SIM_UNSORTED_1004_W20_H
#define HG_SIM_UNSORTED_1004_W20_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * The network "RG" (regeneration) message msg (byte +2: the unit id, +8 for a regenerated one). With [0x1015ecd0] == 1
 * (the host): a message about the own unit is ignored; for a kind-3 unit whose owner (+0xc) is a valid unit the id
 * becomes that owner, without one the unit's +0x164 is cleared and it is removed (Sim_10040564); then a unit with
 * regenerations left ([+0x168] byte) uses one (id + 8 in the message, +0x164 = 1), and the 3 bytes are sent on
 * (Net_DpSend_1007e950 to the id's player, [0x1015d004] +0x16 words, at Sim_1003dbc0's index). Otherwise, for the own
 * unit (id or id + 8) while [0x1015f200] is set: a regenerated one (id >= 8) restarts the own unit (Sim_1003cbb3,
 * Sim_1009d200, Sim_100448cd, the flags 0x1015fdba / fdbd / fdd0 and the state words cleared), else "Out of
 * regenerations" (Sim_1007df6c) with [0x1015f208] / [0x1015ef10] set and the regenerations cleared with
 * [0x1017b8c4] +0xa; another unit below 8 loses its regenerations and +0x164; a regenerated kind-3 unit is
 * restarted (Sim_1009d200, Sim_10011093, its node reset, the flags 0x4800 of +0x1c cleared), "'%s' resurrected." is
 * shown and Sim_10040564 removes the old one.
 *
 * Deviation (DECISIONS.md, round 20): in the two own-unit branches the original clears +0x164 of its gear pointer
 * [ebp-4] before ever setting it (a stale stack value left by the network dispatch); the C stores nothing there.
 * Network round: 1.2's rewritten version (0x1003474e) is the build's; the text above is 1.0's (NETRG_MUTATE).
 */
void __cdecl Sim_10040af8(uint8_t *msg, int who);     /* 1.2: the sender too */

#endif
