/*
 * Round 12: game functions of HGSIM.DLL around 0x100b0000..0x100bffff that read uninitialised stack in the original;
 * converted with the start values recorded in DECISIONS.md ("Round 12: uninitialised stack").
 */
#ifndef HG_SIM_UNSORTED_100B_W12_H
#define HG_SIM_UNSORTED_100B_W12_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* The language letter from lang.ini (0x10150e88, mode 0x10150e84): its first line (fgets, 256 bytes); when it holds
 * "language =" (0x10150e94), the last character before trailing newlines / spaces, upper-cased (toupper of the DLL);
 * without "language =", the line's first character upper-cased (0 for an empty line); 'X' (0x58) without the file.
 * The original reads stack leftovers in the last two cases (DECISIONS.md). */
int __cdecl Sim_100b9dde(void);

/*
 * HUD damage display of the player's unit (record PTRTAB([0x1017b8c8]) +0x28): the frame shape (0x100ba7d5 of
 * [0x109e8498], [0x1014cec0]) drawn at 0x109e8674 (0x100f0205) and released
 * (Sim_100bb2b0); then for the 7 parts of the table 0x101508a0 (segment index - 1, shape slot): while the last
 * segment's divisor (+0x38) is non-zero, level = 15 - trunc((+8 * k + +0 * k) * k2 / divisor) (k, k2 at
 * 0x1012bae0 / 0x1012bae4), clamped to 0..15 (14 instead of 15 when +8 > [0x1012bae8]); style 8 / 5 / 7 / 4 for
 * level 15 / 0 / below 8 / else; the part's shape is drawn in that style's colour table when [0x1017692c] is set.
 * The divisor starts at 1 (the original tests an uninitialised local on the first pass, DECISIONS.md).
 */
void __cdecl Sim_100b66a0(void);

#endif