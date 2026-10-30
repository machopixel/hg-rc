/*
 * Round 9, letter e: functions of HGSIM.DLL at 0x100dc2d7.
 */
#ifndef HG_SIM_UNSORTED_100D_W9E_H
#define HG_SIM_UNSORTED_100D_W9E_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* The game's fatal error report SystemError(code, format, ...) (hooks_sim.c taps it). */
#define W9E_SYSERR SIM_FN(int(__cdecl *)(int, const char *, ...), 0x100bc280)

/* ---- w9e_misc.c ---- */
/*
 * Hands out the next gamething index: the counter at 0x1015dc68 (incremented), or -1 with the error 0x2f
 * "Too many gamethings" once it has reached 0xfe.
 */
int __cdecl GameThing_NextIndex(void);

#endif
