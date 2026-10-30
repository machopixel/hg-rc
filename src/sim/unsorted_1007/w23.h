/*
 * Round 23: menu widgets Ghidra's index missed (reached only through the menu item tables 0x1014bbb0.., 0x10149a78,
 * 0x1013d378..; DECISIONS.md, "Round 23").
 */
#ifndef HG_SIM_UNSORTED_1007_W23_H
#define HG_SIM_UNSORTED_1007_W23_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * A choice item of a menu: item +8 is the list of choices (+4 count, +8.. the texts), +4 the current one, +0xc the
 * context its callbacks get (+0x10 set-up(menu, item), +0x14 get(ctx), +0x18 changed(ctx, index), +0x1c chosen(ctx,
 * index), +0x20 cancelled(ctx)); bit 0 of +0 takes the current one from get. By the menu's state +0: 1 sets up (the
 * current one from get, or 0), 2 for the selected row ([menu+0x10] == row) steps by the key [0x1015dc18] (0x20 or
 * 0xc8 forward, 0xc9 back, the row's digit key forward), 4 and 5 call chosen / cancelled and end the changes. The
 * current text is drawn at (x, y) on the panel's surface +0 with its font +0x48, by the display's text routine; for
 * the selected row a step of a row of type 4 (or the row's digit key on one of type 5) calls chosen, and changed is
 * called while changes go on. Nothing without a panel, its font or surface, a menu, or the list of choices.
 */
void __cdecl Sim_1007760b(void *panel, void *item, int32_t row, int32_t x, int32_t y, void *menu);

/*
 * A text item of a menu (arguments as Sim_1007760b): item +8 is {pane, text}; a pane whose first dword is 0 is first
 * set up on the screen record 0x109e3e20 (its first dword, then 0x1008cafb(panel surface, pane, pane)); the text is
 * drawn wrapped in it (Sim_1008dba2 with the panel's font +0x48), and the item's changed callback +0x18 is called
 * with 0 unless the menu's state is 4 or 5. Nothing without a panel, its surface or font, an item, its record, pane
 * or text, or a menu.
 */
void __cdecl Sim_1007792c(void *panel, void *item, int32_t row, int32_t x, int32_t y, void *menu);

#endif
