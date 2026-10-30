/*
 * Round 9, letter f: the constant 0x100ab1f2 of HGSIM.DLL.
 */
#ifndef HG_SIM_UNSORTED_100A_W9F_H
#define HG_SIM_UNSORTED_100A_W9F_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* ---- w9f_misc.c ---- */
/* Returns 0xa4. */
uint32_t __cdecl Sim_100ab1f2(void);

#endif
