/*
 * Round 9, letter d: the unit state leave-handler of HGSIM.DLL at 0x100bfa33.
 */
#ifndef HG_SIM_UNSORTED_100B_W9D_H
#define HG_SIM_UNSORTED_100B_W9D_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* ---- w9d_states.c ---- */
/*
 * Clean-up by the state word +0x1f8 of the unit (0..0x1b, anything else does nothing), then Sim_100bf9a1(obj):
 *  1 Sim_1001570f and, with bit 0x200 of the word +0x1aa, the byte +0x270 + (+0x218 / 2) of PTRTAB(low byte of
 *    +0x1aa) decremented; 4 +0x204, +0x218 = 0; 5 Sim_100c274a(obj, 0), record bytes +0x21, +0x22 = 0; 7, 0xc,
 *    0xd Sim_1001570f; 8, 0xa record byte +0x59 = 0; 9 Sim_100c274a(obj, 0); 0xb record bytes +0x1f..+0x22 = 0
 *    then Sim_100c274a(obj, 0); 0xe, 0x16, 0x17 the weapon release (bit 0x100 of word +0x1a8: Sim_100ca417(+4,
 *    word +0x1a8), word +0x1a8 = word +0x1aa), +0x218..+0x220 (0x16, 0x17 also +0x224) = 0 and when word +0x22c
 *    is -1 or one of [0x1017e2a8], [0x1017e2ac], [0x1017e2b0] is 1: words +0x1f8, +0x1fa = -1, dwords +0x22e,
 *    +0x23a = -1; 0x10 the entry Sim_10033c42(+0x23a) (when +0x23a >= 0 and found) gets Sim_100335cf(entry, obj),
 *    then Sim_100c48c9; 0x11 the same after words +0x1f8/+0x1fa = -1 when word +0x22c is -1, then +0x218..+0x228
 *    and +0x204 = 0 and Sim_100c48fa(obj, 2); 0x13 the weapon release and +0x218 = 0; 0x14 +0x204, +0x218, +0x21c
 *    = 0; 0x15 +0x218, +0x204 = 0, record dword +8 = 0, bytes +0x59, +0x56 = 0; 0x1a +0x218 = 0 and
 *    Sim_100c48c9; 0x1b +0x218 = 0 and Sim_100c48fa(obj, 2). The record is *(obj + 0x54).
 */
void __cdecl Sim_100bfa33(void *obj);

#endif
