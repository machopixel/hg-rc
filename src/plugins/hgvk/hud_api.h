/*
 * What the hooks in HG.EXE (hooks_hud.c) hand to HGVK.DLL (hud2d.c) to draw
 * the game's 2D HUD at the window's full resolution. All positions are cells of
 * the sim's 640x480 picture; the plugin scales them to pixels. See docs/renderer/hud.md.
 */
#ifndef HGVK_HUD_API_H
#define HGVK_HUD_API_H

#include <stdint.h>

#define HUD_API_VERSION 1
#define HUD_CELLS_W 640
#define HUD_CELLS_H 480

/* Cells, both ends included. Everything drawn is cut to this rectangle. */
typedef struct HudRect {
    int x0, y0, x1, y1;
} HudRect;

typedef struct HudApi {
    unsigned version;
    /* The 640x480 RGB565 buffer the sim draws into, or NULL when the full-resolution layer is off. */
    const void *(*Target)(void);
    /* A one-cell-wide line. color is RGB565; 0 draws nothing. */
    void (*Line)(const HudRect *clip, int x0, int y0, int x1, int y1, unsigned color);
    /* Fills the clip rectangle. color 0 clears it to transparent. */
    void (*Fill)(const HudRect *clip, unsigned color);
    /* w x h RGB565 cells with their top-left cell at (x, y); 0 is transparent. */
    void (*Bitmap)(const HudRect *clip, int x, int y, int w, int h, const uint16_t *pixels);
} HudApi;

#endif
