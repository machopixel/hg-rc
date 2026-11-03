/*
 * Round 9, letter c: the two timed message bars at 0x10149d28 (0x1007e0ff).
 *
 * Unconverted or patched callees, called through their original address (SIM_FN):
 *   0x100ba7d5 (name, extension, out) -> resource handle (hooks_conv.c traces it)
 *   0x100f0205 (patched by hooks_hud.c) / 0x100e91dc (pane, shape, 0, x, y): shape blit
 *   0x100f2f01 / 0x100f1e10 / 0x100ecea6 (pane, x, y, font, text, palette 0x109b9920): text
 */
#ifndef HG_SIM_UNSORTED_1007_W9C_H
#define HG_SIM_UNSORTED_1007_W9C_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* ---- w9c_messages.c ---- */
/*
 * For each of the two 0x3c-byte records at 0x10149d28 whose dword +0xc is set: while the timer Sim_1002ab9d() is
 * below the record's +0x34, loads the font +0x14 (extension [0x1014cec4]) and the shape +0x24 ([0x1014cec0]),
 * blits the shape at the pane +0x38 (0x100f0205) and writes the text
 * +0 at (+4, +8) with colour 0xfe (0x100f2f01 while [0x101668e8], else 0x100f1e10),
 * colour 9 afterwards; once the timer has run out +0xc becomes 0.
 */
void __cdecl Sim_1007e0ff(void);

#endif
