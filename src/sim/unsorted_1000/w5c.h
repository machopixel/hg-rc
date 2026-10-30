/*
 * Round 5, letter c: functions at 0x10007ed5..0x10013eba that call helpers which are not converted yet.
 * Those helpers are called through their original address (SIM_FN) with the prototypes below.
 */
#ifndef HG_SIM_UNSORTED_1000_W5C_H
#define HG_SIM_UNSORTED_1000_W5C_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Not converted: the C library sprintf (destination, format, ...). */
#define SIM_CALL_100fc780 SIM_FN(int(__cdecl *)(char *, const char *, ...), 0x100fc780)
/* Not converted: returns a count that the caller adds to the total at 0x1015d010. */
#define SIM_CALL_100097d3 SIM_FN(int(__cdecl *)(const void *), 0x100097d3)
/* Not converted: (slot, state); the state is 14, 3 or 8 below. */
#define SIM_CALL_10016117 SIM_FN(void(__cdecl *)(int, int), 0x10016117)
/* Not converted: (object, word, word, flag); both words are read as 16-bit values. */
#define SIM_CALL_10015630 SIM_FN(void(__cdecl *)(void *, int16_t, int16_t, int), 0x10015630)

/* Formats two lines of 40 characters from the strings at 0x1012dd94 / 0x1012dc50 and shows them on the text screen. */
void __cdecl Sim_10007ed5(void);
/* Adds the two values of Sim_100094df and the result of 0x100097d3 to the totals at 0x1015d00c..0x1015d018. */
void __cdecl Sim_10007f49(const void *obj);
/* If slot `slot` (< 8) resolves and its record's field +0x4f0 is 2: stores 2 / 1 / 5 in the dword table at 0x1012ff58 and calls 0x10016117. */
void __cdecl Sim_10010496(int slot, int unused);
void __cdecl Sim_100105cb(int slot, int unused);
void __cdecl Sim_10010700(int slot, int unused);
/* Pops the pair stored at obj+0x180 (count at +0x198) and calls 0x10015630 with it; returns 1 when the stack was empty. */
int __cdecl Sim_10013eba(void *obj, int unused);

#endif
