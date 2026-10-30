/*
 * Round 9, letter h: a scene-node release helper (0x10029b1f).
 */
#ifndef HG_SIM_UNSORTED_1002_W9H_H
#define HG_SIM_UNSORTED_1002_W9H_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* ---- w9h_misc.c ---- */
/* Unless node is NULL: Sim_100aa351(node), Sim_100aa406(node), Sim_100aa799(node). */
void __cdecl Sim_10029b1f(void *node);

#endif
