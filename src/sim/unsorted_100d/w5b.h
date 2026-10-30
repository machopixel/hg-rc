/*
 * Round 5, letter b: a function around 0x100d3491 that calls a helper which is not converted yet
 * (called through its address with SIM_FN).
 */
#ifndef HG_SIM_UNSORTED_100D_W5B_H
#define HG_SIM_UNSORTED_100D_W5B_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Not converted. */
#define SIM_CALL_100d3510 SIM_FN(int(__cdecl *)(int, uint32_t, int), 0x100d3510)

/*
 * When the counter [0x1017e65c] is not 0: Sim_1008c803(0x1017e620), decrement the counter and, when it reaches 0,
 * [0x10152420] = [0x10152428] and either 0x100d3510([0x1015242c], [0x1017e658], 0) when [0x1017e658] is set or
 * [0x1017e650] = 1. Then, when [0x1017e660] is set, Sim_1008c5e0(0x1017e640).
 */
void __cdecl Sim_100d3491(void);

#endif
