/* Round 3 (letter r), original 0x10019670. */
#ifndef HG_SIM_UNSORTED_1001_W3R_H
#define HG_SIM_UNSORTED_1001_W3R_H

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Loops over eight 0x128-byte records of the player's gear (gear pointer table
 * 0x109e8b40 indexed by [0x1017b8c8], pointer at +0x28, record array at +0x84) and reads a
 * dword of each without using it: the result of the loop was dropped in the original, so
 * this changes nothing. It is kept as reads only (they can fault like the original's).
 */
void __cdecl Sim_10019670(void);

#endif
