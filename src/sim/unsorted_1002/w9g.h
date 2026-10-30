/*
 * Round 9, letter g: the function of HGSIM.DLL at 0x1002c8cb.
 */
#ifndef HG_SIM_UNSORTED_1002_W9G_H
#define HG_SIM_UNSORTED_1002_W9G_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* ---- w9g_misc.c ---- */
/* Sim_10048287(a, b, c, d). */
void __cdecl Sim_1002c8cb(uint32_t a, uint32_t b, uint32_t c, uint32_t d);

#endif
