/*
 * Round 14: game functions of HGSIM.DLL around 0x10090000..0x1009ffff that were left original as callers of the model
 * loader, settled by the shipped data (DECISIONS.md, "Round 14").
 */
#ifndef HG_SIM_UNSORTED_1009_W14_H
#define HG_SIM_UNSORTED_1009_W14_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * (Re)spawns the gear of group index (0..0x18, its record 0x109e8b40[index] +0x28), unless force is 0 and the group's
 * word +0x1c lacks bit 1: the record's motion and the control block +0x54 cleared, +0x520 = +0x588, the model rebuilt
 * from its design (Sim_100599a0 of the name +0xbeb, Sim_1009d90e, Sim_10072e35, Sim_1005cd4a; [0x1014c99c] = index
 * meanwhile), the node refreshed, the unit flags reset (word +0x1c & 0xbff8 | 0x10), Sim_1007caab in a network game,
 * Sim_100500ab and Sim_1006da46. A start point is drawn from the unit's start zone (0x109b8420 + side * 0x74:
 * Sim_1009c929 of its count, never the last one +0x280, and the next one while another group stands on it, moved by
 * the side's matrix 0x109b7d7c with Mat3x4_TransformPoint), the node placed there on the ground (Sim_1002d961, less
 * the lower of the two feet nodes +0x4c0 / +0x4c4), and the player's view state reset (Sim_1002c113). Returns 1, or 0
 * when there is nothing to spawn.
 */
int __cdecl Sim_1009d200(int32_t index, int32_t force, uint32_t unused3, uint32_t unused4);

#endif
