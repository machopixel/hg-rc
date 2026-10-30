/*
 * Round 10: game functions of HGSIM.DLL around 0x10060000..0x1006ffff, converted one at a time.
 * Calls of functions that are not converted yet go through their original address (SIM_FN).
 */
#ifndef HG_SIM_UNSORTED_1006_W10_H
#define HG_SIM_UNSORTED_1006_W10_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* 0 when ent or obj is 0. Otherwise, with node = (*ent)+0x48: p = Sim_100a9cfb(node), q = Sim_100a9ca3(node),
 * r = Sim_10068e1d(x, y, z, -p), r = Sim_10068d76(r, -q), and returns the original 0x100687a3(*ent + 0x284,
 * r, float obj+0x53c, outX, outY, outZ). NOTE: 0x100687a3 stays original (it reads three of its own locals
 * before writing them), so its result depends on what earlier calls left on the stack, here as in the game. */
int __cdecl Sim_100688a3(const uint8_t *obj, const uint32_t *ent, float x, float y, float z, float *outX,
                         float *outY, float *outZ);

/* Destroys part (1..16) of the damage block b (unit = b[0], parts at +0x88, 0x3c bytes, flag 0x2000 of +0x36): the part
 * is cleared and marked, then hidden on the unit's node (Sim_100ab054, or Sim_100aafe4 for a soft hit or a kind-9
 * unit). Part 1 of a kind-1/9 unit scores the kill (Score_RecordKill) where it counts, else the part comes back; parts
 * 2 and 7 take parts 3, 4 and 1 with them and throw debris (Sim_10092f9d at the points +0x4c0 / +0x4c4); parts 5 and 6
 * cripple the unit (+0x560 bit 0x20, a fall direction +0xc19 / +0x562, +0x570, Sim_100203cf for the own unit). The
 * last part kills the unit unless it is flagged or already down (+0x4f0 4 or 5). */
void __cdecl Sim_1006ad98(int32_t killer, uint8_t *b, int32_t part, int32_t flag);

/* Resets obj's unit for a (re)start (record = obj+0x28, unit = record[0]): another player's unit leaves its slot
 * (Sim_1009d8b9; the own unit rebuilds the cockpit, Sim_1002bca0), the radius +0x53c from the model, the seven
 * interpolators of the record restarted (Sim_10047b76; rates by owner and kind), the record's state cleared, the node
 * placed and read back (Sim_100a9d7c / Sim_100aa799 / Sim_100a9ca3 / Sim_100a9cfb), the box of a kind-10 object
 * (Sim_1002fd4c) or a kind-4 object's model check (Sim_1006d911) into +0x29c, the ground height +0x7c (Sim_1002d961),
 * +0x154 (Sim_100ac21a), the controls, Sim_1004f0d6, Sim_100aa454, the start throttle of a demo, Sim_10011093 (a network
 * kind-3 unit as the own player's) and record+0x4d4 / [0x1013abc4]. */
void __cdecl Sim_1006da46(uint8_t *obj);

/* The weapon label of the HUD for the selected weapon slot of rec (+0x4fc): the slot's state text (localised strings
 * 0x109e854c.., the second set while [0x10176930] is set) placed at the aim point (Sim_10064824, Sim_1006519d) and the
 * weapon gauge (Sim_100c8ba7). A simple weapon of an armed slot shows the 'locked' text and returns 1 when the selected
 * target is within the weapon's range (+0x4c..+0x50) and within 3 of yaw and pitch. */
int __cdecl Sim_10062b7c(uint32_t *rec, float yaw, float pitch, float range);

/* A hit on system cell k of part part of the damage block b (codes at +0x617 + part * 0x50, k words; only for kind-1 units):
 * the code (0x1b8a..0x2328) breaks a system - the cockpit message for the own unit (Sim_100203cf), the system's state
 * fields of b (+0xbab..+0xc4d, the throttle factor +0x500, the repair timer +0xc39) - an empty cell passes the hit on
 * to the next one (up to +0xb67 + part * 4), 0x1b8a destroys the part (Sim_1006abf6); the cell is cleared. */
void __cdecl Sim_1006b52f(int32_t killer, uint8_t *b, int32_t part, int32_t k, int32_t flag);

/* A shot hits part (1..17) of the damage block b: armour +0 and structure +8 of the part (0x3c bytes at +0x88) take the
 * shot's damage (+0x70 less the armour, times +0x74; kinds +0x7c 1..4 split it; a shot that does not pierce takes +0x78
 * off the armour), with sparks (Sim_100939a3) and the hit sound; a structure hit destroys the part at 0 (Sim_1006abf6)
 * or hits two systems (Sim_1006b52f), and the part's damage level goes to the node (Sim_100aae7b). */
void __cdecl Sim_1006c161(int32_t killer, uint8_t *b, int32_t flag, uint8_t *shot, int32_t part);

#endif
