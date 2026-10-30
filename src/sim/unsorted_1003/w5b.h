/*
 * Round 5, letter b: a function around 0x1003cb21 that calls a helper which is not converted yet
 * (called through its address with SIM_FN).
 */
#ifndef HG_SIM_UNSORTED_1003_W5B_H
#define HG_SIM_UNSORTED_1003_W5B_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Not converted. */
#define SIM_CALL_100d3830 SIM_FN(void(__cdecl *)(uint32_t, int), 0x100d3830)

/*
 * Table of 8-byte entries at 0x1013ab98 (dword, dword). Only when [0x1015ebb0] is 1 and index differs from
 * [0x1013abb8]: takes the entry's second dword (0xb5 when [0x109b9e08] is 1, which is cleared; 0x16a, and [0x1015eb98]
 * incremented, when [0x1015eb98] is 2), runs 0x100d3830(entry's first dword, that value) and stores index in
 * [0x1013abb8].
 */
void __cdecl Sim_1003cb21(int index);

#endif
