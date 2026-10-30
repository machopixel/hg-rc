/*
 * Round 6, letter e: functions at 0x10075124..0x10079a80. Helpers that are not converted yet are called through their
 * original address (SIM_FN) with the prototypes below.
 */
#ifndef HG_SIM_UNSORTED_1007_W6E_H
#define HG_SIM_UNSORTED_1007_W6E_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Not converted: (name, table); looks the 8-character name up and lowers its count; returns 1 when found. */
#define W6E_CALL_100bb2b0 SIM_FN(uint32_t(__cdecl *)(uint32_t, uint32_t), 0x100bb2b0)
/* Not converted: the C library _isctype (character, mask), used when [0x1015965c] (__mb_cur_max) > 1. */
#define W6E_CALL_100fd9a0 SIM_FN(int(__cdecl *)(int, int), 0x100fd9a0)

/*
 * Releases the block obj+0xc: the names at +0x10, +0x1c (table [0x1014cec0]) and +0x44 (table [0x1014cec4]) whose
 * flags at +0x14, +0x20 and +0x48 are set go through 0x100bb2b0 and their flag is cleared; 0x1005571d runs when bit 0
 * of +4 is set. Then clears obj+0xc and decrements [0x1015dbcc].
 */
void __cdecl Sim_10075124(uint8_t *obj);
/* Replaces every run of white space (C library class 8) that follows a character of s by one blank, in place. */
void __cdecl Str_CollapseSpaces(char *s);

#endif
