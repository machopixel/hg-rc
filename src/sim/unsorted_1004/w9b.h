/*
 * Round 9, letter b, unsorted_1004: the periodic per-player network work of the message pump
 * (Net_ProcessMessages, unsorted_1003/w9b.h).
 * Helpers that are not converted are called through their original address with SIM_FN and the prototype below.
 */
#ifndef HG_SIM_UNSORTED_1004_W9B_H
#define HG_SIM_UNSORTED_1004_W9B_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Not converted: respawns the unit of player `index` (index, force, 0, 0). */
#define W9B4_CALL_1009d200 SIM_FN(int(__cdecl *)(int, int, int, int), 0x1009d200)

/*
 * For every entry of the player table (count [0x1015dc64]): clears the "dead" flag +0x164 of entries that are
 * no longer dead or hit (word +0x1c bits 2 and 4); once per [0x1013ac9c] ticks sends the state, the weapon and
 * the gear messages of the entries this player drives (state 3, owner +0xc self) and announces their death
 * (text "'%s' %s [...]" via Sim_1007df6c) and, with respawns left (byte at +0x168), their resurrection (Sim_10040ab8
 * as a client, else 0x1009d200 and the rebuild of the unit); as the host ([0x1015ecd0] 1) hands over driven entries
 * whose block has bit 2 of +0x4f0 to the next player of the list at [0x1015d004]+0x126 that is free ("CA" / "TC"
 * message of 0x23 bytes, Sim_10040645 for the record); and reports a driven entry that targets (+0xf0 kind 0x200)
 * another player as taken ("TC").
 */
void __cdecl Net_UpdatePlayers(void);

#endif
