/*
 * Round 11: game functions of HGSIM.DLL around 0x10070000..0x1007ffff, converted one at a time.
 * Methods of the DirectDraw objects are COM calls (__stdcall) through the object's table.
 */
#ifndef HG_SIM_UNSORTED_1007_W11_H
#define HG_SIM_UNSORTED_1007_W11_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#define __stdcall
#endif

/* Releases the DirectDraw surfaces: *[0x1015e698] = 0; the primary [0x1017b394] (Release, then it and 0x1017b398 /
 * 0x1017b3a0 cleared, and the back buffer 0x1017b39c forgotten when *[0x1015e68c] is 0), then the back buffer
 * unless the mode is 320x200 ([0x109e3d14] / [0x109e3d18]). */
void __cdecl Sim_1007f9da(void);

/* DirectDraw shutdown: 0x1017b3b0 = 0; with the object [0x1017b390]: the surfaces (Sim_1007f9da),
 * SetCooperativeLevel(window [0x1017b8f0], 8), RestoreDisplayMode, the palette [0x1017b3a8] released, the object
 * released and cleared. */
void __cdecl Sim_1007f950(void);

#endif
