/*
 * Wave 2 (letter i) additions to unsorted_1007: the stack pop next to Sim_10076624 and the tile map functions around 0x1007b400..0x1007cc00.
 * Plain C, original cdecl convention. Names carry the original address
 * because the code alone does not say what the data is for.
 */
#ifndef HG_SIM_UNSORTED_1007_W2I_H
#define HG_SIM_UNSORTED_1007_W2I_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * The counterpart of Sim_10076624 (view.h): count at +0xc, array pointer at
 * +8. When the count is above 0: sets [0x1017e5ec] to 0 if it was 1, lowers it
 * and returns the dword it now points to; otherwise returns 0.
 */
uint32_t __cdecl Sim_100765a7(void *stack);
/*
 * Maps of 0x13f bytes per row and 0x4fc3a bytes per map start at 0x1047383a
 * (map a, row b): a cell is 12 bytes from +7 with a kind dword (1, 2 or 3), a
 * row number dword and a map number dword (low nibble). Sim_1007b42c(a, b, i)
 * is 1 when cell i of that row's kind says the byte it names (row, map) has
 * value 5 or 6 (kind 1), 5 (kind 2) or 6 (kind 3).
 */
int __cdecl Sim_1007b42c(int a, int b, int i);
/* Folds Sim_1007b42c over the cells of the row (see the .c file). */
int __cdecl Sim_1007b4d6(int a, int b);
/* Resets map a: see the .c file. */
void __cdecl Sim_1007caab(int a);
int __cdecl Sim_1007dbaa(void); /* returns 0 */

#endif
