/*
 * Round 9, letter f: the side table at 0x10473800 (one block of 0x4fc3a bytes per side, [0x104737e0] sides;
 * rows of 0x13f bytes from +0x3a, count +0, byte state +0x38) and its users: the loader 0x10078f60, the
 * per-row evaluation 0x1007b5db with its helpers 0x1007adfc / 0x1007b1d0, the per-frame walk 0x1007c42f and the
 * lookup 0x1007cbcb; also the panel refresh 0x10078a7f.
 */
#ifndef HG_SIM_UNSORTED_1007_W9F_H
#define HG_SIM_UNSORTED_1007_W9F_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Patched by hooks_sim.c: the game's error report (code, argument) (0x100bc280). */
#define W9F7_CALL_100bc280 SIM_FN(void(__cdecl *)(int, uint32_t), 0x100bc280)
/* Not converted: the panel step on an object (0x100782a9). */
#define W9F7_CALL_100782a9 SIM_FN(void(__cdecl *)(uint8_t *), 0x100782a9)
/* Not converted: three tests of a row's id (index byte, kind byte) for a side (0x1007a005, 0x1007a4af, 0x1007a955). */
#define W9F7_CALL_1007a005 SIM_FN(int(__cdecl *)(const uint8_t *, int), 0x1007a005)
#define W9F7_CALL_1007a4af SIM_FN(int(__cdecl *)(const uint8_t *, int), 0x1007a4af)
#define W9F7_CALL_1007a955 SIM_FN(int(__cdecl *)(const uint8_t *, int), 0x1007a955)

/* The side block and row b of side a (see the file comment). */
#define W9F_SIDE(a) (GBYTES(0x10473800) + (uint32_t)(a) * 0x4fc3au)
#define W9F_ROW(a, b) (GBYTES(0x1047383a) + (uint32_t)(a) * 0x4fc3au + (uint32_t)(b) * 0x13fu)

/* ---- w9f_panel.c ---- */
/*
 * Like Sim_10078950 (w7n_misc.c) with Sim_1008f867(1, pair) and nothing done when it gives no rectangle: for an
 * object with word +4 set and dword +0x30 not 0, 1 or 2 that has a pair +0xf4, resets the pair unless dword
 * +0xf8 is 0, 3 or 4, and with a rectangle swaps it into the entry 0x10185400 + dword +0xc * 0x14 and the record
 * +0xec around 0x100782a9(obj). Then dword +0xf8 = [0x109e89d8].
 */
void __cdecl Sim_10078a7f(uint8_t *obj);

/* ---- w9f_sides.c ---- */
/*
 * Fills the side block of p +8 from the record p: (p +4 - 0x26) / 0x97 members of 0x97 bytes from p +0x26
 * (error 0x4a reported when dword p +0x26 is not 0x10): names p +0x12 / +0x1d to +0x18 / +0x28, byte +0x39 from
 * 0x109b7d4c[side * 0x6c], dword +0xc from p +0xc, +4 / +8 = -1. Each row gets its member's bytes and dwords
 * (see the code), the texts +0x89 / +0x99 / +0xad, 'V' / 'M' flags, 8 cells of (kind C=1 S=2 F=3 else 0,
 * row, map) from the letters at +0x30, state 0, no ids (+0xee) and word +0xef = 0x800. Finally
 * [0x104727a0 + side * 4] = 0, the row count, and [0x104737e0] raised to side + 1.
 */
void __cdecl Sim_10078f60(const uint8_t *p);
/*
 * The word +0x129 of the first row of side with dword +2 = 0x20 (of the side block when none); the high half of
 * the result is whatever the index arithmetic left in eax, as in the original.
 */
uint32_t __cdecl Sim_1007cbcb(int side);

/* ---- w9f_objectives.c ---- */
/*
 * Sound of row j of side (only for the current side [0x1013bb40] and while [0x104727e0 + j * 4] is 0): code 5
 * uses the text +0x89 (with [0x101c469c]), code 6 the text +0x99 (with [0x101c468c]); when that name contains
 * "avon" / "avos" the byte +0x8c (of the +0x89 name, for both codes) gets 's' / 'n' by the dword +0x613 of the
 * current block, the name is copied and given to
 * Sim_1007e7b6 and Sim_10020441, "%s %s" of text +0xad is collapsed with Str_CollapseSpaces when its first
 * character is not a space and byte +0x74 is set (the global empty text 0x10179d34 otherwise). Then the flag is
 * set. Returns 1. NOTE: with a code other than 5 or 6 the original reads two uninitialised locals (the callers
 * only pass 5 and 6).
 */
int __cdecl Sim_1007adfc(int side, int j, int code);
/*
 * The same for the side's own texts: code 2 text +0x18 (also [0x10179d30] = 1 and Sim_10041c00 in network
 * games), code 3 text +0x28, code 4 "BET68"; nothing while [0x10179d30] and [0x1015ec58] are set or for another
 * side. Returns 1. NOTE: with any other code the name given to Sim_10020441 is an uninitialised local in the
 * original (Sim_1007c42f can pass 0).
 */
int __cdecl Sim_1007b1d0(int side, int code);
/*
 * Evaluates row j of side: a row in state 5 or 6, or one Sim_1007b4d6 rejects, gets byte +0x73 = 0. Otherwise
 * the first time (+0x67 = -1) +0x67 = [0x1037305c] and +0x73 = 1; the dword +2 selects the test over the ids at
 * +0xef (Sim_10079e3b / Sim_10042908 / Sim_10079ecd / Sim_10079c86 / 0x1007a005 / 0x1007a4af / 0x1007a955, see
 * the code) giving `ok` and `all`, then (with `expired` = [0x1037305c] - +0x67 > +0x6f) the same dword selects
 * the outcome: state 5 (with Sim_1007adfc(side, j, 5)), 6 or 8, side commands 0x10000 / 0x20000 (state 2 or 3
 * of the side +0xa9 through Sim_1007b1d0), the byte +0x74 of another row toggled (0x40000), other rows set to
 * state 6 / 5 or reset (0x80000..0x400000). The state is
 * stored at +0 and +0x6b = [0x1037305c] unless it is 3.
 * NOTE: for dword +2 = 0x400000 the original clears [0x104727e0 + i * 4] with the loop counter i that no loop
 * of this call has set (a stale stack word); this C uses 0 there.
 */
void __cdecl Sim_1007b5db(int side, int j);
/*
 * Per-frame walk: [0x1037305c] = [0x1015dc78] / 0xb5, then for every side all rows through Sim_1007b5db; a side
 * in state 0 moves to 2 (all flagged rows done, or [0x10179d2c]), 3 (a flagged row failed) or 4 (time +0xc
 * over) with Sim_1007b1d0 and +8 = [0x1037305c]; the current side in another state (networked or
 * [0x1015f1e8] = 0) sets [0x1015f1fc] once all sides are done (Sim_100419e9 when networked). The first row
 * Sim_1007b4d6 accepts without high bits in +2 becomes [0x104727a0 + side * 4]; when it changes, a row of kind
 * 0x4000 marks its ids (PTRTAB(i) +0x150 or 0x109e9bec + i * 0x50 = 0x80000000) and Sim_1001661f(side) runs.
 */
void __cdecl Sim_1007c42f(void);

#endif
