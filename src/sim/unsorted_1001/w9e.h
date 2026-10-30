/*
 * Round 9, letter e: functions of HGSIM.DLL at 0x1001dd66.
 */
#ifndef HG_SIM_UNSORTED_1001_W9E_H
#define HG_SIM_UNSORTED_1001_W9E_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* ---- w9e_misc.c ---- */
/*
 * Reads the dword +0x48 of the object of the current player (0x109e8b40[0x1017b8c8]) into a local that is
 * never used; nothing else. No caller in the DLL.
 */
void __cdecl Sim_1001dd66(void);

#endif
