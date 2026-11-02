/*
 * The sim's loading-screen picture (DECISIONS.md, "The loading screen through Vulkan"): the 8-bit picture of the
 * game's windowed display driver (mode 5) and its palette, in place of the Windows GDI calls that driver made
 * (palette objects, SetDIBitsToDevice, StretchDIBits). The window keeps what was drawn into it, as a window does
 * with GDI; each drawing call shows the result at once. Shown through the Vulkan renderer (picture_hgvk.c).
 */
#ifndef HG_PLATFORM_PICTURE_H
#define HG_PLATFORM_PICTURE_H

#include <stdint.h>

/* Starts showing pictures in the window (Vulkan); 0 on success. When Vulkan cannot start, the player is told why
 * and the program ends (Vulkan only: no other way to show the game). */
int Pic_Open(void *window);
void Pic_Close(void);
/* The palette the next drawing uses: 256 entries of red, green, blue at 0..63 (the game's palette). */
void Pic_SetPalette(const uint8_t *rgb6);
/*
 * Draws the w x h part of the picture (width x height bytes, rows from the top, width bytes apart) whose top left
 * is sx, sy at dx, dy of the window's picture of the same size, clipped to both; returns height (the scan lines
 * SetDIBitsToDevice reports for the whole picture).
 */
int Pic_Blit(const uint8_t *pixels, int width, int height, int dx, int dy, int sx, int sy, int w, int h);
/* Draws the w x h part at sx, sy stretched over the whole window picture; returns height. */
int Pic_Stretch(const uint8_t *pixels, int width, int height, int sx, int sy, int w, int h);

#endif
