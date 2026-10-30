/*
 * Round 9, letter g: functions of HGSIM.DLL at 0x100ca670 (target selection stepping) and 0x100cd70f (the
 * mission list of the HUD). Callees that are not converted (or patched by hooks_hud.c) are called through their
 * original address (SIM_FN): 0x100cada2 (who, index, flags) -> int; the height of a font 0x100ecce0; the text
 * drawers 0x100f2f01 / 0x100f1e10 / 0x100ecea6 (surface, x, y, font, text, out rectangle).
 */
#ifndef HG_SIM_UNSORTED_100C_W9G_H
#define HG_SIM_UNSORTED_100C_W9G_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

typedef int(__cdecl *W9gPickFn)(int who, int index, uint32_t flags);
typedef int32_t(__cdecl *W9gHeightFn)(uint32_t font);
typedef void(__cdecl *W9gText6Fn)(uint32_t surface, int32_t x, int32_t y, uint32_t font, const char *text,
                                   void *out);

#define W9GC_CALL_100cada2 SIM_FN(W9gPickFn, 0x100cada2)
#define W9GC_CALL_100ecce0 SIM_FN(W9gHeightFn, 0x100ecce0)
#define W9GC_CALL_100f2f01 SIM_FN(W9gText6Fn, 0x100f2f01)
#define W9GC_CALL_100f1e10 SIM_FN(W9gText6Fn, 0x100f1e10)
#define W9GC_CALL_100ecea6 SIM_FN(W9gText6Fn, 0x100ecea6)

/* ---- w9g_select.c ---- */
/*
 * Steps the selection of the entity `obj` by `delta`: the selection is its dword +0x118 when flags is 1 (set to
 * 0x100 when it was -1; the original then goes on with an unset local), else +0xf0; kind bits 0xf00 (0x100 /
 * 0x200 / 0x400: Sim_100cab7b / 0x100cada2 / Sim_100cb299 on (entity +4, index, flags)), index bits 0xff. With
 * delta 0 or the 0x1000 bit it restarts at 0x200 / 0 with delta 1. Each answer -2 moves to the next kind at 0, -1
 * to the previous kind at its last entry ([0x1015dc68], [0x1017e5a8], [0x1015dc64] - 1), 1 accepts, anything
 * else adds delta; at most [0x1015dc64] + [0x1015dc68] + [0x1017e5a8] + 3 tries. Accepted: kind | index to
 * +0x118 (kind 0x100) or +0xf0; not accepted with kind 0x100: bit 0x1000 of +0x118. Then, unless +0x118 has
 * bit 0x100, when the block's (+0x28) dword +0x50c is set, byte +0x46 of the record +0x54 = 1. The current
 * entity ([0x1017b8c8]) clears [0x1017e5b0] first.
 */
void __cdecl Sim_100ca670(void *obj, int32_t delta, uint32_t flags);

/* ---- w9g_text.c ---- */
/*
 * Draws the mission list of star [0x1013bb40] (table 0x10473800 + star * 0x4fc3a, 0x13f-byte entries from +0x3a)
 * at pos (x, y) on obj's surface (+0xec) with `font`: only the FIRST entry with byte +0x74 set, byte +0 not 5
 * or 6 and dword +0x75 set is drawn (the loop leaves after it): x is reset to obj's +0xb4, the label by the
 * kind byte +1 (1: [0x101c4994]; 2: [0x101c4994]
 * when exactly one active entry exists and it has kind 2 ([0x1015ec58] set and byte +0xa of [0x1017b8c4] clear),
 * else [0x101c498c]; 8: [0x101c49a8]; others [0x101c4988]) is drawn and x advanced by Sim_100ccb0d, then the text
 * +0xad: shorter than [0x10151d60 + 4 * [0x101c64f0]] on one line (y unchanged), else cut after that many
 * characters (at the last space of that part when the next character is not a space) and the rest a line
 * (0x100ecce0(font)) lower, after which x, y are put back and y advanced by one line. Byte [0x109b992e] is 4
 * around the label, 0xc around the text and 9 after.
 */
void __cdecl Sim_100cd70f(void *obj, int32_t *pos, uint32_t font);

#endif
