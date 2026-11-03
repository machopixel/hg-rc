/*
 * Round 10: game functions of HGSIM.DLL around 0x100c0000..0x100cffff, converted one at a time.
 * Calls of functions that are not converted yet go through their original address (SIM_FN).
 */
#ifndef HG_SIM_UNSORTED_100C_W10_H
#define HG_SIM_UNSORTED_100C_W10_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* For the current unit (PTRTAB[[0x1017b8c8]]): steps through the targets with Sim_100cbe68 (0 first, then 1)
 * until the selection +0xf0 has bit 0x1000 or comes back to the best one, keeping the one with the smallest
 * distance +0xd8. When there is none, or the smallest is above [0x1012bc48], or Sim_100cb4fb refuses it, the
 * old selection is put back (and, when (unit+0x28)->+0x50c was 1, byte +0x46 of unit+0x54 is cleared). */
void __cdecl Sim_100cbe94(void);

/* Flight time (st(0)) of weapon wpn fired from obj: obj+0x50 becomes the node of the record obj+0x28 picked by
 * wpn+0x11c (dword +0x498 + 4 * it); 0.0 when obj or that node is 0. Otherwise with the unit aim direction
 * (Sim_100d6ea7 + Sim_10084c95), the muzzle speed s = float +0x3c of the weapon kind wpn+4 (0xd6-byte records at
 * 0x10145508), g = -[0x1013abc0] * (int +0x38) * 0.5, the height h = node.y - obj+0x7c and v = s * dir.y:
 * t = (-v - sqrt(v * v - 4 g h)) / (2 g), or (-v + sqrt(...)) / (2 g) when that is negative. */
float __cdecl Sim_100c97c0(void *obj, void *wpn);

/* Nothing when the word pane+4 is 0. Otherwise Sim_100510d9(pane), loads the font ([0x109b9900], key [0x1014cec4],
 * 0x100ba7d5) and, when there is one: calls the pane's function +0x114 with the text [0x101c4c50] (word +0x560
 * of the own unit's record has bit 4 but not 8), [0x101c4c58] (bits 4 and 8) or [0x101c4c60] (no bit 4),
 * setting the byte 0x109b992e to 7, 7 or 0xc; draws the text pane+0x10 at the pane's position (+0xf0) on
 * pane+0xec with the routine of the display mode, sets 0x109b992e to 0xc and releases the font. */
void __cdecl Sim_100cfff3(uint8_t *pane);

/* As Sim_100cfff3, but the first call is Sim_10051372(pane) and the text goes to the position of [0x109e8934]+0xf0. */
void __cdecl Sim_100cfe48(uint8_t *pane);

/* HUD speed readout: nothing when the word pane+4 is 0. speed = trunc(|own record's +0x548..+0x550| (Sim_10084a94)
 * * 6 * 1.38), negated (and colour byte 0x109b992e = 0x14, else 0xc) when its +0x2c is below 0; Sim_100516c8(pane);
 * with the font: sprintf("%3d", speed) and the text [0x101c4c6c] one line lower at the position of [0x109e8928]+0xf0
 * on pane+0xec; with bit 1 of the record's word +0x562, the text [0x101c4c68] at (1, or trunc([0x109e3de8] * 0.75)
 * with colour 4 when +0x613 is set; y = trunc([0x109e3e1c] * 0.92)) on 0x109e3e00; colour 9 and the font released. */
void __cdecl Sim_100cf96c(uint8_t *pane);

/* HUD rate arrows at (x, y): rate below [0x1012bc18] (or unordered) a chevron opening up, above it one opening down, in
 * the colour 0x101659f4, with the opposite chevron in 0x101659e8 every other frame ([0x1017e578]);
 * equal: both chevrons in the second colour. Lines through 0x100ef727. */
void __cdecl Sim_100c83ed(float rate, int32_t x, int32_t y);

/* The weapon gauge of the HUD at (x, y) for obj's selected target (+0xf0: kind 2 or 4, not 0x1000) and weapon (shown when
 * +0xb2 is set and the own unit's selected slot is armed): a guided weapon (+0x86 bit 0x40) shows the target's range
 * (Sim_100c9572, '%3.0f m' within +0x50) and the closing rate arrows (Sim_100c83ed), a locking weapon (bit 1) the lock
 * state text of the own record (+0x560 0x80 / 0x40), any other the out-of-range text every third frame (0x1017e574)
 * when obj+0xd8 is beyond +0x50. */
void __cdecl Sim_100c8ba7(uint8_t *obj, int32_t x, int32_t y, const uint8_t *weapon);

#endif
