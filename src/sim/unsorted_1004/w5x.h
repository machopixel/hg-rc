/*
 * Round 5, letter x: 0x1004c362, which calls the C library square root (not converted, called through its
 * address, SIM_FN). Built with SSE doubles like the real build.
 */
#ifndef HG_SIM_UNSORTED_1004_W5X_H
#define HG_SIM_UNSORTED_1004_W5X_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Not converted: the C library's sqrt (a double on the stack, the result in st(0)). */
#define SIM_CALL_100fca04 SIM_FN(double(__cdecl *)(double), 0x100fca04)

/*
 * truncate(sqrt(Sim_1004c318(word))) + the dword at 0x10124ec8 + 0x98 * Sim_1004c30d(word). The original
 * truncates with the compiler's _ftol helper (0x100fcb08, st(0) in, eax out), which is an ordinary cast here.
 */
int __cdecl Sim_1004c362(uint32_t word);

#endif
