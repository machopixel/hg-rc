/*
 * Round 4, letter v: functions of HGSIM.DLL around 0x10053000..0x10057000 that call Windows (heap, joystick).
 */
#ifndef HG_SIM_UNSORTED_1005_W4V_H
#define HG_SIM_UNSORTED_1005_W4V_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Frees the singly linked list of blocks headed by [0x1015f3b0] (the link is the first dword) and clears it and [0x109b6880]. */
void __cdecl Sim_10053132(void);

#endif
