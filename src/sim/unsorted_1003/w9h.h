/*
 * Round 9, letter h: the object colour code of the view modes (0x10032a10) and the per-frame view reset that
 * blanks the screen in the sensor modes (0x100352d5).
 */
#ifndef HG_SIM_UNSORTED_1003_W9H_H
#define HG_SIM_UNSORTED_1003_W9H_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* ---- w9h_view.c ---- */
/*
 * The colour word of an object for the current view mode [0x1014cb1c] (0 normal, 1..4 sensor modes) and overlay
 * [0x1014cb20]: `colour` is the object's own word (kind in bits 12..15, two nibbles below), the flags are the word +2
 * of the record at obj +0x24 (its word +0 holds a level in bits 4..7, its dword +0x34 a class). Modes 1..4 answer
 * fixed palette indices (0x17 for flag 0x200, 7 for 0x400, 0x1f / 4 / 0x10 / 0x17 by level, 1 otherwise); mode 3 adds
 * (Sim_100591ff(obj, b, 0xff, d) * 48 / 15) >> 4 and 0x1000, mode 4 answers 0x10 for a Sim_1009c929(100) above 20 when
 * [0x10179ca4] is set. Otherwise (and in the other modes) the colour's kind picks the shading: kind 0 keeps the nibbles
 * (lo >> 4, mid >> 4; [0x1014cb28]: lo >> 6 and 0xf0); 0x8000 / 0x9000 keep the colour; 0x3000 keeps it unless
 * [0x1014cb38] has a flag bit (then kind 0x1000, level = colour & 0xf0); 0x2000, 0x5000..0x7000, 0xb000..0xe000 use
 * level 0xff (kind 0x1000 when [0x1014cb38] is set, else the low byte is kept); the rest use the low byte and keep the
 * colour for flag 0x400. The shade is 0xf for flag 0x400, else Sim_10058c5e(obj, b, &list 0x101c76a0, level) is run
 * and the shade is trunc(first vertex dword +0x3c / 31 * 7 + 0.5). Outside mode 3, for flag 0x100 or level flags 0x50
 * with a level > 0: [0x1015dd70] set adds trunc(level * (15 - shade) / 15) to the shade and likewise to every listed
 * vertex +0x3c; else they are scaled by trunc((16 - level) / 16) (an int) and masked to 0xf. The result is
 * kind | hi | mid | shade (shade << 8 and no mid for the 0xff-level kinds; mid 0xf0 with [0x1014cb28], else
 * (colour & 0xf00) >> 4). Floats are computed in double: the original's x87 ran at 53-bit precision, so the result is
 * bit-exact (see tools/verify/spec_w9h.py).
 */
uint32_t __cdecl Sim_10032a10(uint8_t *obj, uint32_t b, uint32_t colour, uint32_t d);
/*
 * When [0x1017b940] is set: 0x100d2306(camera) once after a viewport change ([0x1013a014], cleared), Sim_100d255e
 * and 0x100d1f90 on the camera [0x1014cae0]; in the sensor views ([0x1014cb18] or [0x1014cb1c] set, or view
 * [0x1015d1ac] 4) draws a full-screen quad (0,0)-(639,479), +0x10 = 7, +0x18 = 0xff000000 through the display slot
 * 0x1014cb4c with flags [0x10138c70], else runs 0x100a0def(camera) when [0x1014cb04] or [0x1014cb08] is set; then
 * Sim_100a4c45, Sim_100071f6 and Sim_100d32e5(0). The camera functions patched by hooks_view.c are called at their
 * original addresses.
 */
void __cdecl Sim_100352d5(void);

#endif
