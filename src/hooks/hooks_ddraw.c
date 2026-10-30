/*
 * The menus' screen through SDL3 (platform/display.h). The hgrc.cfg word
 * "sdldisplay" turns it on.
 *
 * HGSHELL.DLL draws every menu screen through DirectDraw: a 640x480 picture in
 * 256 colours (or 16-bit colour) with a palette, a double-buffered flip chain
 * and a few blits. It runs on modern Windows only inside the "256Color"
 * compatibility layer. Here DirectDrawCreate and DirectDrawEnumerateA in the
 * DLL's import table are pointed at stand-ins. The stand-in objects have the
 * method tables of the real ones, keep the surfaces in ordinary memory and,
 * whenever the front surface changes, turn it into a 32-bit picture and hand it
 * to display.h. The menu code and art are unchanged.
 *
 * The methods the shell uses are done in full; the rest answer "not
 * implemented". Overlays, clippers, GDI access and hardware queries are not
 * needed by the shell.
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <windows.h>

#include "../launcher/debug.h"
#include "../platform/display.h"
#include "hook.h"

#define DD_OK 0
#define DDERR_UNSUPPORTED_ ((HRESULT)0x80004001)
#define DDERR_INVALIDPARAMS_ ((HRESULT)0x80070057)
#define DDERR_OUTOFMEMORY_ ((HRESULT)0x8007000E)
#define DDERR_NOTFOUND_ ((HRESULT)0x887600FE)
#define DDERR_SURFACELOST_ ((HRESULT)0x887601C2)
#define DDERR_NOEXCLUSIVEMODE_ ((HRESULT)0x88760213)

#define DDSD_CAPS 0x1
#define DDSD_HEIGHT 0x2
#define DDSD_WIDTH 0x4
#define DDSD_PITCH 0x8
#define DDSD_BACKBUFFERCOUNT 0x20
#define DDSD_LPSURFACE 0x800
#define DDSD_PIXELFORMAT 0x1000

#define DDSCAPS_BACKBUFFER 0x4
#define DDSCAPS_FLIP 0x10
#define DDSCAPS_PRIMARYSURFACE 0x200

#define DDBLT_COLORFILL 0x400
#define DDBLT_KEYSRC 0x8000
#define DDBLTFAST_SRCCOLORKEY 0x1
#define DDCKEY_SRCBLT 0x8
#define DDPF_PALETTEINDEXED8 0x20
#define DDPF_RGB 0x40
#define DDSCL_FULLSCREEN 0x1

#define DDSD_SIZE 0x6C
#define DDPF_SIZE 0x20

/* DDSURFACEDESC, by dword index. */
enum {
    SD_SIZE = 0, SD_FLAGS = 1, SD_HEIGHT = 2, SD_WIDTH = 3, SD_PITCH = 4, SD_BACKBUFFERS = 5,
    SD_SURFACE = 9, SD_PF = 18, /* 8 dwords: size, flags, bits, 4 masks, (unused) */
    SD_CAPS = 26
};
enum { PF_SIZE = 0, PF_FLAGS = 1, PF_BITS = 3, PF_R = 4, PF_G = 5, PF_B = 6 };

typedef struct DdPalette {
    const void *const *vtbl;
    LONG refs;
    uint32_t argb[256];
} DdPalette;

typedef struct DdSurface DdSurface;
struct DdSurface {
    const void *const *vtbl;
    LONG refs;
    int width, height, bpp, pitch;
    uint8_t *bits; /* what the surface shows now; swapped around by Flip */
    uint8_t *own;  /* the allocation */
    int primary;
    int nbacks;
    DdSurface *backs[3];
    DdPalette *palette;
    int locked;
    int hasKey;
    uint32_t keyLow;
};

typedef struct DdDevice {
    const void *const *vtbl;
    LONG refs;
    HWND window;
    int fullscreen;
    int modeW, modeH, modeBpp;
} DdDevice;

static DdDevice g_device;
static DdSurface *g_front; /* the primary surface, whose contents are on screen */
static uint32_t *g_argb;
static int g_argbSize;
static unsigned g_flips, g_blits, g_palettes, g_locks;
static LARGE_INTEGER g_lastFlip, g_freq;

/* The window as the launcher made it, to give back at the end. */
static WINDOWPLACEMENT g_savedPlacement;
static LONG_PTR g_savedStyle;
static int g_windowSaved;

#define M(ret) static ret __stdcall

/* ---- window ---- */

/*
 * The mode is a picture size now, not a monitor mode: the window covers the
 * monitor and the picture is scaled to fit it (display.h).
 */
static void PlaceWindow(HWND window, int w, int h)
{
    int sx, sy;

    if (window == NULL)
        return;
    if (!g_windowSaved) {
        g_savedPlacement.length = sizeof(g_savedPlacement);
        GetWindowPlacement(window, &g_savedPlacement);
        g_savedStyle = GetWindowLongPtrA(window, GWL_STYLE);
        g_windowSaved = 1;
    }
    (void)w;
    (void)h;
    sx = GetSystemMetrics(SM_CXSCREEN);
    sy = GetSystemMetrics(SM_CYSCREEN);
    ShowWindow(window, SW_SHOWNORMAL);
    SetWindowLongPtrA(window, GWL_STYLE, WS_POPUP | WS_VISIBLE);
    SetWindowPos(window, HWND_TOP, 0, 0, sx, sy, SWP_FRAMECHANGED | SWP_SHOWWINDOW);
}

static void RestoreWindow(HWND window)
{
    if (window == NULL || !g_windowSaved)
        return;
    SetWindowLongPtrA(window, GWL_STYLE, g_savedStyle);
    SetWindowPlacement(window, &g_savedPlacement);
    SetWindowPos(window, NULL, 0, 0, 0, 0,
                 SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
    g_windowSaved = 0;
}

/* ---- showing the front surface ---- */

static void Present(void)
{
    DdSurface *s = g_front;
    uint32_t grey[256];
    const uint32_t *pal;
    int n, i;

    if (s == NULL || !Display_IsOpen())
        return;
    n = s->width * s->height;
    if (g_argbSize < n) {
        free(g_argb);
        g_argb = malloc((size_t)n * 4);
        g_argbSize = g_argb != NULL ? n : 0;
        if (g_argb == NULL)
            return;
    }
    if (s->bpp == 8) {
        if (s->palette != NULL) {
            pal = s->palette->argb;
        } else {
            for (i = 0; i < 256; i++)
                grey[i] = 0xFF000000u | (uint32_t)i * 0x010101u;
            pal = grey;
        }
        for (i = 0; i < s->height; i++) {
            const uint8_t *src = s->bits + (size_t)i * s->pitch;
            uint32_t *dst = g_argb + (size_t)i * s->width;
            int x;

            for (x = 0; x < s->width; x++)
                dst[x] = pal[src[x]];
        }
    } else if (s->bpp == 16) {
        for (i = 0; i < s->height; i++) {
            const uint16_t *src = (const uint16_t *)(s->bits + (size_t)i * s->pitch);
            uint32_t *dst = g_argb + (size_t)i * s->width;
            int x;

            for (x = 0; x < s->width; x++) { /* RGB565 */
                uint32_t p = src[x];
                uint32_t r = (p >> 11) & 31, g = (p >> 5) & 63, b = p & 31;

                dst[x] = 0xFF000000u | (r << 3 | r >> 2) << 16 | (g << 2 | g >> 4) << 8 |
                         (b << 3 | b >> 2);
            }
        }
    } else {
        for (i = 0; i < s->height; i++)
            memcpy(g_argb + (size_t)i * s->width, s->bits + (size_t)i * s->pitch,
                   (size_t)s->width * 4);
    }
    Display_Present(g_argb);
}

/* A flip waited for the monitor's refresh; the menus' animation may rely on it. */
static void PaceFlip(void)
{
    LARGE_INTEGER now;
    LONGLONG frame;

    if (g_freq.QuadPart == 0)
        QueryPerformanceFrequency(&g_freq);
    frame = g_freq.QuadPart / 60;
    QueryPerformanceCounter(&now);
    while (g_lastFlip.QuadPart != 0 && now.QuadPart - g_lastFlip.QuadPart < frame) {
        if (frame - (now.QuadPart - g_lastFlip.QuadPart) > g_freq.QuadPart / 500)
            Sleep(1);
        QueryPerformanceCounter(&now);
    }
    g_lastFlip = now;
}

/* ---- IDirectDrawPalette ---- */

M(HRESULT) PalQueryInterface(DdPalette *p, const GUID *iid, void **out)
{
    (void)p; (void)iid;
    if (out != NULL)
        *out = NULL;
    return DDERR_UNSUPPORTED_;
}
M(ULONG) PalAddRef(DdPalette *p) { return (ULONG)InterlockedIncrement(&p->refs); }
M(ULONG) PalRelease(DdPalette *p)
{
    LONG refs = InterlockedDecrement(&p->refs);

    if (refs == 0)
        free(p);
    return (ULONG)refs;
}
M(HRESULT) PalGetCaps(DdPalette *p, DWORD *caps)
{
    (void)p;
    *caps = 0x44;
    return DD_OK;
}
M(HRESULT) PalGetEntries(DdPalette *p, DWORD flags, DWORD start, DWORD count, uint8_t *out)
{
    DWORD i;

    (void)flags;
    if (start > 256 || count > 256 - start || out == NULL)
        return DDERR_INVALIDPARAMS_;
    for (i = 0; i < count; i++) {
        uint32_t c = p->argb[start + i];

        out[i * 4] = (uint8_t)(c >> 16);
        out[i * 4 + 1] = (uint8_t)(c >> 8);
        out[i * 4 + 2] = (uint8_t)c;
        out[i * 4 + 3] = 0;
    }
    return DD_OK;
}
M(HRESULT) PalInitialize(DdPalette *p, void *dd, DWORD flags, const void *entries)
{
    (void)p; (void)dd; (void)flags; (void)entries;
    return DDERR_UNSUPPORTED_;
}
M(HRESULT) PalSetEntries(DdPalette *p, DWORD flags, DWORD start, DWORD count, const uint8_t *in)
{
    DWORD i;

    (void)flags;
    if (start > 256 || count > 256 - start || in == NULL)
        return DDERR_INVALIDPARAMS_;
    for (i = 0; i < count; i++)
        p->argb[start + i] = 0xFF000000u | (uint32_t)in[i * 4] << 16 |
                             (uint32_t)in[i * 4 + 1] << 8 | in[i * 4 + 2];
    g_palettes++;
    if (g_front != NULL && g_front->palette == p) /* a palette change shows at once */
        Present();
    return DD_OK;
}

static const void *const g_palVtbl[] = {
    (void *)PalQueryInterface, (void *)PalAddRef,     (void *)PalRelease,
    (void *)PalGetCaps,        (void *)PalGetEntries, (void *)PalInitialize,
    (void *)PalSetEntries,
};

/* ---- IDirectDrawSurface ---- */

static void FillPixelFormat(const DdSurface *s, uint32_t *pf)
{
    memset(pf, 0, DDPF_SIZE);
    pf[PF_SIZE] = DDPF_SIZE;
    pf[PF_BITS] = (uint32_t)s->bpp;
    if (s->bpp == 8) {
        pf[PF_FLAGS] = DDPF_PALETTEINDEXED8 | DDPF_RGB;
    } else if (s->bpp == 16) {
        pf[PF_FLAGS] = DDPF_RGB;
        pf[PF_R] = 0xF800;
        pf[PF_G] = 0x07E0;
        pf[PF_B] = 0x001F;
    } else {
        pf[PF_FLAGS] = DDPF_RGB;
        pf[PF_R] = 0xFF0000;
        pf[PF_G] = 0x00FF00;
        pf[PF_B] = 0x0000FF;
    }
}

static void FillDesc(const DdSurface *s, uint32_t *d, int withPointer)
{
    d[SD_FLAGS] = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PITCH | DDSD_PIXELFORMAT |
                  (withPointer ? DDSD_LPSURFACE : 0);
    d[SD_HEIGHT] = (uint32_t)s->height;
    d[SD_WIDTH] = (uint32_t)s->width;
    d[SD_PITCH] = (uint32_t)s->pitch;
    d[SD_SURFACE] = withPointer ? (uint32_t)(UINT_PTR)s->bits : 0;
    FillPixelFormat(s, d + SD_PF);
    d[SD_CAPS] = s->primary ? DDSCAPS_PRIMARYSURFACE : 0;
}

M(HRESULT) SurfQueryInterface(DdSurface *s, const GUID *iid, void **out)
{
    (void)s; (void)iid;
    if (out != NULL)
        *out = NULL;
    return DDERR_UNSUPPORTED_;
}
M(ULONG) SurfAddRef(DdSurface *s) { return (ULONG)InterlockedIncrement(&s->refs); }
M(ULONG) SurfRelease(DdSurface *s)
{
    LONG refs = InterlockedDecrement(&s->refs);
    int i;

    if (refs == 0) {
        if (s == g_front)
            g_front = NULL;
        for (i = 0; i < s->nbacks; i++)
            SurfRelease(s->backs[i]);
        if (s->palette != NULL)
            PalRelease(s->palette);
        free(s->own);
        free(s);
    }
    return (ULONG)refs;
}

/* Clips a rectangle to the surface; returns 0 when nothing is left. */
static int ClipRect(const DdSurface *s, const RECT *in, RECT *out)
{
    out->left = in != NULL ? in->left : 0;
    out->top = in != NULL ? in->top : 0;
    out->right = in != NULL ? in->right : s->width;
    out->bottom = in != NULL ? in->bottom : s->height;
    if (out->left < 0) out->left = 0;
    if (out->top < 0) out->top = 0;
    if (out->right > s->width) out->right = s->width;
    if (out->bottom > s->height) out->bottom = s->height;
    return out->right > out->left && out->bottom > out->top;
}

static uint32_t ReadPixel(const uint8_t *p, int bpp)
{
    return bpp == 8 ? p[0] : bpp == 16 ? (uint32_t)(p[0] | p[1] << 8)
                                       : (uint32_t)(p[0] | p[1] << 8 | p[2] << 16 | p[3] << 24);
}

static void WritePixel(uint8_t *p, int bpp, uint32_t v)
{
    p[0] = (uint8_t)v;
    if (bpp >= 16)
        p[1] = (uint8_t)(v >> 8);
    if (bpp == 32) {
        p[2] = (uint8_t)(v >> 16);
        p[3] = (uint8_t)(v >> 24);
    }
}

/* Copies (and, if the sizes differ, stretches) a rectangle; optional source colour key. */
static void CopyPixels(DdSurface *dst, const RECT *dr, DdSurface *src, const RECT *sr, int useKey)
{
    int dw = dr->right - dr->left, dh = dr->bottom - dr->top;
    int sw = sr->right - sr->left, sh = sr->bottom - sr->top;
    int bytes = dst->bpp / 8, x, y;

    if (dw == sw && dh == sh && !useKey) {
        if (dst == src && dr->top > sr->top) {
            for (y = dh - 1; y >= 0; y--)
                memmove(dst->bits + (size_t)(dr->top + y) * dst->pitch + dr->left * bytes,
                        src->bits + (size_t)(sr->top + y) * src->pitch + sr->left * bytes,
                        (size_t)dw * bytes);
        } else {
            for (y = 0; y < dh; y++)
                memmove(dst->bits + (size_t)(dr->top + y) * dst->pitch + dr->left * bytes,
                        src->bits + (size_t)(sr->top + y) * src->pitch + sr->left * bytes,
                        (size_t)dw * bytes);
        }
        return;
    }
    for (y = 0; y < dh; y++) {
        const uint8_t *sp = src->bits + (size_t)(sr->top + y * sh / dh) * src->pitch;
        uint8_t *dp = dst->bits + (size_t)(dr->top + y) * dst->pitch + dr->left * bytes;

        for (x = 0; x < dw; x++) {
            const uint8_t *px = sp + (sr->left + x * sw / dw) * bytes;

            if (useKey && ReadPixel(px, src->bpp) == src->keyLow)
                continue;
            memcpy(dp + x * bytes, px, (size_t)bytes);
        }
    }
}

M(HRESULT) SurfBlt(DdSurface *d, const RECT *destRect, DdSurface *src, const RECT *srcRect,
                   DWORD flags, const uint32_t *fx)
{
    RECT dr, sr;

    if (!ClipRect(d, destRect, &dr))
        return DD_OK;
    g_blits++;
    if (flags & DDBLT_COLORFILL) {
        uint32_t color = fx != NULL ? fx[20] : 0; /* DDBLTFX.dwFillColor */
        int x, y, bytes = d->bpp / 8;

        for (y = dr.top; y < dr.bottom; y++)
            for (x = dr.left; x < dr.right; x++)
                WritePixel(d->bits + (size_t)y * d->pitch + x * bytes, d->bpp, color);
    } else {
        if (src == NULL || src->bpp != d->bpp || !ClipRect(src, srcRect, &sr))
            return src == NULL ? DDERR_INVALIDPARAMS_ : DD_OK;
        CopyPixels(d, &dr, src, &sr, (flags & DDBLT_KEYSRC) && src->hasKey);
    }
    if (d == g_front)
        Present();
    return DD_OK;
}

M(HRESULT) SurfBltFast(DdSurface *d, DWORD x, DWORD y, DdSurface *src, const RECT *srcRect,
                       DWORD trans)
{
    RECT sr, dr;

    if (src == NULL || src->bpp != d->bpp || !ClipRect(src, srcRect, &sr))
        return src == NULL ? DDERR_INVALIDPARAMS_ : DD_OK;
    dr.left = (LONG)x;
    dr.top = (LONG)y;
    dr.right = dr.left + (sr.right - sr.left);
    dr.bottom = dr.top + (sr.bottom - sr.top);
    if (dr.right > d->width) { sr.right -= dr.right - d->width; dr.right = d->width; }
    if (dr.bottom > d->height) { sr.bottom -= dr.bottom - d->height; dr.bottom = d->height; }
    if (dr.right <= dr.left || dr.bottom <= dr.top)
        return DD_OK;
    g_blits++;
    CopyPixels(d, &dr, src, &sr, (trans & DDBLTFAST_SRCCOLORKEY) && src->hasKey);
    if (d == g_front)
        Present();
    return DD_OK;
}

M(HRESULT) SurfFlip(DdSurface *s, DdSurface *target, DWORD flags)
{
    uint8_t *first;
    int i;

    (void)target;
    (void)flags;
    if (!s->primary || s->nbacks == 0)
        return DDERR_INVALIDPARAMS_;
    first = s->bits; /* the back buffer becomes the front, the old front goes to the end */
    s->bits = s->backs[0]->bits;
    for (i = 0; i + 1 < s->nbacks; i++)
        s->backs[i]->bits = s->backs[i + 1]->bits;
    s->backs[s->nbacks - 1]->bits = first;
    g_flips++;
    Present();
    PaceFlip();
    return DD_OK;
}

M(HRESULT) SurfGetAttachedSurface(DdSurface *s, const uint32_t *caps, DdSurface **out)
{
    DdSurface *found = NULL;

    if (out == NULL || caps == NULL)
        return DDERR_INVALIDPARAMS_;
    *out = NULL;
    if (s->nbacks > 0 && (caps[0] & (DDSCAPS_BACKBUFFER | DDSCAPS_FLIP)))
        found = s->backs[0];
    if (found == NULL)
        return DDERR_NOTFOUND_;
    InterlockedIncrement(&found->refs);
    *out = found;
    return DD_OK;
}

M(HRESULT) SurfGetPixelFormat(DdSurface *s, uint32_t *pf)
{
    FillPixelFormat(s, pf);
    return DD_OK;
}

M(HRESULT) SurfGetSurfaceDesc(DdSurface *s, uint32_t *desc)
{
    uint32_t size = desc[SD_SIZE];

    if (size < DDSD_SIZE)
        return DDERR_INVALIDPARAMS_;
    memset(desc, 0, size);
    desc[SD_SIZE] = size;
    FillDesc(s, desc, 0);
    return DD_OK;
}

M(HRESULT) SurfIsLost(DdSurface *s)
{
    (void)s;
    return DD_OK;
}

M(HRESULT) SurfLock(DdSurface *s, const RECT *rect, uint32_t *desc, DWORD flags, HANDLE event)
{
    uint32_t size = desc[SD_SIZE];

    (void)flags;
    (void)event;
    if (size < DDSD_SIZE)
        return DDERR_INVALIDPARAMS_;
    memset(desc, 0, size);
    desc[SD_SIZE] = size;
    FillDesc(s, desc, 1);
    if (rect != NULL)
        desc[SD_SURFACE] = (uint32_t)(UINT_PTR)(s->bits + (size_t)rect->top * s->pitch +
                                                (size_t)rect->left * (s->bpp / 8));
    s->locked = 1;
    g_locks++;
    return DD_OK;
}

M(HRESULT) SurfRestore(DdSurface *s)
{
    (void)s;
    return DD_OK;
}

M(HRESULT) SurfSetPalette(DdSurface *s, DdPalette *pal)
{
    if (pal != NULL)
        InterlockedIncrement(&pal->refs);
    if (s->palette != NULL)
        PalRelease(s->palette);
    s->palette = pal;
    if (s == g_front)
        Present();
    return DD_OK;
}

M(HRESULT) SurfUnlock(DdSurface *s, void *rect)
{
    (void)rect;
    s->locked = 0;
    if (s == g_front)
        Present();
    return DD_OK;
}

M(HRESULT) SurfSetColorKey(DdSurface *s, DWORD flags, const uint32_t *key)
{
    if ((flags & DDCKEY_SRCBLT) && key != NULL) {
        s->hasKey = 1;
        s->keyLow = key[0];
    }
    return DD_OK;
}

M(HRESULT) SurfGetColorKey(DdSurface *s, DWORD flags, uint32_t *key)
{
    if (!(flags & DDCKEY_SRCBLT) || !s->hasKey || key == NULL)
        return DDERR_UNSUPPORTED_;
    key[0] = key[1] = s->keyLow;
    return DD_OK;
}

M(HRESULT) SurfGetPalette(DdSurface *s, DdPalette **out)
{
    if (out == NULL || s->palette == NULL)
        return DDERR_NOTFOUND_;
    InterlockedIncrement(&s->palette->refs);
    *out = s->palette;
    return DD_OK;
}

M(HRESULT) SurfGetFlipStatus(DdSurface *s, DWORD flags) { (void)s; (void)flags; return DD_OK; }
M(HRESULT) SurfGetBltStatus(DdSurface *s, DWORD flags) { (void)s; (void)flags; return DD_OK; }

/* Not needed: each takes the argument count of the real method (stdcall). */
M(HRESULT) SurfAddAttachedSurface(DdSurface *s, void *a) { (void)s; (void)a; return DDERR_UNSUPPORTED_; }
M(HRESULT) SurfAddOverlayDirtyRect(DdSurface *s, void *a) { (void)s; (void)a; return DDERR_UNSUPPORTED_; }
M(HRESULT) SurfBltBatch(DdSurface *s, void *a, DWORD b, DWORD c) { (void)s; (void)a; (void)b; (void)c; return DDERR_UNSUPPORTED_; }
M(HRESULT) SurfDeleteAttachedSurface(DdSurface *s, DWORD a, void *b) { (void)s; (void)a; (void)b; return DDERR_UNSUPPORTED_; }
M(HRESULT) SurfEnumAttachedSurfaces(DdSurface *s, void *a, void *b) { (void)s; (void)a; (void)b; return DDERR_UNSUPPORTED_; }
M(HRESULT) SurfEnumOverlayZOrders(DdSurface *s, DWORD a, void *b, void *c) { (void)s; (void)a; (void)b; (void)c; return DDERR_UNSUPPORTED_; }
M(HRESULT) SurfGetCaps(DdSurface *s, uint32_t *caps)
{
    caps[0] = s->primary ? DDSCAPS_PRIMARYSURFACE : 0;
    return DD_OK;
}
M(HRESULT) SurfGetClipper(DdSurface *s, void **a) { (void)s; (void)a; return DDERR_UNSUPPORTED_; }
M(HRESULT) SurfGetDC(DdSurface *s, void **a) { (void)s; (void)a; return DDERR_UNSUPPORTED_; }
M(HRESULT) SurfGetOverlayPosition(DdSurface *s, LONG *a, LONG *b) { (void)s; (void)a; (void)b; return DDERR_UNSUPPORTED_; }
M(HRESULT) SurfInitialize(DdSurface *s, void *a, void *b) { (void)s; (void)a; (void)b; return DDERR_UNSUPPORTED_; }
M(HRESULT) SurfReleaseDC(DdSurface *s, void *a) { (void)s; (void)a; return DDERR_UNSUPPORTED_; }
M(HRESULT) SurfSetClipper(DdSurface *s, void *a) { (void)s; (void)a; return DDERR_UNSUPPORTED_; }
M(HRESULT) SurfSetOverlayPosition(DdSurface *s, LONG a, LONG b) { (void)s; (void)a; (void)b; return DDERR_UNSUPPORTED_; }
M(HRESULT) SurfUpdateOverlay(DdSurface *s, void *a, void *b, void *c, DWORD d, void *e) { (void)s; (void)a; (void)b; (void)c; (void)d; (void)e; return DDERR_UNSUPPORTED_; }
M(HRESULT) SurfUpdateOverlayDisplay(DdSurface *s, DWORD a) { (void)s; (void)a; return DDERR_UNSUPPORTED_; }
M(HRESULT) SurfUpdateOverlayZOrder(DdSurface *s, DWORD a, void *b) { (void)s; (void)a; (void)b; return DDERR_UNSUPPORTED_; }

static const void *const g_surfVtbl[] = {
    (void *)SurfQueryInterface, (void *)SurfAddRef, (void *)SurfRelease,
    (void *)SurfAddAttachedSurface, (void *)SurfAddOverlayDirtyRect, (void *)SurfBlt,
    (void *)SurfBltBatch, (void *)SurfBltFast, (void *)SurfDeleteAttachedSurface,
    (void *)SurfEnumAttachedSurfaces, (void *)SurfEnumOverlayZOrders, (void *)SurfFlip,
    (void *)SurfGetAttachedSurface, (void *)SurfGetBltStatus, (void *)SurfGetCaps,
    (void *)SurfGetClipper, (void *)SurfGetColorKey, (void *)SurfGetDC,
    (void *)SurfGetFlipStatus, (void *)SurfGetOverlayPosition, (void *)SurfGetPalette,
    (void *)SurfGetPixelFormat, (void *)SurfGetSurfaceDesc, (void *)SurfInitialize,
    (void *)SurfIsLost, (void *)SurfLock, (void *)SurfReleaseDC, (void *)SurfRestore,
    (void *)SurfSetClipper, (void *)SurfSetColorKey, (void *)SurfSetOverlayPosition,
    (void *)SurfSetPalette, (void *)SurfUnlock, (void *)SurfUpdateOverlay,
    (void *)SurfUpdateOverlayDisplay, (void *)SurfUpdateOverlayZOrder,
};

static DdSurface *NewSurface(int w, int h, int bpp)
{
    DdSurface *s = calloc(1, sizeof(*s));

    if (s == NULL)
        return NULL;
    s->vtbl = g_surfVtbl;
    s->refs = 1;
    s->width = w;
    s->height = h;
    s->bpp = bpp;
    s->pitch = w * (bpp / 8);
    s->own = calloc((size_t)s->pitch * h + 16, 1);
    if (s->own == NULL) {
        free(s);
        return NULL;
    }
    s->bits = s->own;
    return s;
}

/* ---- IDirectDraw ---- */

M(HRESULT) DevQueryInterface(DdDevice *d, const GUID *iid, void **out)
{
    (void)d; (void)iid;
    if (out != NULL)
        *out = NULL;
    return DDERR_UNSUPPORTED_;
}
M(ULONG) DevAddRef(DdDevice *d) { return (ULONG)InterlockedIncrement(&d->refs); }
M(ULONG) DevRelease(DdDevice *d)
{
    LONG refs = InterlockedDecrement(&d->refs);

    if (refs == 0) {
        if (Display_IsOpen()) {
            DebugPrintf("DirectDraw stand-in closed (flips %u, blits %u, palette writes %u, "
                        "locks %u)\n", g_flips, g_blits, g_palettes, g_locks);
            Display_Close();
        }
        RestoreWindow(d->window);
        g_front = NULL;
    }
    return (ULONG)refs;
}

M(HRESULT) DevCreatePalette(DdDevice *d, DWORD flags, const uint8_t *entries, DdPalette **out,
                            void *outer)
{
    DdPalette *p;
    int i;

    (void)d; (void)outer;
    if (out == NULL || (flags & 0x4) == 0) /* only 8-bit palettes */
        return DDERR_INVALIDPARAMS_;
    p = calloc(1, sizeof(*p));
    if (p == NULL)
        return DDERR_OUTOFMEMORY_;
    p->vtbl = g_palVtbl;
    p->refs = 1;
    for (i = 0; i < 256; i++)
        p->argb[i] = entries != NULL ? 0xFF000000u | (uint32_t)entries[i * 4] << 16 |
                                           (uint32_t)entries[i * 4 + 1] << 8 | entries[i * 4 + 2]
                                     : 0xFF000000u;
    *out = p;
    return DD_OK;
}

M(HRESULT) DevCreateSurface(DdDevice *d, const uint32_t *desc, DdSurface **out, void *outer)
{
    uint32_t flags, caps;
    DdSurface *s;
    int w, h, bpp, i, backs;

    (void)outer;
    if (desc == NULL || out == NULL || desc[SD_SIZE] < DDSD_SIZE)
        return DDERR_INVALIDPARAMS_;
    *out = NULL;
    flags = desc[SD_FLAGS];
    caps = desc[SD_CAPS];
    if (caps & DDSCAPS_PRIMARYSURFACE) {
        if (d->modeW == 0)
            return DDERR_NOEXCLUSIVEMODE_;
        backs = (flags & DDSD_BACKBUFFERCOUNT) ? (int)desc[SD_BACKBUFFERS] : 0;
        if (backs < 0 || backs > 3 || g_front != NULL)
            return DDERR_INVALIDPARAMS_;
        s = NewSurface(d->modeW, d->modeH, d->modeBpp);
        if (s == NULL)
            return DDERR_OUTOFMEMORY_;
        s->primary = 1;
        for (i = 0; i < backs; i++) {
            s->backs[i] = NewSurface(d->modeW, d->modeH, d->modeBpp);
            if (s->backs[i] == NULL) {
                SurfRelease(s);
                return DDERR_OUTOFMEMORY_;
            }
            s->nbacks++;
        }
        g_front = s;
    } else {
        if (!(flags & DDSD_WIDTH) || !(flags & DDSD_HEIGHT) || desc[SD_WIDTH] == 0 ||
            desc[SD_HEIGHT] == 0 || desc[SD_WIDTH] > 4096 || desc[SD_HEIGHT] > 4096)
            return DDERR_INVALIDPARAMS_;
        w = (int)desc[SD_WIDTH];
        h = (int)desc[SD_HEIGHT];
        bpp = (flags & DDSD_PIXELFORMAT) ? (int)desc[SD_PF + PF_BITS] : d->modeBpp;
        if (bpp != 8 && bpp != 16 && bpp != 32)
            return DDERR_INVALIDPARAMS_;
        s = NewSurface(w, h, bpp);
        if (s == NULL)
            return DDERR_OUTOFMEMORY_;
    }
    *out = s;
    return DD_OK;
}

M(HRESULT) DevGetCaps(DdDevice *d, uint32_t *drv, uint32_t *hel)
{
    (void)d;
    if (drv != NULL) {
        uint32_t size = drv[0];

        if (size >= 0x44) {
            memset(drv, 0, size);
            drv[0] = size;
            drv[1] = 0x40;           /* DDCAPS_BLT */
            drv[0x3C / 4] = 64u << 20; /* video memory, total and free */
            drv[0x40 / 4] = 64u << 20;
        }
    }
    if (hel != NULL) {
        uint32_t size = hel[0];

        if (size >= 8) {
            memset(hel, 0, size);
            hel[0] = size;
        }
    }
    return DD_OK;
}

M(HRESULT) DevGetDisplayMode(DdDevice *d, uint32_t *desc)
{
    uint32_t size = desc[SD_SIZE];

    if (size < DDSD_SIZE)
        return DDERR_INVALIDPARAMS_;
    memset(desc, 0, size);
    desc[SD_SIZE] = size;
    desc[SD_FLAGS] = DDSD_HEIGHT | DDSD_WIDTH | DDSD_PITCH | DDSD_PIXELFORMAT;
    desc[SD_HEIGHT] = (uint32_t)d->modeH;
    desc[SD_WIDTH] = (uint32_t)d->modeW;
    desc[SD_PITCH] = (uint32_t)(d->modeW * d->modeBpp / 8);
    desc[SD_PF + PF_SIZE] = DDPF_SIZE;
    desc[SD_PF + PF_BITS] = (uint32_t)d->modeBpp;
    desc[SD_PF + PF_FLAGS] = d->modeBpp == 8 ? (DDPF_PALETTEINDEXED8 | DDPF_RGB) : DDPF_RGB;
    return DD_OK;
}

M(HRESULT) DevRestoreDisplayMode(DdDevice *d) { (void)d; return DD_OK; }

M(HRESULT) DevSetCooperativeLevel(DdDevice *d, HWND window, DWORD level)
{
    d->window = window;
    d->fullscreen = (level & DDSCL_FULLSCREEN) != 0;
    return DD_OK;
}

M(HRESULT) DevSetDisplayMode(DdDevice *d, DWORD w, DWORD h, DWORD bpp)
{
    if (w == 0 || h == 0 || w > 4096 || h > 4096 || (bpp != 8 && bpp != 16 && bpp != 32))
        return DDERR_INVALIDPARAMS_;
    d->modeW = (int)w;
    d->modeH = (int)h;
    d->modeBpp = (int)bpp;
    PlaceWindow(d->window, (int)w, (int)h);
    if (Display_IsOpen())
        Display_Close();
    if (!Display_Open(d->window, (int)w, (int)h)) {
        DebugPrintf("DirectDraw stand-in: the display could not be opened\n");
        return DDERR_UNSUPPORTED_;
    }
    DebugPrintf("DirectDraw stand-in: mode %lux%lux%lu\n", (unsigned long)w, (unsigned long)h,
                (unsigned long)bpp);
    return DD_OK;
}

M(HRESULT) DevWaitForVerticalBlank(DdDevice *d, DWORD flags, HANDLE event)
{
    (void)d; (void)flags; (void)event;
    PaceFlip();
    return DD_OK;
}

M(HRESULT) DevCompact(DdDevice *d) { (void)d; return DD_OK; }
M(HRESULT) DevCreateClipper(DdDevice *d, DWORD a, void **b, void *c) { (void)d; (void)a; (void)b; (void)c; return DDERR_UNSUPPORTED_; }
M(HRESULT) DevDuplicateSurface(DdDevice *d, void *a, void **b) { (void)d; (void)a; (void)b; return DDERR_UNSUPPORTED_; }
M(HRESULT) DevEnumDisplayModes(DdDevice *d, DWORD a, void *b, void *c, void *e) { (void)d; (void)a; (void)b; (void)c; (void)e; return DDERR_UNSUPPORTED_; }
M(HRESULT) DevEnumSurfaces(DdDevice *d, DWORD a, void *b, void *c, void *e) { (void)d; (void)a; (void)b; (void)c; (void)e; return DDERR_UNSUPPORTED_; }
M(HRESULT) DevFlipToGDISurface(DdDevice *d) { (void)d; return DD_OK; }
M(HRESULT) DevGetFourCCCodes(DdDevice *d, DWORD *n, DWORD *codes) { (void)d; (void)codes; if (n != NULL) *n = 0; return DD_OK; }
M(HRESULT) DevGetGDISurface(DdDevice *d, void **a) { (void)d; (void)a; return DDERR_UNSUPPORTED_; }
M(HRESULT) DevGetMonitorFrequency(DdDevice *d, DWORD *hz) { (void)d; *hz = 60; return DD_OK; }
M(HRESULT) DevGetScanLine(DdDevice *d, DWORD *line) { (void)d; *line = 0; return DDERR_UNSUPPORTED_; }
M(HRESULT) DevGetVerticalBlankStatus(DdDevice *d, LONG *in) { (void)d; *in = 0; return DD_OK; }
M(HRESULT) DevInitialize(DdDevice *d, const GUID *guid) { (void)d; (void)guid; return DD_OK; }

static const void *const g_devVtbl[] = {
    (void *)DevQueryInterface, (void *)DevAddRef, (void *)DevRelease, (void *)DevCompact,
    (void *)DevCreateClipper, (void *)DevCreatePalette, (void *)DevCreateSurface,
    (void *)DevDuplicateSurface, (void *)DevEnumDisplayModes, (void *)DevEnumSurfaces,
    (void *)DevFlipToGDISurface, (void *)DevGetCaps, (void *)DevGetDisplayMode,
    (void *)DevGetFourCCCodes, (void *)DevGetGDISurface, (void *)DevGetMonitorFrequency,
    (void *)DevGetScanLine, (void *)DevGetVerticalBlankStatus, (void *)DevInitialize,
    (void *)DevRestoreDisplayMode, (void *)DevSetCooperativeLevel, (void *)DevSetDisplayMode,
    (void *)DevWaitForVerticalBlank,
};

/*
 * The shell's mouse code reads the cursor in window pixels and expects them to
 * be picture pixels (and the window rectangle to be the picture's, at 0,0). With
 * the picture scaled into a bigger window, its user32 calls are answered in
 * picture terms. Only HGSHELL's import table is changed.
 */
static unsigned g_mouseLogged, g_cursorSets;

static BOOL __stdcall StandInScreenToClient(HWND window, POINT *pt)
{
    POINT screen = *pt;
    BOOL ok = ScreenToClient(window, pt);
    POINT client = *pt;

    if (ok && Display_IsOpen())
        Display_ClientToPicture((int *)&pt->x, (int *)&pt->y);
    /* The first few and then one in a while, to see the mapping at work. */
    if (g_mouseLogged < 40 || g_mouseLogged % 50000 == 0)
        DebugPrintf("mouse: screen %ld,%ld -> client %ld,%ld -> picture %ld,%ld\n", screen.x,
                    screen.y, client.x, client.y, pt->x, pt->y);
    g_mouseLogged++;
    return ok;
}

static BOOL __stdcall StandInClientToScreen(HWND window, POINT *pt)
{
    POINT picture = *pt;

    if (Display_IsOpen())
        Display_PictureToClient((int *)&pt->x, (int *)&pt->y);
    if (g_cursorSets++ < 20)
        DebugPrintf("mouse: set cursor picture %ld,%ld -> client %ld,%ld\n", picture.x,
                    picture.y, pt->x, pt->y);
    return ClientToScreen(window, pt);
}

static BOOL __stdcall StandInGetWindowRect(HWND window, RECT *rect)
{
    if (!Display_IsOpen() || window != g_device.window)
        return GetWindowRect(window, rect);
    rect->left = 0;
    rect->top = 0;
    rect->right = g_device.modeW;
    rect->bottom = g_device.modeH;
    DebugPrintf("mouse: window rectangle answered as 0,0,%d,%d\n", g_device.modeW,
                g_device.modeH);
    return TRUE;
}

/*
 * The shell's window procedure also takes mouse messages (move, buttons) and
 * reads the position from lParam, in client pixels. Called by the launcher's
 * window procedure before it hands a message to the shell.
 */
LPARAM HooksDDraw_TranslateMouse(UINT msg, LPARAM lParam)
{
    int x, y;

    if (msg < WM_MOUSEMOVE || msg > WM_MBUTTONDBLCLK || !Display_IsOpen())
        return lParam;
    x = (short)LOWORD(lParam);
    y = (short)HIWORD(lParam);
    Display_ClientToPicture(&x, &y);
    if (x < -32768) x = -32768;
    if (x > 32767) x = 32767;
    if (y < -32768) y = -32768;
    if (y > 32767) y = 32767;
    return MAKELPARAM((WORD)(short)x, (WORD)(short)y);
}

/* ---- entry points that replace DDRAW.DLL's ---- */

static HRESULT __stdcall StandInDirectDrawCreate(const GUID *guid, DdDevice **out, void *outer)
{
    (void)guid;
    (void)outer;
    if (out == NULL)
        return DDERR_INVALIDPARAMS_;
    memset(&g_device, 0, sizeof(g_device));
    g_device.vtbl = g_devVtbl;
    g_device.refs = 1;
    *out = &g_device;
    return DD_OK;
}

/* No hardware adapters to offer: the shell then creates the default device. */
static HRESULT __stdcall StandInDirectDrawEnumerateA(void *callback, void *context)
{
    (void)callback;
    (void)context;
    return DD_OK;
}

static int ConfigHasWord(const char *wanted)
{
    FILE *fp = fopen("hgrc.cfg", "r");
    char word[32];
    int found = 0;

    while (fp != NULL && !found && fscanf(fp, "%31s", word) == 1)
        found = _stricmp(word, wanted) == 0;
    if (fp != NULL)
        fclose(fp);
    return found;
}

/* The shell has returned: whatever it left open must not outlive its DLL. */
void HooksDDraw_OnShellExit(void)
{
    if (Display_IsOpen()) {
        Display_Close();
        DebugPrintf("DirectDraw stand-in: display closed at shell exit\n");
    }
    RestoreWindow(g_device.window);
    g_front = NULL;
}

int Hooks_DisplayWanted(void)
{
    return ConfigHasWord("sdldisplay") || Hooks_SdlPlatform();
}

void HooksDDraw_OnShellLoaded(HMODULE shell)
{
    int n = 0;

    if (!Hooks_DisplayWanted())
        return;
    n += Hook_PatchImport(shell, "DDRAW.dll", "DirectDrawCreate",
                          (void *)StandInDirectDrawCreate);
    n += Hook_PatchImport(shell, "DDRAW.dll", "DirectDrawEnumerateA",
                          (void *)StandInDirectDrawEnumerateA);
    n += Hook_PatchImport(shell, "USER32.dll", "ScreenToClient", (void *)StandInScreenToClient);
    n += Hook_PatchImport(shell, "USER32.dll", "ClientToScreen", (void *)StandInClientToScreen);
    n += Hook_PatchImport(shell, "USER32.dll", "GetWindowRect", (void *)StandInGetWindowRect);
    DebugPrintf("HGSHELL.DLL: %d DirectDraw import(s) replaced by the SDL3 stand-in\n", n);
}
