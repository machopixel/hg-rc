/*
 * Round 10: game functions of HGSIM.DLL around 0x10090000..0x1009ffff, converted one at a time.
 * Calls of functions that are not converted yet go through their original address (SIM_FN).
 */
#ifndef HG_SIM_UNSORTED_1009_W10_H
#define HG_SIM_UNSORTED_1009_W10_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Returns 0 when count < 2. Otherwise snaps the two floats +0x1c / +0x20 of each of the count 0x24-byte
 * vertices to 0.99f (when above the double 0.5 at 0x1012b9b0) or 0.01f, sets +0x18 to 0xff000000, and returns
 * Render_SubmitVerts_100068d9(index, count, verts). The fourth argument is passed on by the original but not
 * read by the callee. */
int __cdecl Sim_10098f8c(int index, int count, uint8_t *verts, uint32_t unused);

/* Unless [0x1017cc24] + 1 >= [0x1017cc20]: makes an object with Sim_10090c7d(0x100027fc, 0x10001785, 0, 0) (the
 * two code addresses at the preferred base) at x, y, z moving with v: its node (+0x552)->+0x8c record gets the
 * length of v * [0x1012b994] (Sim_10084a94) at +0x50, x, y, z at +0x44 and 0 at +0x38..+0x40; +0x56e..+0x576 =
 * v / [0x1012b968]; the node is set up (Sim_100aa39f, Sim_100a9d47(node, x, y, z), Sim_100aa799), [0x1017cc24]
 * counts it, +0x55e = [0x1015dc78], the ten 0x2a-byte points from +0x12 get word +0x18 = 1 and the positions
 * 2, 3, .. 11 times v / [0x1012b958] (added up in float), and +0xc = 1. */
void __cdecl Sim_10096ac3(uint32_t x, uint32_t y, uint32_t z, float vx, float vy, float vz);

/* HUD readout item: with the font obj+0x30 (key [0x1014cec4], 0x100ba7d5) draws the text [obj+0x40] at
 * (obj+0x60, obj+0x64) on [obj] (+0x1c takes +0x18 when they differ) and, in view mode 4 ([0x1015d1ac]), the own
 * unit's heading h = fmod(fmod(+0x68, 360) + 360, 360): when it differs from obj+0x4c the text [obj+0x48] becomes
 * sprintf("%s%3.1lf", [obj+0x44], h) and +0x4c = h; that text is drawn at (obj+0x68, obj+0x6c). Releases the font. */
void __cdecl Sim_1009b03d(uint8_t *obj);

#endif
