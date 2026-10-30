/*
 * Round 9, letter c: the entry table of slots.c (0x7e-byte entries at 0x109b9e20, count 0x1015ea80), the
 * plugin probe by list file and a row copy on the display record [0x1015e698].
 */
#ifndef HG_SIM_UNSORTED_1003_W9C_H
#define HG_SIM_UNSORTED_1003_W9C_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* ---- w9c_slots.c ---- */
/*
 * The first entry whose dword +0x60 is `key` gets its four dwords +0x50..+0x5c cleared, +4 = -1 and the word
 * +0x7c = 0 (the twin of 0x1003b8e3, which also releases the four and sets the word to 1). Nothing otherwise.
 */
void __cdecl Sim_1003b986(uint32_t key);

/* ---- w9c_plugin.c ---- */
/*
 * The probing twin of Plugin_LoadList (0x10036876, letter f): clears [0x1015e56c] and opens the list file
 * `name` ("r"); without it returns Plugin_ProbeAll(arg). Otherwise every line's first token (separators
 * " \t\r\n"; a NULL token for a blank line) goes to Plugin_ProbeRaster(token, arg), the file is closed and
 * [0x1015e56c] (the device count the probe stored) is returned. Not referenced anywhere in the DLL.
 */
uint32_t __cdecl Plugin_ProbeList(const char *name, uint32_t arg);

/* ---- w9c_surface.c ---- */
/*
 * On the display record r = [0x1015e698] (+0 pixel pointer, +4 stride - 1, +8 a signed size): for each of the
 * first [r+8] / 4 rows (truncating division) copies the 10 bytes at the row start (pixels + ([r+4] + 1) * row)
 * to the row start + 0x14 (Mem_Move). Not referenced anywhere in the DLL.
 */
void __cdecl Sim_10035aec(void);

#endif
