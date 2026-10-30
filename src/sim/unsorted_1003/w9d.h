/*
 * Round 9, letter d: the recursive node-size sum of HGSIM.DLL at 0x1003218b.
 */
#ifndef HG_SIM_UNSORTED_1003_W9D_H
#define HG_SIM_UNSORTED_1003_W9D_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* ---- w9d_tree.c ---- */
/* 0 for a NULL node, else 0x2c + 4 * dword +0x18 plus the sums of the four child nodes at +0x1c..+0x28. */
int __cdecl Sim_1003218b(void *node);

#endif
