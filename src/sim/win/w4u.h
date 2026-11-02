/*
 * Round 4 (letter u), Windows wrappers.
 *
 * (The CD audio wrappers of 0x1009dec0..0x1009eaa2 are gone: no CD audio, DECISIONS.md "dropped features".)
 *
 * Display (original 0x1007eda5..0x1008400d): the game's window and its device-independent bitmap (the GDI window
 * driver, display mode 5; the DISPDIB driver functions are gone: DECISIONS.md "dropped features").
 */
#ifndef HG_SIM_WIN_W4U_H
#define HG_SIM_WIN_W4U_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Creates the window's DIB section, or sets its colour table when it exists. Returns 1, or 0 when the DIB section cannot be created. */
int __cdecl Sim_1008375a(void);
/* SetDIBitsToDevice of the rectangle (a, b)..(c, d) of the bitmap at dword 0x1015e9a0 (the message "GDI BitBlt Rect err" on failure); returns -1 unless all lines (dword 0x109e3d18) were drawn. */
int __cdecl Gdi_BitBltRect(int a, int b, int c, int d);
/* Copies `count` RGB triples to palette entries start.. (0x1015e6a0), rebuilds the GDI palette from all 256 (value << 2). Returns 1, or 0 for bad arguments. */
int __cdecl Gdi_SetPalette(int start, int count, const uint8_t *rgb);
/* Gdi_SetPalette(a, b, c) when the flag 0x1017b7d8 is set; 0 on success, -1 otherwise. The fourth argument is not used. */
int __cdecl Sim_1008400d(int a, int b, const uint8_t *c, int d);

#endif
