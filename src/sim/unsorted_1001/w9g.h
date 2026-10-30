/*
 * Round 9, letter g: functions of HGSIM.DLL at 0x10012af0 and 0x10016de7 (integer game code).
 * The per-star mission table is 0x10473800 + star * 0x4fc3a (dword +0 count; the entries of 0x13f bytes start at
 * +0x3a: byte +0 state, byte +1 kind, word +2 key, dwords +0x67 / +0x6b / +0x6f times, byte +0x73 active, text
 * +0xad).
 */
#ifndef HG_SIM_UNSORTED_1001_W9G_H
#define HG_SIM_UNSORTED_1001_W9G_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* ---- w9g_mono.c ---- */
/*
 * Mono-screen page of star `star`'s missions: a headline "Star %d mission: ..." from the byte +0x38 (0: "%d sec
 * left" / "in progress", 2: "Successful", 3: "Failed", 4: "Out of time"; 1 and above 4 print the line buffer as
 * it was) on row 3, then one row per entry from [0x101303d8] to [0x101303d4] (a window of 0x15 rows moved
 * after the cursor [0x1015d108], blanked by Mono_BlankRows when it moves) with the kind ("Primary  " ...),
 * the key's name from the pair table 0x10130678 and the state. Printed only when [0x10a3ade8] is set.
 */
void __cdecl Mono_ShowMissions(int star);

/* ---- w9g_select.c ---- */
/*
 * Gives key `key` (16 bits; bits 0xf000 set: nothing, 0) to a member of group `kind`: with the group's current
 * entry (0x104727a0[kind]) of kind 4 every eligible member (and the fifth when the group's dword +8 is set) gets
 * Sim_10015630(member, 2, key, 1) and the result is 1 when any did; otherwise the eligible member other than
 * Sim_10052aaf(kind) whose word +0x1ac is nearest to that of PTRTAB(key & 0xff) (when key's bits 0xf00 are
 * 0x200, else 0) gets it, or the Sim_10052aaf(kind) one when none, with bit 0x20 set and 0x40 cleared in its
 * dword +0x1ae; 1 when one was found. Eligible: word +0x1a6 not 2 or 3, Sim_100c3668(dword +0x28) == 0, dword
 * +0x1b2 set (and, for the nearest search, no bits 0..1 in +0x1ae). Nothing (0) when Sim_10052aaf(kind) is -1
 * or Sim_10016cb7(kind, list, 1) lists nobody.
 */
int __cdecl Sim_10016de7(int kind, uint32_t key);

#endif
