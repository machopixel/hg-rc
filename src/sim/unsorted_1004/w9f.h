/*
 * Round 9, letter f: the screenshot entry point 0x100473e4 and the code mapper 0x1004b8b0 of HGSIM.DLL.
 */
#ifndef HG_SIM_UNSORTED_1004_W9F_H
#define HG_SIM_UNSORTED_1004_W9F_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* ---- w9f_screenshot.c ---- */
/*
 * Saves screenshot number [0x1015f150] (nothing from 1000 on; the counter goes up by one): "hg%04d.tga" through
 * Screenshot_WriteTga ([0x109e3e14] + 1 by [0x109e3de0] + 1 pixels, stride [0x109e3e24] + 1, pixels [0x109e3e20]).
 * (The original's GIF without the renderer is gone, round 42.)
 */
void __cdecl Screenshot_Save(void);

/* Maps a code 0..0x5e to one of 0..0x10 by a fixed table (0 for anything else). */
uint32_t __cdecl Sim_1004b8b0(uint32_t code);

#endif
