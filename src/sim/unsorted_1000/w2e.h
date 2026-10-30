/*
 * Wave 2e, original 0x10007a8e..0x1000fee8. The first group is the debug
 * output on the text screen at 0xB0000 (see unsorted_1003/w2e.h): a spinner in
 * the top right corner, a status line on row 0 and a scrolling log in rows
 * 3..23, all guarded by the dword 0x10a3ade8. The second group works on a table of
 * named records.
 */
#ifndef HG_SIM_UNSORTED_1000_W2E_H
#define HG_SIM_UNSORTED_1000_W2E_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Sets the dword 0x10a3ade8 to 1 (text screen in use), clears the screen, sets
 * [0x1015f1e8] to 1 and fills the 80-character line at 0x10a3ae20 with spaces.
 */
void __cdecl Mono_Enable(void);
/*
 * Counts [0x1012dca4] down; at 0 it restarts at 4 and writes the next of the four
 * glyphs at 0x1012dca0 (index [0x1012dca8], counting down from 3) at column 79 of row 0.
 */
void __cdecl Mono_TickSpinner(void);
/* Writes s (at most 78 characters, padded with spaces, one leading space) on row 0 with attribute 0x78. */
void __cdecl Mono_PutTopLine(const char *s);
/* Blanks rows 3..23 with the spaces line at 0x10a3ae20 and resets the cursor row [0x1012dc7c], [0x1012dc78] to 3 and [0x1015d024] to 0. */
void __cdecl Mono_ClearLog(void);
/*
 * Prints s character by character at the cursor (column [0x1015d080], row [0x1012dcac],
 * attribute 7); a newline or column 80 moves to the next row, and past row 23 the log is
 * cleared and starts again at row 3. Ends with [0x1012dc78] = the row.
 */
void __cdecl Mono_PrintText(const char *s);

/*
 * The table of 0x14-byte entries from 0x10a39200 (dword use count at +0, name at +4;
 * count of names in [0x10a38de0]). Returns the index of the entry holding the name, whose
 * count is incremented, or else stores the name in the first entry with count 0
 * (count 1) and returns that index.
 */
int __cdecl Sim_1000df9c(const char *name);
/* With the block at [0x1015d0a4] set: finds the entry of that name (Sim_1000df9c) and sets the first of 16 non-zero states (+0x54) whose id (+0x2d4) is that index to 3. */
void __cdecl Sim_1000ecb0(const char *name);
/* With b non-null: runs Sim_10010183, Sim_10010267 and Sim_1001034b on it and returns 1; else 0. a is unused. */
int __cdecl Sim_1000fee8(uint32_t a, void *b);

#endif
