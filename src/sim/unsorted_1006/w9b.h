/*
 * Round 9, letter b, unsorted_1006: a quaternion from three axis angles, the flashing text of the HUD and the
 * kill tally of the multiplayer score.
 */
#ifndef HG_SIM_UNSORTED_1006_W9B_H
#define HG_SIM_UNSORTED_1006_W9B_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * out = qz * (qy * qx) with qx, qy, qz the rotations (Sim_100610ae) by a, b, c about the x, y, z axes; `order`
 * picks which angle's rotation sits where: 1 = x inner, y middle, z outer; 0 = y inner, x middle, z outer;
 * 2 = x inner, z middle, y outer. Any other order gives the identity (0, 0, 0, 1). All float work is done by
 * the two callees; this function only moves the values.
 */
void __cdecl Quat_FromAxisAngles(float a, float b, float c, int order, float *out);
/*
 * Starts the flashing text (0x101658c0.. : 0x389 ticks, counter 0, period 0x5a, 0x389 / 0x5a flashes, the text
 * number, visible) when [0x10179ce0] is set, copying the text of `which` into 0x101658d8: 0 .. 4 and 0xa, 0x14,
 * 0x15 pick one of the pointers [0x101c4eb0], [0x101c4e7c], [0x101c4e94], [0x101c4e58], [0x101c4e64],
 * [0x101c4e40], [0x101c4e48], [0x101c4560]; any other number the empty text at 0x10176958.
 */
void __cdecl FlashText_Start(uint32_t which);
/*
 * Records the death of the unit whose block is `block` (its player entry at block[0]) killed by player `killer`
 * (-2 for "left the game", -1 nobody): counts it in the 0x101653xx / 0x101654xx / 0x101655xx tallies (per team of
 * Sim_10052be7, per unit class block+0xbdb, for the two team slots of Sim_10015c61(1) / (2)), updates the score
 * [0x10179cac] and the ratings table 0x1016552c (Sim_100413c7), starts the matching sound (Sim_100203cf) for
 * own kills, marks the entry dead (bits 2 and 4 of +0x1c, block mode 4), announces a new group leader
 * (Sim_1007df6c, Sim_100128dd, Sim_1005f9aa) and releases the unit (Sim_1001150d, Gear_SetLod, Sim_100ac58e).
 * Nothing when the own, replaying ([0x1017b8c4] byte 1) unit dies in a mode other than 7.
 */
void __cdecl Score_RecordKill(int killer, uint8_t *block);

#endif
