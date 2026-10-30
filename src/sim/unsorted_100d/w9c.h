/*
 * Round 9, letter c: the weapon panel writer 0x100d3d10 (stored at +0x13c of the eight weapon panels of
 * 0x109e88e0 by 0x1002b68f).
 *
 * Unconverted or patched callees, called through their original address (SIM_FN):
 *   0x100ba7d5 (name, extension, out) -> resource handle (hooks_conv.c traces it)
 *   0x100f0205 (patched by hooks_hud.c) / 0x100e91dc (pane, shape, 0, x, y): shape blit
 *   0x100f2f01 / 0x100f1e10 / 0x100ecea6 (pane, x, y, font, text, palette 0x109b9920): text
 */
#ifndef HG_SIM_UNSORTED_100D_W9C_H
#define HG_SIM_UNSORTED_100D_W9C_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* ---- w9c_hud.c ---- */
/*
 * For the weapon [panel+0xc] (a 0x128-byte record of the array +0x84 of the block +0x28 of the current player's
 * PTRTAB entry) while [0x101355b8] is set and the record's +4 (its kind) is not negative: loads the font
 * [0x109b98c0] (extension [0x1014cec4]; nothing without it), Sim_1008d405(pane) for the selected weapon
 * ([block+0x4fc]), blits the shape [0x109e84b8] (extension [0x1014cec0]) at the pane and, while [0x101c64f0] is
 * set, the kind's shape (name at 0x10145508 + 0xd6 * kind, +0xa2 for a record whose +8 is -1, else +0x92) at
 * ([[panel+0xf0]], [[panel+0xf0]+4]); each loaded shape is released again (Sim_100bb2b0). Then writes the name
 * panel+0x10 at (+0xd4, +0xd8) and, when the record's +0x20 is positive, that number at (+0xdc, +0xe0), in the
 * colour given by the record's +8: -1 -> 0xf0, 0 -> 0x14, 2 -> 4, 1 -> by +0xc (0: 0xf, 1: 0x10, 2: 7, 3: 8,
 * 4: 0x17), anything else 0xf; frames the selected weapon's pane with it (Sim_1008d4f1), sets the colour byte
 * 0x109b992e back to 9 and releases the font.
 *
 * The original leaves the colour uninitialised for +8 = 1 with +0xc above 4 (no case in its switch); it is 0
 * here. The shape blit goes through 0x100f0205 while [0x1017b940] is set, else 0x100e91dc; the texts through
 * 0x100f2f01 while [0x101668e8] is set, else 0x100f1e10 / 0x100ecea6 by [0x1017b940].
 */
void __cdecl Sim_100d3d10(void *panel);

#endif
