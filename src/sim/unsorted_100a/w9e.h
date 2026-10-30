/*
 * Round 9, letter e: functions of HGSIM.DLL at 0x100a2d91 (2-D polygon clipping).
 */
#ifndef HG_SIM_UNSORTED_100A_W9E_H
#define HG_SIM_UNSORTED_100A_W9E_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* ---- w9e_clip.c ---- */
/*
 * Clips the polygon of `count` vertices (0x18 bytes each, x at +0 and y at +4, in the pool at 0x101ce4a0)
 * against the planes selected by `planes` in this order: bit 1 = x <= 0x2000, bit 4 = y >= -0x2000, bit 2 =
 * x >= -0x2000, bit 8 = y < 0x2000 (its crossing test uses <= 0x2000). New vertices made by Sim_100a2c08 are
 * appended to the pool after the input. The result (count vertices of 0x18 bytes) is copied to 0x101ccca0.
 * Returns the number of vertices left, 0 when no plane was selected, or -1 as soon as a plane leaves none.
 */
uint32_t __cdecl Clip_Polygon2D(uint32_t count, uint32_t planes);

#endif
