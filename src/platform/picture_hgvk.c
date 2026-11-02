/*
 * The loading-screen picture (picture.h) through the Vulkan renderer HGVK.DLL (its HgvkPictureApi). The window's
 * picture is kept here in 32 bits, as the window kept what GDI drew into it; the palette in force at each drawing
 * call colours what that call draws, as with GDI.
 */
#include <stdlib.h>
#include <string.h>

#include "picture.h"
#include "plat.h"
#include "../plugins/hgvk/picture_api.h"

static const HgvkPictureFns *g_api;
static void *g_window;
static uint32_t g_palette[256];     /* bytes R, G, B, A in memory order */
static uint32_t *g_canvas;
static int g_canvasW, g_canvasH;

int Pic_Open(void *window)
{
    void *lib;
    HgvkPictureApiFn get;

    if (g_api == NULL) {
        lib = Plat_LoadLibrary("HGVK.DLL");
        get = lib != NULL ? (HgvkPictureApiFn)Plat_LibraryFunction(lib, "HgvkPictureApi") : NULL;
        g_api = get != NULL ? get() : NULL;
    }
    if (g_api == NULL || g_api->Open(window) != 0) {
        /* Vulkan only (Diego, 2026-11-02): no other way to show the game */
        Plat_MessageBox(window,
                        "Heavy Gear could not start Vulkan.\n\nIt needs HGVK.DLL next to HG.EXE and a graphics card "
                        "driver with Vulkan support. See hgvk.log in the game folder for details.",
                        "Heavy Gear", 0x10);
        exit(1);
    }
    g_window = window;
    return 0;
}

void Pic_Close(void)
{
    if (g_api != NULL && g_window != NULL)
        g_api->Close();
    g_window = NULL;
    free(g_canvas);
    g_canvas = NULL;
    g_canvasW = g_canvasH = 0;
}

void Pic_SetPalette(const uint8_t *rgb6)
{
    int i;

    for (i = 0; i < 256; i++) {
        /* 0..63 to 0..252, as the GDI palette entries were made (value << 2) */
        g_palette[i] = (uint32_t)(uint8_t)(rgb6[i * 3] << 2) | (uint32_t)(uint8_t)(rgb6[i * 3 + 1] << 2) << 8 |
                       (uint32_t)(uint8_t)(rgb6[i * 3 + 2] << 2) << 16 | 0xff000000u;
    }
}

/* The window's picture at width x height (a new size starts black). */
static int Canvas(int width, int height)
{
    if (width <= 0 || height <= 0)
        return 0;
    if (g_canvas == NULL || g_canvasW != width || g_canvasH != height) {
        free(g_canvas);
        g_canvas = (uint32_t *)calloc((size_t)width * (size_t)height, sizeof(uint32_t));
        g_canvasW = g_canvas != NULL ? width : 0;
        g_canvasH = g_canvas != NULL ? height : 0;
    }
    return g_canvas != NULL;
}

static void Show(void)
{
    if (g_api != NULL && g_window != NULL)
        g_api->Show(g_canvas, g_canvasW, g_canvasH);
}

int Pic_Blit(const uint8_t *pixels, int width, int height, int dx, int dy, int sx, int sy, int w, int h)
{
    int x, y;

    if (pixels == NULL || !Canvas(width, height))
        return height;
    /* clip to the source and to the window picture */
    if (sx < 0) {
        dx -= sx, w += sx, sx = 0;
    }
    if (sy < 0) {
        dy -= sy, h += sy, sy = 0;
    }
    if (dx < 0) {
        sx -= dx, w += dx, dx = 0;
    }
    if (dy < 0) {
        sy -= dy, h += dy, dy = 0;
    }
    if (sx + w > width)
        w = width - sx;
    if (dx + w > width)
        w = width - dx;
    if (sy + h > height)
        h = height - sy;
    if (dy + h > height)
        h = height - dy;
    for (y = 0; y < h; y++) {
        const uint8_t *from = pixels + (size_t)(sy + y) * (size_t)width + (size_t)sx;
        uint32_t *to = g_canvas + (size_t)(dy + y) * (size_t)width + (size_t)dx;

        for (x = 0; x < w; x++)
            to[x] = g_palette[from[x]];
    }
    Show();
    return height;
}

int Pic_Stretch(const uint8_t *pixels, int width, int height, int sx, int sy, int w, int h)
{
    int x, y;

    if (pixels == NULL || w <= 0 || h <= 0 || sx < 0 || sy < 0 || sx + w > width || sy + h > height ||
        !Canvas(width, height))
        return height;
    for (y = 0; y < height; y++) {
        /* each pixel takes the source pixel under its centre, as GDI's StretchDIBits picks it */
        const uint8_t *from = pixels + (size_t)(sy + (int)((2 * (int64_t)y + 1) * h / (2 * (int64_t)height))) * (size_t)width +
                              (size_t)sx;
        uint32_t *to = g_canvas + (size_t)y * (size_t)width;

        for (x = 0; x < width; x++)
            to[x] = g_palette[from[(2 * (int64_t)x + 1) * w / (2 * (int64_t)width)]];
    }
    Show();
    return height;
}
