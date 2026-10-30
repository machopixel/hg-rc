/*
 * Wave 2l: functions around 0x10087a43..0x10087b41 that set the globals of
 * the cell grid (see cell_link.c). The list functions of the same range are in
 * view/w2l.h. Plain C, original cdecl convention.
 */
#ifndef HG_SIM_UNSORTED_1008_W2L_H
#define HG_SIM_UNSORTED_1008_W2L_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Returns 0x50605. */
uint32_t __cdecl Sim_10087a43(void);
/* dword 0x1017c8c4 = 1. */
void __cdecl Sim_10087b23(void);
/* Stores fixed values into the 32 dwords 0x1017c840..0x1017c8bc (see the code for the pattern of 0, 1 and -1). */
void __cdecl Sim_10087b41(void);

#endif
