/*
 * Round 9, letter d: functions of HGSIM.DLL at 0x10016632, 0x10017b3e and 0x1001edd0 (integer game code).
 */
#ifndef HG_SIM_UNSORTED_1001_W9D_H
#define HG_SIM_UNSORTED_1001_W9D_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* ---- w9d_select.c ---- */
/*
 * Applies the selection of group `index`: the records Sim_10016cb7(index, list, 1) lists (and the fifth one when
 * the group's dword +8 is set) that have dword +0x14 = 2, or 3 with +0xc = [0x1017b8c8], get
 * Sim_10016a6b(rec, 2, first set bit of the kind word + 1, idx), Sim_1001555c(rec, mode, idx | 0x200) (or (0, 0)
 * when rec +4 is idx and the mode is 5), bits 0..1 of +0x1ae replaced by 0x40 | Sim_10016a4a(entry +0xb7), and
 * Sim_10017852(rec). idx = (short)Sim_10052aaf(index), or Sim_10012843(index) when that is -1 (nothing happens
 * when both are). The entry is 0x10473800 + index * 0x4fc3a + [0x104727a0 + index * 4] * 0x13f, its kind the
 * word +0x3c (0x200 when the selection is -1; the +0xb7 dword is read even then): 0x400 gives mode 10, 0x800 or
 * 0x10 mode 11, anything else mode 5. A record is left alone when [0x1015d120 + index * 4] is set and (with
 * [0x1015ec58]) PTRTAB(idx) +0x14 is 0 or 1, or (without) idx is [0x1017b8c8]. Finally [0x1015d120 + index * 4]
 * = 1 under the same condition on Sim_10052aaf(index) unless the kind is 0x10.
 */
void __cdecl Sim_10016632(int index);

/* ---- w9d_misc.c ---- */
/* obj +0x23e = value and dword +0x58c of the block obj +0x28 = 1. */
void __cdecl Sim_10017b3e(void *obj, uint32_t value);
/*
 * Settings by number: 0 [0x101321b4], 1 [0x101321b8], 2 [0x101321bc] (also sets bit 3 of [0x101321c0] and calls
 * Cd_StopWait unless [0x1015ec58]), 3 [0x101321b0], 4 bit 1 of [0x101321c0] as 0 / 1; anything else 0.
 */
uint32_t __cdecl Sim_1001edd0(int which);

#endif
