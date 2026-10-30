/*
 * Round 9, letter c: the keyboard state kept from the window messages (0x10057d2d).
 */
#ifndef HG_SIM_UNSORTED_1005_W9C_H
#define HG_SIM_UNSORTED_1005_W9C_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* ---- w9c_keys.c ---- */
/*
 * Called by the message pump (0x10057b4e) with the wParam / lParam of WM_KEYDOWN, WM_SYSKEYDOWN (down = 1) and
 * WM_KEYUP, WM_SYSKEYUP (down = 0). The scan code (bits 16..23 of lParam) of an extended key (bit 24) below
 * 0x59 is replaced by the dword 0x10143c00[scan] (0 = ignore). Bit (scan - 1) of the key bitmap 0x1015e650 is set
 * or cleared; scan codes 0x1d / 0x70 also set or clear 0x800000, 0x2a / 0x36 0x400000 and 0x38 / 0x71 0x1000000
 * in the modifier dword 0x1015e65c. wParam is not used.
 */
void __cdecl Key_SetState(uint32_t wparam, uint32_t lparam, int down);

#endif
