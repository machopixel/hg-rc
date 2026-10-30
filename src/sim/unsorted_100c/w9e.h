/*
 * Round 9, letter e: functions of HGSIM.DLL at 0x100ce983 (a HUD text block). Helpers that are not converted
 * (or patched elsewhere) are called through their original address with the prototypes below.
 */
#ifndef HG_SIM_UNSORTED_100C_W9E_H
#define HG_SIM_UNSORTED_100C_W9E_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Patched by hooks_conv.c: finds a resource (name, extension, out size); upper-cases both texts in place. */
#define W9E_CALL_100ba7d5 SIM_FN(void *(__cdecl *)(char *, char *, uint32_t *), 0x100ba7d5)
/* Not converted: the height of a font. */
#define W9E_CALL_100ecce0 SIM_FN(int(__cdecl *)(void *), 0x100ecce0)
/* Not converted: draws a text (pane, x, y, font, text, colours at 0x109b9920): 8-bit, 16-bit and double-byte
 * versions. */
typedef void(__cdecl *W9eDrawTextFn)(void *pane, int32_t x, int32_t y, void *font, const char *text, void *colours);
#define W9E_CALL_100ecea6 SIM_FN(W9eDrawTextFn, 0x100ecea6)
#define W9E_CALL_100f1e10 SIM_FN(W9eDrawTextFn, 0x100f1e10)
#define W9E_CALL_100f2f01 SIM_FN(W9eDrawTextFn, 0x100f2f01)

/* ---- w9e_hud.c ---- */
/*
 * Draws the message block of the HUD `hud` (pane at +0xec, position pair at +0xf0) when it is shown (word +4)
 * and there is a mode [0x1015f208] or a chat target [0x10151d50] != -2, with the font [0x109b9900] (released
 * with Sim_100bb2b0 at the end): the mode line (1: "<prefix><name of unit 0x1015ef18> <label>", colour 7 when
 * Sim_10052be7 of that unit is set; 2 and 3: fixed texts), then for the chat target -1 the text [0x101c4c18]
 * underlined and the texts for the state byte +0xa of [0x1017b8c4], for a target 0..7 that is a live unit
 * (dword +0 == 1, word +0x1c without 0x4800; the own unit index maps to -2 and ends the function without the
 * release) its name underlined and [0x101c4c20], and finally the input line 0x1015dba0 in a box with the
 * cursor " " after its width (Sim_100ccb0d). The text colour byte 0x109b992e is set around every text.
 */
void __cdecl Sim_100ce983(uint8_t *hud);

#endif
