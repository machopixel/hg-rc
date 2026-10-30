/*
 * Round 10 (helper d): game functions of HGSIM.DLL around 0x100c0000..0x100cffff.
 * Calls of functions that are not converted yet go through their original address (SIM_FN).
 */
#ifndef HG_SIM_UNSORTED_100C_W10D_H
#define HG_SIM_UNSORTED_100C_W10D_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Can entity `who` pick entity `index` (pointer table) as its target with `flags`? Negative codes: -1 / -2 index out
 * of range, -9 flags 0x15 or the side test of Sim_10052be7 (flag 0x20000: must be 0, 0x40000: must be 1), -3 target
 * hidden (+0x1c flags 6, or 0x10 for the player while [0x1017e5b0] is 0) or owned by who, -4 flag 0x10000 without
 * target flag 0x40, -5 (player only) target flags without 0x1400 while [0x1017e5b0] is 0, or with 0x800, -8 no node
 * +0x48 or Sim_100a9b71 of it 0, -7 (player only) too far (the second result of Sim_100cbc6b above [0x1012bc48]
 * unless target flag 0x1000, or above [0x1012bc4c] times the range of the clamped level difference). Otherwise
 * stores the target's position (height adjusted for a vehicle target) at who+0xdc..0xe4, the four results of
 * Sim_100cbc6b at +0xe8, +0xd8, +0xd4, +0xec and returns 1. */
int __cdecl Sim_100cada2(int32_t who, int32_t index, uint32_t flags);

#endif
