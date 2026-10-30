/*
 * Round 10: game functions of HGSIM.DLL around 0x100d0000..0x100dffff, converted one at a time.
 * Calls of functions that are not converted yet go through their original address (SIM_FN).
 */
#ifndef HG_SIM_UNSORTED_100D_W10_H
#define HG_SIM_UNSORTED_100D_W10_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Nothing when the word pane+4 is 0. Otherwise calls the pane's function +0x114 with (pane, the text at
 * 0x10151e74), loads the font ([0x109b9900], key [0x1014cec4], 0x100ba7d5) and, when there is one, draws the
 * text pane+0x10 at the position of [0x109e8934]->+0xf0 on pane+0xec with the routine of the display mode
 * (0x100f2f01 / 0x100f1e10 / 0x100ecea6), releases the font (Sim_100bb2b0) and calls Sim_10051532(pane). */
void __cdecl Sim_100d0198(uint8_t *pane);

/* Unless the point x, y, z is farther from the camera ([0x1014cae0]) than its distance +0x40 (Sim_10084a94):
 * takes a slot of owner (SlotPool_Take) and makes an object for it with Sim_10090c7d(0x100034e0, 0x100034b8, slot,
 * 0) (released again when that fails); slot+4 = object+8, slot+0x14 / +0x1c random in -250..249 (Sim_1009c929)
 * and +0x18 = 0; the object moves with v (record +0x50 = length of v, +0x44 = x, y, z; +0x56e = v), its node
 * is set up (Sim_100aa39f, Sim_100a9d47(node, x, y, z), Sim_100aa799), +0x55e = [0x1015dc78] and the 32 0x2a-byte
 * points from +0x12 get word +0x18 = 1 and the positions k * v / 16 plus a random offset (0..9 - 5, 0..49 - 25,
 * 0..9 - 5). */
void __cdecl Sim_100d965e(uint32_t x, uint32_t y, uint32_t z, float vx, float vy, float vz, void *owner);

/* HUD pane with three lines at the position of [0x109e8934+4]->+0xf0 (0x109e8938): nothing when the word pane+4 is
 * 0 or there is no font. The pane's function +0x114 sets its text from [0x101c4c1c], drawn two font heights down;
 * then sprintf("%.2f m", trunc(((own record)->[0]->+0x5c - own record+0x520) * 1000) / 65536) is drawn three
 * heights down; unless the own record's +0x510 is negative (which leaves without releasing the font), the text
 * [0x101c4c28] is drawn at the position itself, the font is released and Sim_100517dc(pane) follows. */
void __cdecl Sim_100d02bf(uint8_t *pane);

/* Fires a shot of obj (record +0x28) with the weapon definition weapon from the weapon slot slot: unless the record's
 * +0xc3d is set, counts the shot (0x101653f3 for the own unit, 0x1016540c for the own side), takes one round off the slot
 * (+0x20; the clip +0x110 / +0x108 / +0x38; empty: +0x20 = -2, +8 = -1 and 0 is returned) when the unit fires
 * here, and fills the first free of the 20 shots 0x109a3f80 (0x156 bytes, +0x48 = 0) from the weapon, the slot and
 * the muzzle segment (Sim_100d6f16, Sim_1009cd36), with 15 trail parts by the weapon's kind +0x10 (6 / 4 / other).
 * Sim_100d6d6a(obj) first for another unit's shot. Returns 1 (or 0 when the slot ran empty); 0 without obj. */
int __cdecl Sim_100d4f71(uint8_t *obj, const uint8_t *weapon, uint8_t *slot);

/* Aim state of the weapon block b (unit = b[0], flags word +0x560, timer +0x4d8): clears bit 0x80 (unless the unit's
 * weapon is locked: kind 2 or own kind 3 with +0x1e8 set and bit 0x80 set); without a usable selected weapon (+0x4fc,
 * the unit's +0xf0, the definition's +0x86 bit 1) bits 0x8000 and 0x40 go off. In range (above +0x4c, within +0x50 of
 * the aim point) and on target (yaw and side within [0x1012bd30], or a definition with bits 0x20 and 1) bit 0x8000 is
 * set and the lock timer runs (0x16a, down by [0x1015dc80]; at 0 or with record bit 0x20 of +0xbcf bit 0x80 is set).
 * Else bit 0x8000 goes off and the timer runs back up to 0x16a, where bit 0x40 is cleared. */
void __cdecl Sim_100d6833(uint8_t *b);

/* Launches a missile of obj with weapon from slot, as Sim_100d4f71 for a shot: the own unit's launch sound (Sim_10045f3b) and
 * recoil 0x10179cec by the salvo size, the counters, the slot's round count, and the first free of the 0x200 missiles
 * 0x1096fc80 (0x156 bytes) of the weapon's kind filled: guidance (Sim_100c97c0, flags 0x40 / 0x8000), the launch
 * direction (Sim_100d6ea7, Sim_10084cee) times the speed +0x3c, gravity, the trail, the aim 0x100d71b9, the node, a
 * light (Sim_100585e1) and the sound +0x6c; the own unit's lock warning timer +0xc49. 1, or 0 when the slot ran empty;
 * 0 without obj, ammo or a free hard point. */
int __cdecl Sim_100d5446(uint8_t *obj, const uint8_t *weapon, uint8_t *slot);

/* Weapon handling of the unit block u (in mode +0x4f0 == 2): the control byte +0x2d toggles 0x1015243c (message 0x21);
 * unless flag 0x4000 of +0x560 (Sim_100d67af), the selected slot +0x4fc reacts to the trigger bytes of the controls
 * (+0x28 / +0x2e / +0x29..+0x2b / +0x26; flag 1 of +0x560 latches one press); then each of the 8 slots (0x128 bytes
 * at +0x84, kind record 0x10145508 + kind * 0xd6) runs its kind's routine +0 and its state +8: 0 / 3 wait out the
 * reload +0x54, 2 fires through routine +4 while the frame time covers the interval +0x58, with sound and rounds. */
void __cdecl Sim_100d4590(uint8_t *u);

#endif
