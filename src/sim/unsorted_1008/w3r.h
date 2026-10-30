/* Round 3 (letter r), original 0x10087b00 and 0x100883a2. */
#ifndef HG_SIM_UNSORTED_1008_W3R_H
#define HG_SIM_UNSORTED_1008_W3R_H

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Stores the float at 0x1012b8d0 plus x in the float at 0x1014b8e0 and plus y in the float at 0x1014b8e4. */
void __cdecl Sim_10087b00(float x, float y);
/*
 * Resets the world grid state: clears 0x1036f3f8 and 0x1036f400, calls Sim_10087b41, clears the 25
 * pointers at 0x1036f2a0, initialises the 9 x 9 cells of 0x2c bytes (row stride 0x18c) from
 * 0x1017ba28 (word +0 = 0x4000, dwords +4..+0x28 = 0 except +0x24, which points to the cell itself),
 * initialises five more cells in the same way without +0x24 and +0x28 (0x1017b988, 0x1017b9b0,
 * 0x1017b9d8, 0x1017ba00, 0x1017c818), points 0x1017c8cc / c8d0 / c8c8 / c8d4 to four of them and
 * calls Sim_10087b00(3979.0f, -3984.5f).
 */
void __cdecl Sim_100883a2(void);

#endif
