/*
 * Showing a finished picture in the game window. Backend-neutral; today SDL3
 * (display_sdl3.c). The picture is 32-bit 0xAARRGGBB; the window can be any size
 * and the picture is scaled to fit it, keeping its proportions.
 */
#ifndef HG_PLATFORM_DISPLAY_H
#define HG_PLATFORM_DISPLAY_H

#include <stdint.h>

/* Takes over the drawing of `nativeWindow` (an HWND) for pictures of width x height. */
int Display_Open(void *nativeWindow, int width, int height);
void Display_Close(void);
int Display_IsOpen(void);

/*
 * Where the picture sits in the window: scaled up as far as the window allows,
 * centred, with the proportions kept. These convert a point between window
 * client pixels and picture pixels (the result can lie outside the picture).
 */
void Display_ClientToPicture(int *x, int *y);
void Display_PictureToClient(int *x, int *y);

/* Shows the picture (width x height, rows packed with no padding). */
void Display_Present(const uint32_t *argb);

#endif
