/*
 * Round 9, letter e: functions of HGSIM.DLL at 0x1009b516.
 */
#ifndef HG_SIM_UNSORTED_1009_W9E_H
#define HG_SIM_UNSORTED_1009_W9E_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* ---- w9e_misc.c ---- */
/*
 * The colour-selection word of a scene object for the current world ([0x1014c4c8 + 4 * [0x1015d1ac]], whose
 * dword +0x74 points at a table of words): 0 without a world, object or `arg`; the object's class bits (word
 * +0 & 0xf000) when they are 0x8000 or 0x9000; otherwise 0xf0 when [0x1014cb28] is set, or-ed with a table
 * word chosen by the record at object +0x24 (its word +2): class 0x100 = table[Sim_10052be7(record word
 * +0x24)], 0x200 = table[3 + Sim_10052c02(record word +0x24)], 0x400 = table[6], 0x800 = 1; else by its bits
 * 0xf0: 0x10/0x20/0x60 = `fallback` (replacing the 0xf0), 0x40/0x50/0x80 = table[7/8/9], others = 1.
 */
uint32_t __cdecl Sim_1009b516(const uint8_t *obj, int arg, uint32_t fallback);

#endif
