/*
 * Round 24: the in-mission menu draw (DECISIONS.md, "Round 24").
 */
#ifndef HG_SIM_UNSORTED_1007_W24_H
#define HG_SIM_UNSORTED_1007_W24_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * The key code the original's menu draw last stored in its [ebp-0x128] (the key [0x1015dc18], or 0xd for a number
 * key that picked an item); a menu's first frame reads that slot unwritten, the C reads this. 0 before any store.
 */
extern int32_t g_w24MenuKey;

/*
 * Draws and runs the top menu of the screen s (Sim_100765f6(s); font s+0x48, pane s+0): the key [0x1015dc18] (a digit
 * picks an item, 0 the last kind 2 / 6 item; then Sim_10075753 with the item kind for the step), the selection +0x10
 * skipping text lines (kind 3), the title +4 at s+0x60 / +0x64 with its underline (Sim_1008d65d with flag 4 of s+4,
 * else Sim_1008d6f7), then per item from s+0x70 / +0x74 down by s+0x5c: the cursor shape s+0x20, its number, its label
 * (wrapped at the first space when it is as long as the width [0x10149950 + 4 * [0x101c64f0]]) and its routine +8;
 * state 3 opens the selected item's sub-menu +0x10 (Sim_10076624), 4 / 5 close the menu (Sim_100765a7).
 */
void __cdecl Sim_1007581a(uint8_t *s);

#endif
