/*
 * Round 9, letter g: functions of HGSIM.DLL at 0x10073041 and 0x100766ca.
 */
#ifndef HG_SIM_UNSORTED_1007_W9G_H
#define HG_SIM_UNSORTED_1007_W9G_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* ---- w9g_misc.c ---- */
/* Dword +0x4fc of the block obj +0x28 = value. */
void __cdecl Sim_10073041(void *obj, uint32_t value);
/* The dword +4 of the list node Sim_1007652f(key) finds (the list at [0x1015dc14]), 0 without one. */
int32_t __cdecl Sim_100766ca(int32_t key);

#endif
