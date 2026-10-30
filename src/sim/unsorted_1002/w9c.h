/*
 * Round 9, letter c: the record initialiser at 0x1002a2f6 (integer game code).
 */
#ifndef HG_SIM_UNSORTED_1002_W9C_H
#define HG_SIM_UNSORTED_1002_W9C_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* ---- w9c_record.c ---- */
/*
 * Clears the record Sim_1002a1f2 allocates for a pointer-table entry: the dwords +0..+0x54 (words +0x1c, +0x1e),
 * +0x58..+0x94, +0xa8, +0xcc..+0x120 and the 0x16 bytes at +0x124 become 0 except +0x24, +0x84, +0xf0 and
 * +0x118 (-1) and +0x150 (0x80000000); the two 16-byte timers at +0xac and +0xbc are set with
 * Sim_10047aa0(timer, 50000, 50000, 0.2 / 20.0). +0x16c is cleared too.
 */
void __cdecl Sim_1002a2f6(void *rec);

#endif
