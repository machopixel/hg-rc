/*
 * Round 9, letter c: 0x100c4f6c and the HUD text-line writer 0x100ccc60.
 *
 * Unconverted callees, called through their original address (SIM_FN):
 *   0x100ecce0 (font) -> the dword +8 of the font (line height)
 *   0x100f2f01 / 0x100f1e10 / 0x100ecea6 (pane, x, y, font, text, palette 0x109b9920): text
 */
#ifndef HG_SIM_UNSORTED_100C_W9C_H
#define HG_SIM_UNSORTED_100C_W9C_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* ---- w9c_misc.c ---- */
/* Always -1. */
int __cdecl Sim_100c4f6c(void);

/* ---- w9c_hud.c ---- */
/*
 * Writes the entries of kind `kind` (byte +1) of the message table 0x10473800 + [0x1013bb40] * 0x4fc3a (count
 * +0, 0x13f-byte entries from +0x3a) into the panel: entries in use (byte +0x74) whose dword +2 is not
 * 0x8000..0x8002. Each one starts at x = [[panel+0xf0]] with a label by kind (1: [0x101c4994]; 2: [0x101c498c],
 * or [0x101c4994] when exactly one entry is in use and it is of kind 2 while [0x1015ec58] is set and byte 10 of
 * [0x1017b8c4] is 0; 8: [0x101c49a8]; else [0x101c4988]) in colour 0x14, then the text +0xad in colour 0xc:
 * on one line when shorter than 0x20, else cut at 0x20 or at the last space before it and written on two lines
 * (the second one a line height, 0x100ecce0(font), lower). A right-aligned mark follows at the panel's width
 * ([pane+0xc] - [pane+4] less its width Sim_100ccb0d): [0x101c49a0] in colour 0x17 for byte +0 = 5, [0x101c499c]
 * in 7 for 6, [0x101c4980] in 0xc otherwise. The colour byte 0x109b992e ends at 9; pos (x, y) moves down one
 * line per entry (two for a cut text). The texts go through 0x100f2f01 while [0x101668e8] is set, else
 * 0x100f1e10, on the pane [panel+0xec].
 */
void __cdecl Sim_100ccc60(void *panel, int32_t *pos, uint32_t font, uint32_t kind);

#endif
