/*
 * Round 10: game functions of HGSIM.DLL around 0x100b0000..0x100bffff, converted one at a time.
 * Calls of functions that are not converted yet go through their original address (SIM_FN).
 */
#ifndef HG_SIM_UNSORTED_100B_W10_H
#define HG_SIM_UNSORTED_100B_W10_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Starts the AI order of obj (order word +0x1f8, 0..0x1b): +0x1aa = +0x1a8, the order fields +0x204 / +0x268 / +0x218 /
 * +0x21c / +0x264 cleared, the order timer +0x200 = now + 0x235a and 0x1017e2a8..0x1017e2b0 cleared; then per order the
 * timers (+0x200, +0x204, some random by Sim_1009c929), the order values +0x218.. +0x23a, the control bytes of +0x54
 * and the order's helper (Sim_100c274a, Sim_100c3725, Sim_100c00c7 towards the target at 10000.0, Sim_100157ca, the
 * path record Sim_10033c42 / Sim_1003359f). */
void __cdecl Sim_100bf28e(uint8_t *obj);

#endif
