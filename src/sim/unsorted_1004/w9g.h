/*
 * Round 9, letter g: the function of HGSIM.DLL at 0x1004e838 (end of the GIF writer of w7m_gif.c / w8v_shots.c).
 */
#ifndef HG_SIM_UNSORTED_1004_W9G_H
#define HG_SIM_UNSORTED_1004_W9G_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* ---- w9g_gif.c ---- */
/*
 * Writes the GIF trailer: Sim_1004e719 on a 0x14-byte record whose first byte is 0x3b (';'; the other bytes are
 * whatever the stack held, as in the original), 2 when that fails; else Gif_CloseFile, frees the block
 * [0x1015f360] (Heap_FreeBlock) and clears it, returns 0.
 */
int __cdecl Gif_Finish(void);

#endif
