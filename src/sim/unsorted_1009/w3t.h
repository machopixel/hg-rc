/*
 * Round 3 (letter t) functions of HGSIM.DLL in 0x1009xxxx. Plain C, original
 * cdecl convention. Names carry the original address because the code alone
 * does not say what the data is for.
 *
 * The screen layout: 0x1014c4c8 is a table of 6 pointers to "view" records
 * (one per mode 0..5, the current mode is the dword 0x1015d1ac). A view record
 * has a pointer to its rect at +0, a dword at +8 (an index), points at +0x58,
 * +0x60 and +0x68, a pointer to a child record at +0x14 (whose +4 points to a
 * table of rect pointers at +4, +8, +0xc), and four dwords at +0x7c..+0x88 that
 * are indices into the table at 0x1014bf08. Rects are as in view/view.h.
 */
#ifndef HG_SIM_UNSORTED_1009_W3T_H
#define HG_SIM_UNSORTED_1009_W3T_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Lays out the view `view` for `mode` (nothing happens for a null view): makes
 * mode the current mode, takes its rect from the saved rect at 0x109e8660 (mode
 * 1) or the display rect scaled by View_ScaleRect (other modes), saves it
 * again for mode 1 and into 0x10185400 + 0x14 * view[2], scales the three
 * points, lays out the child rects (Sim_1008ce40 and View_CenterRect for mode 4,
 * else Rect_ScaleToRect), replaces the four indices at +0x7c..+0x88 by the
 * table entries and calls Sim_1009b3a1(0).
 */
void __cdecl Sim_100996c0(int mode, void *view);

/* Lays out all 6 views, sets the dwords 0x1015d1ac and 0x1015d098 to 0 and 0x1015d1cc to 1, scales a point of the record at 0x1014c06c and stores 0x1014befc - 0x1017cc58 in 0x1014bf00. */
void __cdecl Sim_10099928(void);

/*
 * Changes the dword +0x18 of the current view (unless it is null, the byte
 * at +9 of the structure at 0x1017b8c4 is 0, or the mode is 4): mode 0 copies
 * +0x20 into it, mode 1 halves it (limited to +0x28 from below by +0x24),
 * mode 2 doubles it (limited to +0x24 from above by +0x28); then stores
 * the float (int)(+0x20 / +0x18) at +0x2c.
 */
void __cdecl Sim_1009b3a1(int mode);

#endif
