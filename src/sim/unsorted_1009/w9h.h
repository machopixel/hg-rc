/*
 * Round 9, letter h: a two-variable reset (0x1009bd47).
 */
#ifndef HG_SIM_UNSORTED_1009_W9H_H
#define HG_SIM_UNSORTED_1009_W9H_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* ---- w9h_misc.c ---- */
/* [0x1015d104] = 0 and [0x1015d1c8] = 0. */
void __cdecl Sim_1009bd47(void);

#endif
