/*
 * Round 9, letter h: the order dispatcher of a group entry (0x10017263), integer game code.
 */
#ifndef HG_SIM_UNSORTED_1001_W9H_H
#define HG_SIM_UNSORTED_1001_W9H_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* ---- w9h_group.c ---- */
/*
 * For group `index` (idx = Sim_10052aaf(index), its record PTRTAB(idx)): hands a pending word +0x1a0 of the record to
 * Sim_10016d83(index, rec + 0x1a0) and clears it. Then, unless idx is the player [0x1017b8c8] or the record's dword
 * +0x14 is 0 or 1, takes the entry 0x10473800 + index * 0x4fc3a + [0x104727a0 + index * 4] * 0x13f + 0x3a (selection
 * -1: nothing) with flags = dword +2 (0 or any of 0xe10: nothing) and the records Sim_10016cb7(index, list, 0) lists.
 * Walks the entry's words (+0xef, count byte +0xee) until Sim_100152fc(index, word, flags) is 0 and fewer than
 * `need` (dword +0x79, or the list count) listed records with word +0x1aa equal to it have the kind (word +0x1a6) the
 * flags ask for (1,2: 2 or 3; 4: 7; 8: 2; 0x20/0x100/0x200/0x400: 8; 0x1000: any; 0x2000: 4; 0x4000: 2 or 3). With such a
 * word: the target is the first listed record of kind 0 (or kind 5 with Sim_100c3668(+0x28) == 0) other than the
 * group's own record, else the own record under the same test (else nothing). Sim_10016a6b(target, 2, lowest set bit
 * of flags + 1, idx) for the first of the low 16 flag bits set; mode 7 for flags 4, 2 for 1/2/8/0x4000, 8 for
 * 0x20/0x100/0x200/0x400, 4 for 0x2000, 0 for 0x1000 (after Sim_10025f6b(target)); then Sim_1001555c(target, mode,
 * word) and target +0x1ae |= 0x40 | Sim_10016a4a(entry dword +0x7d).
 */
void __cdecl Sim_10017263(int index);

#endif
