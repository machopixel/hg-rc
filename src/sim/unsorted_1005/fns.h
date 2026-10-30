/*
 * Wave 2 functions of HGSIM.DLL around 0x100505f0..0x10050b66: each stores
 * constant points (and some values derived from the 16-bit fields at +0x100
 * and +0x102) into a structure and scales the points with Rect_ScalePoint by
 * the rect whose pointer is at +0xec of that structure.
 */
#ifndef HG_SIM_UNSORTED_1005_H
#define HG_SIM_UNSORTED_1005_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Picks its constants by the dword at +0x613 of [[table 0x109e8b40 + 4 * [0x1017b8c8]] + 0x28]. */
void __cdecl Sim_100505f0(void *obj);
void __cdecl Sim_100508df(void *obj);
void __cdecl Sim_1005095b(void *obj);
void __cdecl Sim_100509df(void *obj);
void __cdecl Sim_10050a63(void *obj);
void __cdecl Sim_10050b66(void *obj);

#endif
