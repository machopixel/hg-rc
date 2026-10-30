/*
 * Round 9, letter e: functions of HGSIM.DLL at 0x10075753.
 */
#ifndef HG_SIM_UNSORTED_1007_W9E_H
#define HG_SIM_UNSORTED_1007_W9E_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* ---- w9e_misc.c ---- */
/*
 * Maps an event code (and for code 0xd its sub code) to two outputs: codes 9 and 0xc7 set *outB = 1, codes
 * 0xc6 and 0x209 set *outB = -1, code 0x1b sets *outA = 5, code 0xd sets *outA to 3 / 4 / 5 for sub 0 / 2 /
 * 5 and 6. Anything else leaves both untouched.
 */
void __cdecl Sim_10075753(int code, uint32_t sub, uint32_t *outA, uint32_t *outB);

#endif
