/*
 * Round 9, letter b, unsorted_100c: two HUD panes of the multiplayer screen, the score and the status text.
 * Helpers that are not converted (or that another hook file patches) are called through their original address
 * with SIM_FN and the prototypes below.
 */
#ifndef HG_SIM_UNSORTED_100C_W9B_H
#define HG_SIM_UNSORTED_100C_W9B_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Patched by hooks_cache.c / not converted: upper-cases its two texts in place and looks the first up as a resource (0 when absent). */
#define W9BC_CALL_100ba7d5 SIM_FN(void *(__cdecl *)(void *, void *, int), 0x100ba7d5)
/* Not converted: draw a text (pane dword +0xec, x, y, font, text, colour record 0x109b9920); one per display mode. */
#define W9BC_CALL_100f2f01 SIM_FN(int(__cdecl *)(uint32_t, uint32_t, uint32_t, void *, const char *, void *), 0x100f2f01)
#define W9BC_CALL_100f1e10 SIM_FN(int(__cdecl *)(uint32_t, uint32_t, uint32_t, void *, const char *, void *), 0x100f1e10)
#define W9BC_CALL_100ecea6 SIM_FN(int(__cdecl *)(uint32_t, uint32_t, uint32_t, void *, const char *, void *), 0x100ecea6)

/*
 * Draws the score "<text [0x101c4c2c]><number [0x10179cac]>" in the pane (word +4 set: visible; +0xf0 -> x, y;
 * +0xec the target) with the font [0x109b9900] of type [0x1014cec4], during a network game that has started
 * ([0x1015ec58], not [0x1015f208], [0x10179cd4], [0x1017e5e0]); releases the font with Sim_100bb2b0.
 */
void __cdecl Hud_DrawScore(uint8_t *pane);
/*
 * Draws the status text of the own unit's block mode +0x50c (1, 2: the text [0x101c4c48]; 6: the text at
 * 0x1017e5f8 after FlashText_Start(3) and mode 0; any other mode: nothing) in colour 0xf of 0x109b9920 (byte
 * +0xe, restored to 9) with the font [0x109b9900], which is released with Sim_100bb2b0.
 */
void __cdecl Hud_DrawStatusText(uint8_t *pane);

#endif
