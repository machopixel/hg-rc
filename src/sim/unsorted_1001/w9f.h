/*
 * Round 9, letter f: the selection dispatcher 0x10016117 (integer game code of HGSIM.DLL).
 */
#ifndef HG_SIM_UNSORTED_1001_W9F_H
#define HG_SIM_UNSORTED_1001_W9F_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* ---- w9f_select.c ---- */
/*
 * Handles selection `kind` (2..15) for the record whose dword +0x10 is `value` (Sim_10015c61; 0 when there is
 * none) on behalf of the current record cur = PTRTAB([0x1017b8c8]): chooses a selector word from cur +0xf0 (kinds
 * 2, 3, 7, 13), cur +0x118 (kind 14), Sim_10015f5b (kind 15) or a constant (5: 0x2201, 8: 0x2100, 11: 0), with
 * Sim_10020256 messages (code 0xc when nothing is selected, 8 / 9 / 0xa / 0xb otherwise). A selector with bit
 * 0x1000 ends with 0. Otherwise the selection goes out with Sim_10040f5f(cur, idx | 0x200, kind, selector) when
 * the found record (+0x14 = 3) belongs to another player (+0xc), else with Sim_10015d30 (key 0x4200 for the
 * current player, also Sim_10040f5f(cur, 0x4200, ...) in network games [0x1015ec58]); kinds 2, 5 and 7 may end
 * with Sim_10020256(1 / 2 / 3, value). Returns 1.
 */
int __cdecl Sim_10016117(int value, int16_t kind);

#endif
