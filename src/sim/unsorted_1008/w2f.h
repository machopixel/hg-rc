/*
 * Wave 2 (batch f) functions of HGSIM.DLL in 0x1008xxxx. Plain C, original
 * cdecl convention.
 */
#ifndef HG_SIM_UNSORTED_1008_W2F_H
#define HG_SIM_UNSORTED_1008_W2F_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Clears the dword 0x1017c8c4. */
void __cdecl Sim_10087b32(void);
/*
 * Unless the dword 0x1017c8c4 is set, walks the list that starts at the dword
 * 0x1017b998 and continues through +0x10, adding 1 to the dword 0x1017c8e0 for
 * every node, and to 0x1017c8d8 when the node has no pointer at +0x28 or the
 * first dword of what it points to is 0, else to 0x1017c8e4.
 */
void __cdecl Sim_1008988f(void);
/*
 * Like View_ScaleRect but over elements 0..3 of rect: out[0] and out[2] are
 * rect[0] and rect[2] times the dword 0x109e3e14 (16.16, rounded), out[1] and
 * out[3] are rect[1] and rect[3] times the dword 0x109e3de0. Returns out. The
 * first argument is unused.
 */
int32_t *__cdecl Sim_1008cc04(void *unused, const int32_t *rect, int32_t *out);

#endif
