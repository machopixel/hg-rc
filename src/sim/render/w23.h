/*
 * Round 23: functions of HGSIM.DLL that Ghidra's index missed (reached only through pointers; DECISIONS.md,
 * "Round 23").
 */
#ifndef HG_SIM_RENDER_W23_H
#define HG_SIM_RENDER_W23_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * The scene pass's polygon routine (put in the driver slot [0x1014cb4c] by the scene setup): nothing without a
 * texture set ([0x1014c4c8 + [0x1015d1ac] * 4]); by the kind flags & 0xf000: 0x3000 to 0x10098f8c((flags & 0xff0)
 * >> 4, count, vertices, -1, 1), 0 drawn twice by Sim_1009ef50 (flat 0xf0, then the flags with 0xa000), 0x8000 and
 * 0x9000 not at all, any other kind by Sim_1009ef50 as given.
 */
void __cdecl Sim_1009b772(int32_t count, void *verts, uint32_t flags);

/*
 * A cockpit panel of the player's weapon slot (panel +0xc, with a non-zero word +4): nothing when the slot is empty
 * (+4 < 0). It loads the image [0x109b9900] (table [0x1014cec4], released again at the end without being drawn) and,
 * while the time [0x1015dc78] is past the panel's +8, draws the image [0x109e84b8] (table [0x1014cec0]) on the
 * panel's surface +0xec at 0, 0 with the shape drawer of the display (0x100f0205 accelerated, else 0x100e91dc).
 */
void __cdecl Sim_100d422f(void *panel);

/*
 * The sliding side panel (with a non-zero word +4 and the mode [0x10151d54]): with the font [0x109b9900] (table
 * [0x1014cec4]; nothing without it) and its height, the panel's position from +0xb4 / +0xb8, the two marks
 * [0x10151d9c] / [0x10151db4] at +0xd0 + the slide [0x1017e5e4] + 0x42; it draws its frame [0x109e8494] at the slide
 * offset on 0x109e878c and moves the slide out ([0x1017e5ec] set, by ([0x1017e5e8] * [0x1015dc80]) >> 16, up to half
 * of the width [0x10151d58 + [0x101c64f0] * 4]) or in (down to that width), then draws [0x109e8490] on the panel's
 * surface +0xec. Mode 1 shows the current objective (Sim_100cd70f); otherwise one line "name: number" (format
 * 0x10151e20, the name +0x124, the word 0x1015d0f0[i]) per unit of 0x109e8b40.. up to [0x1015dc64] (stopping at an
 * empty one, skipping one with bit 0x4800 of the word +0x1c), each a font height lower, in colour 0xc.
 */
void __cdecl Sim_100ce586(void *panel);

/*
 * The F12 objectives screen (with a non-zero word +4 and the page [0x1017e5e0]): with the font [0x109b9900] (table
 * [0x1014cec4]; nothing without it), from the panel's position pair [+0xf0] on the surface +0xec. Page 1: the title
 * [0x101c4c4c] in colour 0x14 (and Sim_1008d6f7), the objective lines Sim_100ccc60 of kinds 1, 2, 4, 0, 8, then one
 * status line by the byte +0x38 of the mission record 0x10473800 + [0x1013bb40] * 0x4fc3a: 0 the time to go (or
 * since) by Sim_100cca20, 2 / 3 / 4 a time by Sim_100ccaa0 after the texts [0x101c4c3c] / [0x101c4bf4] /
 * [0x101c4bf0]; 1 and above 4 an empty line. Any other page: one line "name: number" (format 0x10151e18) per unit,
 * as Sim_100ce586's list.
 */
void __cdecl Sim_100cde94(void *panel);

#endif
