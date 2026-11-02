/* Native check of src/platform/picture_hgvk.c (the loading-screen picture, DECISIONS.md "The loading screen through
 * Vulkan") against Windows GDI: the windowed display driver's three kinds of drawing, called as the original called
 * GDI (SetDIBitsToDevice of a rectangle, Gdi_BitBltRect; of the whole picture from a scan line, Sim_10083c45 and
 * Sim_10083801's W11_Blit; StretchDIBits of a part over the whole, Sim_10083de1), on random 8-bit top-down pictures
 * and palettes, into a 32-bit bitmap of the same size; after each call the two pictures must be the same pixels.
 * Widths are multiples of 4: GDI reads 8-bit rows rounded up to 4 bytes, while the game packs its rows width bytes
 * apart (a desktop width such as 1366 showed the original picture sheared; ours keeps the rows, DECISIONS.md).
 * HGVK is a stand-in that keeps what Show is given. Build and run (32-bit mingw, from the repository root):
 *   i686-w64-mingw32-gcc -O2 -I src -o picture_native.exe tools/verify/picture_native.c src/platform/picture_hgvk.c
 *       -lgdi32
 *   picture_native.exe
 */
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "platform/picture.h"
#include "platform/plat.h"
#include "plugins/hgvk/picture_api.h"

/* ---- HGVK and the platform calls picture_hgvk.c makes ---------------------------------------------------------- */

static const uint32_t *g_shown;
static int g_shownW, g_shownH;

static int FakeOpen(void *window) { (void)window; return 0; }
static void FakeClose(void) {}
static void FakeShow(const uint32_t *rgba, int width, int height)
{
    g_shown = rgba;
    g_shownW = width;
    g_shownH = height;
}
static const HgvkPictureFns g_fake = {FakeOpen, FakeClose, FakeShow};
static const HgvkPictureFns *FakeApi(void) { return &g_fake; }

void *Plat_LoadLibrary(const char *name) { (void)name; return (void *)1; }
void *Plat_LibraryFunction(void *library, const char *name) { (void)library, (void)name; return (void *)FakeApi; }
int Plat_MessageBox(void *window, const char *text, const char *caption, uint32_t type)
{
    (void)window, (void)caption, (void)type;
    printf("message box: %s\n", text);
    return 1;
}

/* ---- the check ------------------------------------------------------------------------------------------------- */

static unsigned g_seed = 7;
static unsigned rnd(void) { g_seed = g_seed * 1103515245u + 12345u; return g_seed >> 8; }
static int rndIn(int lo, int hi) { return lo + (int)(rnd() % (unsigned)(hi - lo + 1)); }

int main(void)
{
    static const int kSizes[][2] = {{640, 480}, {320, 200}, {100, 37}, {36, 64}};
    long calls = 0, bad = 0;
    int s;

    for (s = 0; s < 4; s++) {
        int W = kSizes[s][0], H = kSizes[s][1], round, i;
        struct {
            BITMAPINFOHEADER h;
            RGBQUAD colors[256];
        } src;
        BITMAPINFO dst;
        uint8_t *pixels = (uint8_t *)malloc((size_t)W * H), rgb6[768];
        uint32_t *gdiBits;
        HDC dc = CreateCompatibleDC(NULL);
        HBITMAP bmp;
        void *bits;

        memset(&dst, 0, sizeof(dst));
        dst.bmiHeader.biSize = sizeof(dst.bmiHeader);
        dst.bmiHeader.biWidth = W;
        dst.bmiHeader.biHeight = -H;
        dst.bmiHeader.biPlanes = 1;
        dst.bmiHeader.biBitCount = 32;
        bmp = CreateDIBSection(dc, &dst, DIB_RGB_COLORS, &bits, NULL, 0);
        SelectObject(dc, bmp);
        gdiBits = (uint32_t *)bits;
        memset(gdiBits, 0, (size_t)W * H * 4);

        memset(&src, 0, sizeof(src));
        src.h.biSize = sizeof(src.h);
        src.h.biWidth = W;
        src.h.biHeight = -H;          /* top-down, as Dib_InitHeader makes it */
        src.h.biPlanes = 1;
        src.h.biBitCount = 8;
        src.h.biClrUsed = 256;

        Pic_Open((void *)1);         /* a window: any nonzero handle for the stand-in */
        for (round = 0; round < 60; round++) {
            int kind = round % 4, x0, y0, x1, y1, n;

            /* a new picture and palette now and then, as the game draws and fades */
            if (round % 3 == 0) {
                for (i = 0; i < 768; i++)
                    rgb6[i] = (uint8_t)(rnd() & 63);
                for (i = 0; i < 256; i++) {
                    src.colors[i].rgbRed = (BYTE)(rgb6[i * 3] << 2);
                    src.colors[i].rgbGreen = (BYTE)(rgb6[i * 3 + 1] << 2);
                    src.colors[i].rgbBlue = (BYTE)(rgb6[i * 3 + 2] << 2);
                }
                Pic_SetPalette(rgb6);
            }
            for (i = 0; i < W * H; i++)
                pixels[i] = (uint8_t)rnd();
            x0 = rndIn(0, W - 1), x1 = rndIn(x0, W - 1);
            y0 = rndIn(0, H - 1), y1 = rndIn(y0, H - 1);
            if (kind == 0) {            /* Gdi_BitBltRect(a, b, c, d) */
                n = SetDIBitsToDevice(dc, x0, y0, (DWORD)(x1 - x0 + 1), (DWORD)(y1 - y0 + 1), x0, H - y1 - 1, 0,
                                      (UINT)H, pixels, (BITMAPINFO *)&src, DIB_RGB_COLORS);
                Pic_Blit(pixels, W, H, x0, y0, x0, y0, x1 - x0 + 1, y1 - y0 + 1);
            } else if (kind == 1) {     /* Sim_10083c45: the whole picture */
                n = SetDIBitsToDevice(dc, 0, 0, (DWORD)W, (DWORD)H, 0, 0, 0, (UINT)H, pixels,
                                      (BITMAPINFO *)&src, DIB_RGB_COLORS);
                Pic_Blit(pixels, W, H, 0, 0, 0, 0, W, H);
            } else if (kind == 2) {     /* W11_Blit(y, start): the rolling picture, both of its calls */
                int start = rndIn(0, H - 1);

                n = SetDIBitsToDevice(dc, 0, 0, (DWORD)W, (DWORD)H, 0, 0, (UINT)start, (UINT)H, pixels,
                                      (BITMAPINFO *)&src, DIB_RGB_COLORS);
                Pic_Blit(pixels, W, H, 0, 0, 0, start, W, H);
                n = SetDIBitsToDevice(dc, 0, 0x1ea - start, (DWORD)W, (DWORD)H, 0, 0, 0, (UINT)H, pixels,
                                      (BITMAPINFO *)&src, DIB_RGB_COLORS);
                Pic_Blit(pixels, W, H, 0, 0x1ea - start, 0, 0, W, H);
            } else {                    /* Sim_10083de1: a part stretched over the whole picture */
                n = StretchDIBits(dc, 0, 0, W, H, x0, H - y1 - 1, x1 - x0 + 1, y1 - y0 + 1, pixels,
                                  (BITMAPINFO *)&src, DIB_RGB_COLORS, SRCCOPY);
                Pic_Stretch(pixels, W, H, x0, y0, x1 - x0 + 1, y1 - y0 + 1);
            }
            GdiFlush();
            calls++;
            {
                long diff = 0;
                int first = -1;

                for (i = 0; i < W * H; i++) {
                    uint32_t g = gdiBits[i], p = g_shown != NULL ? g_shown[i] : 0;
                    uint32_t gRgb = (g >> 16 & 0xff) | (g & 0xff00) | (g & 0xff) << 16;   /* BGRA to RGBA */

                    if (gRgb != (p & 0xffffff)) {
                        if (first < 0 && bad < 2)
                            printf("  pixel %d: GDI %08x ours %08x (index %u)\n", i, g, p, pixels[i]);
                        if (first < 0)
                            first = i;
                        diff++;
                    }
                }
                if (diff != 0 || g_shownW != W || g_shownH != H) {
                    if (bad++ < 12)
                        printf("%dx%d round %d kind %d (x %d..%d, y %d..%d, GDI answered %d): %ld pixels differ, "
                               "first at %d,%d\n", W, H, round, kind, x0, x1, y0, y1, n, diff,
                               first >= 0 ? first % W : -1, first >= 0 ? first / W : -1);
                }
            }
        }
        Pic_Close();
        g_shown = NULL;
        DeleteDC(dc);
        DeleteObject(bmp);
        free(pixels);
    }
    printf("%ld drawing calls, %ld with a different picture\n", calls, bad);
    return bad != 0;
}
