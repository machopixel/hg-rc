/*
 * Round 30: the sim's 2D drawing library (the HUD's lines, shapes, fills and text) in C (DECISIONS.md, "Round 30").
 */
#ifndef HG_SIM_RENDER_W30_H
#define HG_SIM_RENDER_W30_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * VFX line draw (pane, x0, y0, x1, y1, mode, colour): mode other than 0 returns -3; an empty window -1, an empty
 * pane -2. The ends (pane-relative) are cut to the pane by outcodes with the original's 0.32 fixed-point rounding;
 * a line wholly outside returns 2. The cells are those of the original's stepping (x- or y-major, diagonals,
 * straight lines). Returns 1 when it was cut, else 0 (straight lines: 0, see DECISIONS.md round 30).
 */
int32_t __cdecl Sim_100ef727(void *pane, int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t mode,
                             uint32_t color);
/* VFX pane wipe (pane, colour): the pane (cut to the window) filled with the colour. -1 / -2 as the line, else 0. */
int32_t __cdecl Sim_100f0865(void *pane, uint32_t color);
/*
 * VFX shape draw without clipping (pane, shape, x, y, slot): the shape's run-length rows (bounds at +8, rows from
 * +0x18; colour codes through the table 0x10156dcc) at (x, y) in the pane. The original keeps the window's pitch in
 * the fifth argument's stack slot. Returns 0.
 */
int32_t __cdecl Sim_100f0727(void *pane, const uint8_t *shape, int32_t x, int32_t y, int32_t slot);
/*
 * VFX shape draw (pane, shape table, number, x, y): shape number of the table (offset at +8 + 8 * number) cut to
 * the pane: -4 for empty bounds, -3 wholly outside, the unclipped routine when wholly inside, else rows above the
 * pane skipped and each row cut on the left and / or right. -1 / -2 as the line, else 0.
 */
int32_t __cdecl Sim_100f0205(void *pane, const uint8_t *table, int32_t number, int32_t x, int32_t y);
/*
 * VFX character draw (pane, x, y, font, code, lookaside): the glyph (font +0x10 + 4 * code: width, then width x
 * height bytes; height at font +8) cut to the pane, each byte through the 256-byte lookaside (0xff transparent)
 * and the colour table. Returns the glyph's width (0 for an empty glyph; -1 / -2 as the line).
 */
int32_t __cdecl Sim_100f1c93(void *pane, int32_t x, int32_t y, const uint8_t *font, int32_t code,
                             const uint8_t *lookaside);
/* VFX string draw (pane, x, y, font, text, lookaside): the characters one after another (the first even for an
 * empty text); returns the last character's result. */
int32_t __cdecl Sim_100f1e10(void *pane, int32_t x, int32_t y, const uint8_t *font, const uint8_t *text,
                             const uint8_t *lookaside);
/*
 * The 8-bit set's 1-bit font character (pane, x, y, font, code, lookaside): one or two byte codes (table
 * [0x10157590], length to [0x10157594]); the colour byte [0x10157598] (through the lookaside when given, 0xff
 * transparent) set for each 1 bit, one byte per cell, with the original's clipping and bit-count quirks. Returns
 * the glyph's width.
 */
int32_t __cdecl Sim_100f2d3c(void *pane, int32_t x, int32_t y, const uint8_t *font, uint32_t code,
                             const uint8_t *lookaside);
/*
 * The entry points hooked at the library's addresses (w30_hud.c): each runs the routine above and, with the
 * full-resolution HUD on (the hook layer sets g_w30HudApi / g_w30Scratch / g_w30Cells), draws a pane of the
 * renderer's HUD buffer into scratch memory and hands the same drawing to the renderer.
 */
int32_t __cdecl W30_Line(void *pane, int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t mode, uint32_t color);
int32_t __cdecl W30_Shape(void *pane, const uint8_t *table, int32_t number, int32_t x, int32_t y);
int32_t __cdecl W30_ShapeRaw(void *pane, const uint8_t *shape, int32_t x, int32_t y, int32_t slot);
int32_t __cdecl W30_Wipe(void *pane, uint32_t color);
int32_t __cdecl W30_Char(void *pane, int32_t x, int32_t y, const uint8_t *font, int32_t code,
                         const uint8_t *lookaside);
struct HudApi;
extern const struct HudApi *(*g_w30HudApi)(void);
extern uint16_t *(*g_w30Scratch)(uint32_t bytes);
extern uint16_t *(*g_w30Cells)(uint32_t cells);
/* Its string routine: characters (16-bit reads) advanced by [0x10157594] bytes until a 0 byte. */
int32_t __cdecl Sim_100f2f01(void *pane, int32_t x, int32_t y, const uint8_t *font, const uint8_t *text,
                             const uint8_t *lookaside);

#endif
