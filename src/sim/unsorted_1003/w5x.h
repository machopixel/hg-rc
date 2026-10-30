/*
 * Round 5, letter x: two functions at 0x10034777 and 0x100347b9 that call helpers which are not
 * converted yet (called through their address, SIM_FN).
 */
#ifndef HG_SIM_UNSORTED_1003_W5X_H
#define HG_SIM_UNSORTED_1003_W5X_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Not converted. 0x100fd850 (name, mode) is the library's fopen: it calls the stream opener with the share flag 0x40. */
#define SIM_CALL_1003462f SIM_FN(void(__cdecl *)(const char *), 0x1003462f)
#define SIM_CALL_100fd850 SIM_FN(uint32_t(__cdecl *)(const void *, const void *), 0x100fd850)

/* Runs 0x1003462f(s + i) for i = 0, 0x50, 0xa0, ... while i < Str_Length(s). */
void __cdecl Sim_10034777(const char *s);
/* [0x1015e500] = 0x100fd850(0x10139e58, 0x10139fb8). */
void __cdecl Sim_100347b9(void);

#endif
