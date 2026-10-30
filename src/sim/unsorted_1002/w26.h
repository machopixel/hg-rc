/*
 * Round 26: the beam-shot update (DECISIONS.md, "Round 26").
 */
#ifndef HG_SIM_UNSORTED_1002_W26_H
#define HG_SIM_UNSORTED_1002_W26_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * One frame of the beam shot idx (record 0x109a3f80 + idx * 0x156, filled by Sim_100d4f71 for the lasers, particle
 * accelerators and two autocannons): nothing without +0x48; a shot whose time +0x34 ran out is stopped (+0x48 = 0),
 * the weapon kinds 4 / 6 (weapon record +0x10) last one frame. The firing unit's muzzle segment (Sim_100d6f16) looks
 * for a hit (Sim_1002dda9); the end point goes to +0x18.., the 16 trail points +0x92.. lie evenly from the node's
 * position (Sim_100a9cfb) to it. A hit beyond the weapon's range (+0x50) ends there. A unit hit (bit 0x100 of the
 * hit's word +2): sometimes (weapon flag 8, Sim_1009c929(100) < 10) a running gear with no order falls (Sim_100203cf
 * for the player, else Sim_10015630) for 0x21f; the unit's +0x150 takes the shooter; the shot's +8 gets 0x100 (and
 * 0x1000 with +0x7e bit 0x40); the hit counters of the statistics block; the damage (Sim_1006c161, +0x4e0 of the
 * record gaining the shot's +0x5c) unless a network rule spares it; +8 0x4000 when the player is hit; Sim_10015afd. A
 * building hit (0x200) gets Sim_10025aee with the shot's power +0x70; other hits set +8 bit 0x200 (0x400 in the word)
 * or 0x400. A unit hit makes the effect (Sim_100246b3) at the end point. Deliberate fix (round 26): its camera point
 * is the end point too; the original passes three stack words it never writes.
 */
void __cdecl Sim_10021e8c(int idx);

#endif
