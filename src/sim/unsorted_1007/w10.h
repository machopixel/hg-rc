/*
 * Round 10: game functions of HGSIM.DLL around 0x10070000..0x1007ffff, converted one at a time.
 * Calls of functions that are not converted yet go through their original address (SIM_FN).
 */
#ifndef HG_SIM_UNSORTED_1007_W10_H
#define HG_SIM_UNSORTED_1007_W10_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Menu item that cycles through a list of texts (item+8: +0 text callback, +4 count, +8 the texts): the current
 * index comes from the item's callback +0x14 (item+0xc) when flag 1 is set, else item+4. The state's code
 * (state+0) 1 refreshes it (+0x10 and +0x14 callbacks), 2 calls +0x10, 4 calls +0x1c(item+0xc, index) and +0x10,
 * 5 calls +0x20(item+0xc). The text (index modulo the count; plus the list callback's text, at most 127
 * characters) is drawn at (x, y) with menu's font +0x48 on menu+0. For the selected row (state+0x10 == row):
 * +0x1c again when the row is [0x1015dc18] - 0x31 and that row's state word +0x18 is 5, and +0x18(item+0xc,
 * index) unless the code was 4 or 5. Stores the index in item+4. */
void __cdecl Sim_10077272(uint8_t *menu, uint8_t *item, int32_t row, uint32_t x, uint32_t y, uint8_t *state);

/* Menu slider item (item+8: the shape names +0 / +0x10, or +8 / +0x18 while [0x101c64f0] is set; byte 7 = 'G' while
 * [0x1014cb38] is set): the value (as Sim_10077272: callback +0x14 or item+4; codes 1, 4, 5 alike) moves by
 * +-0x199a for the selected row with code 2 (keys 0x20 / 0xc8 up, 0xc9 down, or the row's hot key), clamped to
 * 0..0x10000. The bar is drawn at (x / 3, y + font height) and the knob at the bar's width times the value, less
 * half the knob's width; the selected row then calls +0x1c (moved with the row's state 4, or hot with 5) and +0x18.
 * Stores the value in item+4 and releases the two shapes +8 / +0x18 (Sim_100bb2b0). */
void __cdecl Sim_10076de0(uint8_t *menu, uint8_t *item, int32_t row, int32_t x, int32_t y, uint8_t *state);

#endif
