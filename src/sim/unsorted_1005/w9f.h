/*
 * Round 9, letter f: the game key map reader 0x10054a79 and the score sum 0x1005e426 of HGSIM.DLL.
 */
#ifndef HG_SIM_UNSORTED_1005_W9F_H
#define HG_SIM_UNSORTED_1005_W9F_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Patched by hooks_sim.c: the game's error report (code, format, ...) (0x100bc280). */
#define W9F5_CALL_100bc280 SIM_FN(void(__cdecl *)(int, const char *, ...), 0x100bc280)

/* ---- w9f_gamekey.c ---- */
/*
 * Reads "gamekey.map" (lines "ACTION KEY+KEY..." through Input_ReadLine, line count in [0x1015fdf8]): the key
 * names add their code from the 27-entry table at 0x101402d8 (ALT, CTRL, SHIFT, F1.., see the DLL), any other
 * name adds its first character (lower-cased below 0x80), and the byte of the action (table 0x1013ff00, 196
 * names) is stored at 0x10164bc8 + the 16-bit sum. First sets bytes 0x10164be8 = 'k' and 0x10164d39 = 'v'.
 * Returns 1, or 0 after an error report 0x11 (no file, bad line, no '+' part, unknown action, a different
 * action already at that sum).
 */
int __cdecl GameKey_LoadMap(void);

/* ---- w9f_score.c ---- */
/*
 * A 16-bit score (in the low half of the result; the high half is that of the advanced record pointer, or of
 * the advanced list pointer when the list loop ran, as the original leaves it in eax) from p (word +0 start
 * value, dword +0 as multiplier, dword +0x10 list count), 17
 * records of 0x3c bytes (floats +0, +4, +8, word count +0x34, words from +0xc) and a list of dword pairs: per
 * record, for every word a counter of the first threshold (23 entries, highest first) it exceeds goes up, the
 * counters times their weights (times p[0] for positive weights) are added, then the counters' total (doubled
 * when entry 21 counted) and the truncated sums of floats +0 and +4, then the truncated float +8; finally the
 * words of 0x101440e0 indexed by list value / 100 (up to 8 entries).
 */
uint32_t __cdecl Sim_1005e426(const int32_t *p, const uint8_t *recs, const int32_t *list);

#endif
