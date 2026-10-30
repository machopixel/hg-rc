/*
 * The full-resolution 2D layer: a CPU-side RGBA picture the size of the 4:3
 * area of the window, drawn into by the HUD hooks (hud_api.h) during the
 * frame and put over the 3D view by vk.c. Pixels are premultiplied RGBA.
 */
#ifndef HGVK_HUD2D_H
#define HGVK_HUD2D_H

#include <stdint.h>

#include "hud_api.h"

/* Tells the layer where the sim's RGB565 buffer is (NULL: not available). */
void Hud_SetTarget(const void *rgb565);

/* Starts a frame for a window of this client size: sizes the layer and clears it. */
void Hud_Frame(int clientW, int clientH);

/* The picture, or NULL when nothing was drawn this frame. */
const uint32_t *Hud_Pixels(int *w, int *h);

/* F12: write the picture over black as a 24-bit bitmap, and log what was drawn. */
void Hud_Dump(const char *bmpPath);

const HudApi *Hud_Api(void);

#endif
