/*
 * HGVK.DLL: Vulkan renderer plugin for Heavy Gear.
 *
 * Implements the plugin contract of 3DFXGEAR.DLL ("the plugin owns the
 * screen", type id 2). The sim hands over pre-projected polygons through
 * Render and draws its 2D HUD into the buffer LockDisplay returns; vk.c turns
 * both into a Vulkan frame. See docs/renderer.md and the notes in docs/renderer.
 *
 * All exports are cdecl. Set HGVK_LOG=2 to log every Render call. If Vulkan
 * cannot start, the HUD is still shown through GDI and the 3D view stays empty.
 */
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#include "hud2d.h"
#include "vk.h"

#define EXPORT __declspec(dllexport)

#define SCREEN_W 640
#define SCREEN_H 480
#define PLUGIN_ID 2 /* /3dfx contract: the plugin owns the display */

/* The block FirstDevice receives (docs/renderer/plugin-api.md, "Parameter struct P"). */
typedef struct FirstDeviceParams {
    void *surfaceDesc; /* +0x00 */
    int textureBpp;    /* +0x04 */
    unsigned flags;    /* +0x08 */
    int pad0c;         /* +0x0c */
    int fogMode;       /* +0x10 */
    unsigned fogColor; /* +0x14 */
    float fogStart;    /* +0x18 */
    float fogEnd;      /* +0x1c */
    void *ddraw;       /* +0x20, NULL in the /3dfx contract */
    int pad24;         /* +0x24 */
    int cacheSize;     /* +0x28 */
    void *target;      /* +0x2c */
    int pad30[3];      /* +0x30 */
    int width;         /* +0x3c */
    int height;        /* +0x40 */
    unsigned zMask;    /* +0x44 */
    int pad48;         /* +0x48 */
    HWND hwnd;         /* +0x4c */
} FirstDeviceParams;

/* The sim's polygon vertex, 36 bytes (docs/renderer/plugin-api.md). */
#define VERTEX_STRIDE 0x24

static FILE *g_log;
static int g_verbose;
static unsigned g_frame;
static int g_hudDumpPending; /* F12: save the next 2D image as hgvk-hud.bmp */
static unsigned g_renders;
static unsigned short *g_screen; /* RGB565, what the sim draws its 2D into */
static unsigned char *g_screenMem; /* g_screen with SCREEN_SLACK before it */

/*
 * The sim's 2D window record takes its height from the window (1440 lines at
 * 1440p), not from the 480-line picture, and its drawing clips to that, so
 * panes can reach rows past 479. The buffer has that many rows, and slack
 * before its start (a crash showed writes just before the HUD hooks' copy).
 */
#define SCREEN_SLACK 0x10000
static HWND g_hwnd;
static int g_linear; /* bilinear texture filtering, SetState(4) */

void HgvkLog(const char *fmt, ...)
{
    va_list ap;

    if (g_log == NULL)
        return;
    va_start(ap, fmt);
    vfprintf(g_log, fmt, ap);
    va_end(ap);
    fputc('\n', g_log);
    fflush(g_log);
}

BOOL WINAPI DllMain(HINSTANCE inst, DWORD reason, LPVOID reserved)
{
    (void)inst;
    (void)reserved;
    if (reason == DLL_PROCESS_ATTACH) {
        const char *v = getenv("HGVK_LOG");

        g_verbose = v != NULL ? atoi(v) : 0;
        g_log = fopen("hgvk.log", "w");
        HgvkLog("HGVK loaded");
    } else if (reason == DLL_PROCESS_DETACH && g_log != NULL) {
        HgvkLog("HGVK unloaded");
        fclose(g_log);
        g_log = NULL;
    }
    return TRUE;
}

EXPORT int GetFuncDesc(char *kind, int *id, char *extra)
{
    strcpy(kind, "RASTER");
    if (extra != NULL)
        strcpy(extra, "Vulkan");
    *id = PLUGIN_ID;
    HgvkLog("GetFuncDesc");
    return 0;
}

EXPORT int CheckFunc(const char *driverName)
{
    HgvkLog("CheckFunc(%s)", driverName != NULL ? driverName : "NULL");
    return 1;
}

EXPORT int GetNumDevice(void *buf)
{
    (void)buf;
    return 0;
}

EXPORT void SetLumaTable(const unsigned char *table)
{
    (void)table;
    HgvkLog("SetLumaTable");
}

/*
 * With the depth buffer on, every Render call is made a little nearer than the
 * one before (its depth 1/z scaled up a little per call). Surfaces the game draws on top
 * of others at the same depth, like decals on the ground, then still win, and
 * the depth test only overrides the game's order where depths really differ.
 * The whole frame's bias is kept to about 0.1%: the game draws smoke, shadows
 * and other translucent polygons after the sorted world, so a bias that grows
 * with every call (it was 0.002% per call, 3% or more by then) pulled them in
 * front of things that are really nearer. Each step is at least two float
 * steps of depth, so ties still go to the later call.
 */
#define ORDER_BIAS_SPREAD 1.0e-3
#define ORDER_BIAS_MIN_STEP 2.5e-7
static int g_depth;
static unsigned g_order;
static double g_orderStep = ORDER_BIAS_MIN_STEP;
static float g_orderBias = 1.0f;
/*
 * Nearest depth of the last polygon that had one. Smoke and spark particles
 * are tiny screen-space quads whose z the game fills for one corner only, or
 * not at all; the original simply drew them over what came before. Corners
 * without a z take the polygon's own depth, and polygons with none take this.
 */
static float g_lastIz = -1.0f;

/* True when hgrc.cfg (written by tools/dev/dev.cmd) contains the word. */
static int ConfigHasWord(const char *word)
{
    char buf[32];
    FILE *fp = fopen("hgrc.cfg", "r");
    int found = 0;

    if (fp == NULL)
        return 0;
    while (!found && fscanf(fp, "%31s", buf) == 1)
        found = _stricmp(buf, word) == 0;
    fclose(fp);
    return found;
}

/* Starts the full-resolution HUD layer's next frame for the window's current size. */
static void FrameHud(void)
{
    RECT rc;

    if (g_hwnd != NULL && GetClientRect(g_hwnd, &rc))
        Hud_Frame(rc.right, rc.bottom);
}

EXPORT int FirstDevice(FirstDeviceParams *p)
{
    HgvkLog("FirstDevice: bpp=%d flags=0x%x %dx%d zmask=0x%x hwnd=%p ddraw=%p cache=%d",
            p->textureBpp, p->flags, p->width, p->height, p->zMask, (void *)p->hwnd, p->ddraw,
            p->cacheSize);
    g_hwnd = p->hwnd;
    g_linear = (p->flags & 0x200) != 0;
    /* round 33: the fog as 1.2's dx5draw reads it (flags bit 2: fog on, colour +0x14; mode 3 linear, start / end
       +0x18 / +0x1c are D3D light states, which pre-transformed vertices ignore: the fog amount is per vertex) */
    HgvkLog("FirstDevice: fog %s mode %d colour %08x start %g end %g", (p->flags & 2) ? "on" : "off", p->fogMode,
            p->fogColor, p->fogStart, p->fogEnd);
    Vk_SetFog((p->flags & 2) != 0, p->fogColor);
    {
        int rows = SCREEN_H;

        if (p->height > rows)
            rows = p->height;
        if (GetSystemMetrics(SM_CYVIRTUALSCREEN) > rows)
            rows = GetSystemMetrics(SM_CYVIRTUALSCREEN);
        g_screenMem = (unsigned char *)calloc(
            (size_t)SCREEN_W * (size_t)rows * sizeof(unsigned short) + 2 * SCREEN_SLACK, 1);
        if (g_screenMem == NULL)
            return 1;
        g_screen = (unsigned short *)(g_screenMem + SCREEN_SLACK);
        HgvkLog("FirstDevice: 2D buffer of %d rows", rows);
    }
    g_depth = !ConfigHasWord("nozbuf");
    if (Vk_Init(p->hwnd, g_depth) != 0)
        HgvkLog("FirstDevice: Vulkan unavailable, falling back to GDI for the HUD");
    else
        Hud_SetTarget(g_screen); /* the HUD hooks may now draw at full resolution */
    FrameHud();
    return 0;
}

EXPORT void LastDevice(void)
{
    HgvkLog("LastDevice");
    Hud_SetTarget(NULL);
    Vk_Shutdown();
    free(g_screenMem);
    g_screenMem = NULL;
    g_screen = NULL;
}

EXPORT unsigned GetSocketCaps(void)
{
    return 0;
}

#define HGVK_STATE_PAINTER 0x48470001u   /* "HG" 1; the sim's W32_STATE_PAINTER */
static int g_painter;
/*
 * State 0xd (1.2's 0xc) clears the depth buffer at that point of the frame, as 3dfxgear's SetState 12 does (a
 * depth-only grBufferClear). 1.2 sends it before its cloud layer (the sky routine 0x1008dff3) and again after the sky
 * and background, before the world (0x1002a195 = Sim_100352d5): the cloud card, nearer than the hills it hangs over,
 * so lands behind the scene (round 33; this replaced a stopgap that drew the call after the first clear without depth).
 */

EXPORT int SetState(unsigned code, unsigned value)
{
    /* each distinct call once: 1.2's sky band sets a state every frame (round 31) */
    static unsigned seen[16][2];
    static int nSeen;
    int i;

    for (i = 0; i < nSeen && !(seen[i][0] == code && seen[i][1] == value); i++)
        ;
    if (i == nSeen && nSeen < 16) {
        seen[nSeen][0] = code;
        seen[nSeen][1] = value;
        nSeen++;
        HgvkLog("SetState(0x%x, 0x%x)", code, value);
    }
    if (code == 4)
        g_linear = (value & 0x20000) != 0;
    /* round 32: our own state, set by the sim around 1.2's particle squares (src/sim/fx), which carry no depth:
       draw them in painter's order, as 1.2's renderer drew everything */
    if (code == HGVK_STATE_PAINTER)
        g_painter = value != 0;
    if (code == 0xd && value != 0) {
        Vk_ClearDepth();
        g_lastIz = -1.0f;       /* as at the frame's start */
    }
    return 0;
}

EXPORT int SetTexturePalette(int unused, const unsigned *palette)
{
    static unsigned last[256];
    static int haveLast;

    (void)unused;
    HgvkLog("SetTexturePalette: [0]=%06x [1]=%06x [255]=%06x", palette[0], palette[1],
            palette[255]);
    /* Textures are converted with the palette they had when first drawn: a changed palette (the brightness
       slider, a mission's palette switch) drops them so they are converted again with the new one. */
    if (haveLast && memcmp(last, palette, sizeof(last)) != 0)
        Vk_RequestDropAllTextures();
    memcpy(last, palette, sizeof(last));
    haveLast = 1;
    return 0;
}

/* ---- Render ------------------------------------------------------------- */

/* Per mode, from docs/renderer/plugin-api.md. Modes above 18 are not drawn. */
typedef struct ModeInfo {
    unsigned char textured, keyed, clamp, blend, flat, topology;
} ModeInfo;

static const ModeInfo g_modes[19] = {
    /*  0 */ {1, 0, 1, 0, 0, HGVK_TRIANGLES},
    /*  1 */ {1, 0, 1, 0, 0, HGVK_TRIANGLES},
    /*  2 */ {1, 0, 0, 0, 0, HGVK_TRIANGLES},
    /*  3 */ {1, 1, 1, 0, 0, HGVK_TRIANGLES},
    /*  4 */ {1, 1, 0, 0, 0, HGVK_TRIANGLES},
    /*  5 */ {1, 1, 1, 1, 0, HGVK_TRIANGLES},
    /*  6 */ {1, 1, 0, 1, 0, HGVK_TRIANGLES},
    /*  7 */ {1, 0, 1, 0, 1, HGVK_TRIANGLES},
    /*  8 */ {1, 1, 1, 0, 1, HGVK_TRIANGLES},
    /*  9 */ {1, 1, 0, 0, 1, HGVK_TRIANGLES},
    /* 10 */ {1, 1, 1, 1, 1, HGVK_TRIANGLES},
    /* 11 */ {1, 1, 0, 1, 1, HGVK_TRIANGLES},
    /* 12 */ {0, 0, 0, 0, 1, HGVK_TRIANGLES},
    /* 13 */ {0, 0, 0, 0, 0, HGVK_TRIANGLES},
    /* 14 */ {0, 0, 0, 1, 1, HGVK_TRIANGLES},
    /* 15 */ {0, 0, 0, 1, 0, HGVK_TRIANGLES},
    /* 16 */ {0, 0, 0, 0, 1, HGVK_LINES},
    /* 17 */ {0, 0, 0, 0, 0, HGVK_LINES},
    /* 18 */ {0, 0, 0, 0, 1, HGVK_POINTS},
};

/*
 * The colour combine of 1.2's 3dfxgear.dll (Diego: "full voodoo"; the sim treats HGVK as its 3DFX renderer, w21).
 * Its Render switch (3dfxgear 0x10001590, mode -> case byte table 0x10001b24) sets grColorCombine per 1.2 mode; in
 * the sim's 1.0 numbering (1.0 modes 5..18 are 1.2's 7..20):
 *   0..4    texture x iterated colour (SCALE_OTHER, LOCAL, ITERATED, TEXTURE)              - HGVK_TEX_COLOUR
 *   5..11   texture only (SCALE_OTHER, ONE, -, TEXTURE); in 5 / 6 / 10 / 11 the alpha is the texture's
 *           (grAlphaCombine SCALE_OTHER, ONE, -, TEXTURE) with source-alpha blending          - HGVK_TEX_ONLY
 *   12      the constant colour grConstantColorValue(tex arg dword +4) (LOCAL, -, CONSTANT)    - HGVK_CONSTANT
 *   13..15  iterated colour, smooth (LOCAL, -, ITERATED); 14 / 15 blended by the iterated alpha - HGVK_ITERATED
 *   16..18  lines and points: 1.2 sets no combine and keeps the last polygon's; here their own colour
 *           (Diego, 2026-10-31: "Keep their own colour")
 */
enum { HGVK_TEX_COLOUR, HGVK_TEX_ONLY, HGVK_CONSTANT, HGVK_ITERATED, HGVK_KEEP };
static const unsigned char g_combine[19] = {
    HGVK_TEX_COLOUR, HGVK_TEX_COLOUR, HGVK_TEX_COLOUR, HGVK_TEX_COLOUR, HGVK_TEX_COLOUR,
    HGVK_TEX_ONLY, HGVK_TEX_ONLY, HGVK_TEX_ONLY, HGVK_TEX_ONLY, HGVK_TEX_ONLY, HGVK_TEX_ONLY, HGVK_TEX_ONLY,
    HGVK_CONSTANT, HGVK_ITERATED, HGVK_ITERATED, HGVK_ITERATED, HGVK_KEEP, HGVK_KEEP, HGVK_KEEP,
};

static HgvkVertex *g_scratch;
static int g_scratchCap;

/*
 * Textures offered with the description's flag +0x18 (1.2's .TEX textures, round 31): their pixel pointers, so a
 * texture keeps its alpha when it is made again (after a palette change) from a description without the flag, as
 * 1.2's renderer keeps the format it chose when the texture was first offered. The sim loads HGVK.DLL again with
 * every mission, so the list starts empty with each one.
 */
#define ALPHA_KEYS 8192
static const void *g_alphaKeys[ALPHA_KEYS];

static unsigned AlphaSlot(const void *key)
{
    return (unsigned)(((uintptr_t)key >> 4) * 2654435761u) % ALPHA_KEYS;
}

static void AlphaMark(const void *key)
{
    unsigned i = AlphaSlot(key), n;

    for (n = 0; n < ALPHA_KEYS && g_alphaKeys[i] != NULL && g_alphaKeys[i] != key; n++)
        i = (i + 1) % ALPHA_KEYS;
    if (n < ALPHA_KEYS)
        g_alphaKeys[i] = key;
}

static int AlphaMarked(const void *key)
{
    unsigned i = AlphaSlot(key), n;

    for (n = 0; n < ALPHA_KEYS && g_alphaKeys[i] != NULL; n++, i = (i + 1) % ALPHA_KEYS)
        if (g_alphaKeys[i] == key)
            return 1;
    return 0;
}

/* The texture argument is a pointer to a pointer to {w, h, pixels8, palette, key} (1.2: and +0x14, +0x18 flag). */
static HgvkTexture *GetTexture(const void *texArg, int keyed)
{
    const uintptr_t *indirect = (const uintptr_t *)texArg;
    const int *t;
    const uint8_t *pixels;
    const uint32_t *palette;
    HgvkTexture *tex;
    int w, h;

    if (texArg == NULL || IsBadReadPtr(indirect, sizeof(*indirect)))
        return NULL;
    t = (const int *)*indirect;
    if (t == NULL || IsBadReadPtr(t, 0x14))
        return NULL;
    w = t[0];
    h = t[1];
    pixels = (const uint8_t *)(uintptr_t)t[2];
    palette = (const uint32_t *)(uintptr_t)t[3];
    if (w <= 0 || h <= 0 || w > 4096 || h > 4096 || pixels == NULL || palette == NULL)
        return NULL;

    if (!IsBadReadPtr(t, 0x1c) && t[6] == 1)
        AlphaMark(pixels);
    tex = Vk_FindTexture(pixels);
    if (tex != NULL)
        return tex;
    if (IsBadReadPtr(pixels, (UINT_PTR)w * h) || IsBadReadPtr(palette, 256 * 4))
        return NULL;
    tex = Vk_CreateTexture(pixels, w, h, pixels, palette,
                           AlphaMarked(pixels) ? HGVK_KEY_ALPHA : keyed ? t[4] : -1);
    if (tex == NULL)
        HgvkLog("Render: could not create a %dx%d texture", w, h);
    return tex;
}

/*
 * The vertex colour exactly as 1.2's dx5draw.dll builds it (0x10004360): each colour byte times the light dword +0x10,
 * unsigned and wrapping, shifted down 8 and packed by shifting and ORing with no clamp, so a channel over 255 spills
 * into the next byte. The light can be huge: the particle squares reuse their corners and the textured submitter
 * shifts the light (light << 3 | 7) at every draw, as in 1.2, so after a few frames it is 0x7fffff .. 0xffffffff,
 * which this packing turns into white (round 33; a clamped signed product gave black).
 */
static uint32_t PackColour(const uint8_t *v, int blend)
{
    uint32_t light = *(const uint32_t *)(v + 0x10);
    uint32_t r = (uint32_t)v[0x14] * light >> 8;
    uint32_t g = (uint32_t)v[0x15] * light >> 8;
    uint32_t b = (uint32_t)v[0x16] * light >> 8;
    uint32_t d = (((((uint32_t)v[0x17] << 8) | r) << 8 | g) << 8) | b;      /* D3D's ARGB */
    uint32_t a = blend ? d >> 24 : 255;

    return ((d >> 16) & 0xff) | (((d >> 8) & 0xff) << 8) | ((d & 0xff) << 16) | (a << 24);
}

static unsigned g_badRhw, g_noTexture;

/*
 * Every Render call of one frame is written to hgvk.log, to study the scene:
 * frame 300, and the frame after each press of F12.
 */
#define DUMP_FRAME 300
static unsigned g_dumpFrame = DUMP_FRAME;

static void ReadVertex(HgvkVertex *out, const uint8_t *v, int blend, int flat,
                       const uint8_t *first)
{
    out->x = *(const float *)(v + 0x00);
    out->y = *(const float *)(v + 0x04);
    out->rhw = *(const float *)(v + 0x0c);
    /* Zero, negative or garbage rhw would send the vertex to infinity and the
       whole polygon would be clipped away. Treat it as screen space (vk.h). */
    if (!(out->rhw > 1.0e-6f && out->rhw < 1.0e6f)) {
        out->rhw = -1.0f;
        g_badRhw++;
    }
    /*
     * Depth comes from the camera depth z, not rhw: the game sends rhw = 1 for
     * polygons it wants drawn without perspective (most translucent ones) but
     * still gives their real z. z <= 1 marks backdrops such as the sky band,
     * which belong behind everything.
     */
    {
        float z = *(const float *)(v + 0x08);

        out->iz = z > 1.0f && z < 1.0e9f ? g_orderBias / z : -1.0f;
    }
    out->rgba = PackColour(flat ? first : v, blend);
    out->u = *(const float *)(v + 0x1c);
    out->v = *(const float *)(v + 0x20);
    out->fog = (float)v[0x1b] / 255.0f;
}

EXPORT int Render(int mode, void *surf, void *verts, int count, void *tex)
{
    const ModeInfo *m;
    const uint8_t *src = (const uint8_t *)verts;
    HgvkTexture *texture = NULL;
    int out = 0, i, needed, vblend;

    (void)surf;
    g_renders++;
    if (g_verbose >= 2)
        HgvkLog("Render(mode=%d, n=%d)", mode, count);
    if (!Vk_IsReady() || mode < 0 || mode > 18 || count <= 0 || verts == NULL)
        return 0;
    m = &g_modes[mode];
    if (g_depth)
        g_orderBias = (float)(1.0 + g_orderStep * g_order++);

    if (m->topology == HGVK_TRIANGLES) {
        if (count < 3)
            return 0;
        needed = (count - 2) * 3;
    } else if (m->topology == HGVK_LINES) {
        needed = count * 2;
    } else {
        needed = count;
    }
    if (needed > g_scratchCap) {
        HgvkVertex *grown = (HgvkVertex *)realloc(g_scratch, (size_t)needed * sizeof(*grown));

        if (grown == NULL)
            return 0;
        g_scratch = grown;
        g_scratchCap = needed;
    }

    if (g_frame == g_dumpFrame) {
        float zMin = 1e30f, zMax = -1e30f, rMin = 1e30f, rMax = -1e30f;
        float xMin = 1e30f, xMax = -1e30f, yMin = 1e30f, yMax = -1e30f;

        for (i = 0; i < count; i++) {
            const uint8_t *v = src + (size_t)i * VERTEX_STRIDE;
            float x = *(const float *)(v + 0x00), y = *(const float *)(v + 0x04);
            float z = *(const float *)(v + 0x08), r = *(const float *)(v + 0x0c);

            if (x < xMin) xMin = x;
            if (x > xMax) xMax = x;
            if (y < yMin) yMin = y;
            if (y > yMax) yMax = y;
            if (z < zMin) zMin = z;
            if (z > zMax) zMax = z;
            if (r < rMin) rMin = r;
            if (r > rMax) rMax = r;
        }
        if (g_frame == g_dumpFrame) {
            const float *v0 = (const float *)src;

            /* Exact first-vertex position: shows whether the game snaps to whole pixels. */
            HgvkLog("dump %u: first vertex x %.4f y %.4f", g_renders, v0[0], v0[1]);
        }
        if (g_frame == g_dumpFrame)
            HgvkLog("dump %u: mode %d n %d tex %p x %.0f..%.0f y %.0f..%.0f z %g..%g rhw %g..%g "
                    "rgba %08x bias %.7f",
                    g_renders, mode, count, tex, xMin, xMax, yMin, yMax, zMin, zMax, rMin, rMax,
                    *(const uint32_t *)(src + 0x14), g_orderBias);
    }

    if (m->textured) {
        texture = GetTexture(tex, m->keyed);
        if (texture == NULL) {
            if (g_noTexture++ < 20)
                HgvkLog("Render: mode %d, texture %p could not be read", mode, tex);
            return 1;
        }
    }

    /* 1.2's dx5draw never sets the texture blend, so D3D's default MODULATE applies: a texture with alpha (1.2's .TEX
       effect textures) replaces the vertex alpha with its own; the vertex alpha counts only without texture alpha */
    vblend = m->blend && !(texture != NULL && Vk_TextureHasAlpha(texture));

    if (m->topology == HGVK_TRIANGLES) {
        for (i = 1; i + 1 < count; i++) {
            ReadVertex(&g_scratch[out++], src, vblend, m->flat, src);
            ReadVertex(&g_scratch[out++], src + (size_t)i * VERTEX_STRIDE, vblend, m->flat, src);
            ReadVertex(&g_scratch[out++], src + (size_t)(i + 1) * VERTEX_STRIDE, vblend, m->flat,
                       src);
        }
    } else if (m->topology == HGVK_LINES) {
        for (i = 0; i < count; i++) {
            ReadVertex(&g_scratch[out++], src + (size_t)i * VERTEX_STRIDE, 0, m->flat, src);
            ReadVertex(&g_scratch[out++], src + (size_t)((i + 1) % count) * VERTEX_STRIDE, 0,
                       m->flat, src);
        }
    } else {
        for (i = 0; i < count; i++)
            ReadVertex(&g_scratch[out++], src + (size_t)i * VERTEX_STRIDE, 0, 1,
                       src + (size_t)i * VERTEX_STRIDE);
    }

    /* 3dfxgear's colour combine (above) */
    switch (g_combine[mode]) {
    case HGVK_TEX_ONLY:
        for (i = 0; i < out; i++)
            g_scratch[i].rgba = 0xffffffffu;        /* texture only: its colour and (blended modes) its alpha */
        break;
    case HGVK_CONSTANT:
        if (tex != NULL) {
            uint32_t c = *(const uint32_t *)((const uint8_t *)tex + 4);      /* Glide's ARGB */

            for (i = 0; i < out; i++)
                g_scratch[i].rgba = ((c >> 16) & 0xff) | (c & 0xff00) | ((c & 0xff) << 16) | (c & 0xff000000u);
        }
        break;
    case HGVK_ITERATED:
        if (m->flat && m->topology == HGVK_TRIANGLES) {      /* smooth: each corner its own colour */
            out = 0;
            for (i = 1; i + 1 < count; i++) {
                ReadVertex(&g_scratch[out++], src, vblend, 0, src);
                ReadVertex(&g_scratch[out++], src + (size_t)i * VERTEX_STRIDE, vblend, 0, src);
                ReadVertex(&g_scratch[out++], src + (size_t)(i + 1) * VERTEX_STRIDE, vblend, 0, src);
            }
        }
        break;
    default:
        break;
    }

    if (g_depth) {
        float nearest = -1.0f;

        for (i = 0; i < out; i++)
            if (g_scratch[i].iz > nearest)
                nearest = g_scratch[i].iz;
        if (nearest > 0.0f)
            g_lastIz = nearest;
        else
            nearest = g_lastIz;
        for (i = 0; i < out; i++)
            if (g_scratch[i].iz <= 0.0f)
                g_scratch[i].iz = nearest;
    }

    /* a texture with alpha (1.2's .TEX) is not colour-keyed: it blends in the modes with blending, else is opaque */
    Vk_AddDraw(m->topology, m->blend, (m->clamp ? 1 : 0) | (g_linear ? 2 : 0),
               Vk_TextureHasAlpha(texture) ? 0 : m->keyed, g_painter, texture, g_scratch, out);
    return 0;
}

EXPORT int RenderNoClip(int mode, void *surf, void *verts, int count, void *tex)
{
    return Render(mode, surf, verts, count, tex);
}

EXPORT int RenderRefresh(void)
{
    if (g_frame < 5 || g_frame % 300 == 0)
        HgvkLog("RenderRefresh: frame %u, %u Render calls, bad rhw %u, missing textures %u",
                g_frame, g_renders, g_badRhw, g_noTexture);
    if (GetAsyncKeyState(VK_F12) & 1) {
        g_dumpFrame = g_frame + 1;
        HgvkLog("F12: dumping frame %u", g_dumpFrame);
        g_hudDumpPending = 1;
    }
    g_frame++;
    /* Spread the bias over about as many calls as this frame had. */
    g_orderStep = g_renders > 0 ? ORDER_BIAS_SPREAD / g_renders : ORDER_BIAS_MIN_STEP;
    if (g_orderStep < ORDER_BIAS_MIN_STEP)
        g_orderStep = ORDER_BIAS_MIN_STEP;
    g_renders = 0;
    g_order = 0;
    g_lastIz = -1.0f;
    return 0;
}

/*
 * 1.2's renderer makes a texture when it is offered, in the format the description asks for: with the flag +0x18 a
 * 16-bit ARGB one (round 31). Such a texture is made here, so later draws (whose descriptions may lack the flag)
 * find it; other textures are still made on first use.
 */
EXPORT int PreloadTexture(void *tex, int keyed)
{
    const uintptr_t *indirect = (const uintptr_t *)tex;
    const int *t;

    if (tex != NULL && !IsBadReadPtr(indirect, sizeof(*indirect)) && Vk_IsReady()) {
        t = (const int *)*indirect;
        if (t != NULL && !IsBadReadPtr(t, 0x1c) && t[6] == 1)
            GetTexture(tex, keyed);
    }
    return 1;
}

EXPORT int UpdateTexture(void *tex, int keyed)
{
    const uintptr_t *indirect = (const uintptr_t *)tex;
    const int *t;

    (void)keyed;
    if (tex != NULL && !IsBadReadPtr(indirect, sizeof(*indirect))) {
        t = (const int *)*indirect;
        if (t != NULL && !IsBadReadPtr(t, 0x14))
            Vk_DropTexture((const void *)(uintptr_t)t[2]);
    }
    return 0;
}

EXPORT int RestoreDevice(void)
{
    HgvkLog("RestoreDevice");
    return 0;
}

EXPORT int LostDeviceDisplay(void)
{
    HgvkLog("LostDeviceDisplay");
    return 0;
}

/* ---- 2D display --------------------------------------------------------- */

/*
 * *pitch is the row stride in BYTES (as Glide's strideInBytes): the sim's own
 * rect fill advances its pointer by it, and a dump of the first version, which
 * returned a width in pixels, showed the sim's rows 320 pixels (640 bytes) apart.
 */
EXPORT void *LockDisplay(int *pitch)
{
    if (g_frame < 3)
        HgvkLog("LockDisplay");
    *pitch = SCREEN_W * (int)sizeof(unsigned short);
    return g_screen;
}

EXPORT void UnlockDisplay(void)
{
}

/* Fallback when Vulkan is unavailable: stretch the 640x480 RGB565 image with GDI. */
static void PresentGdi(void)
{
    struct {
        BITMAPINFOHEADER header;
        DWORD masks[3];
    } bmi;
    RECT rc;
    HDC dc;

    if (g_hwnd == NULL || g_screen == NULL || !IsWindow(g_hwnd))
        return;
    memset(&bmi, 0, sizeof(bmi));
    bmi.header.biSize = sizeof(bmi.header);
    bmi.header.biWidth = SCREEN_W;
    bmi.header.biHeight = -SCREEN_H; /* top-down */
    bmi.header.biPlanes = 1;
    bmi.header.biBitCount = 16;
    bmi.header.biCompression = BI_BITFIELDS;
    bmi.masks[0] = 0xF800;
    bmi.masks[1] = 0x07E0;
    bmi.masks[2] = 0x001F;

    GetClientRect(g_hwnd, &rc);
    dc = GetDC(g_hwnd);
    if (dc == NULL)
        return;
    StretchDIBits(dc, 0, 0, rc.right, rc.bottom, 0, 0, SCREEN_W, SCREEN_H, g_screen,
                  (const BITMAPINFO *)&bmi, DIB_RGB_COLORS, SRCCOPY);
    ReleaseDC(g_hwnd, dc);
}

/*
 * Writes the RGB565 2D image as a 24-bit bitmap. With the full-resolution HUD on
 * this is only what the hooks did not take over; hgvk-hud2.bmp is the rest.
 */
static void DumpHud(void)
{
    static unsigned char row[SCREEN_W * 3];
    BITMAPFILEHEADER fh;
    BITMAPINFOHEADER ih;
    FILE *fp = fopen("hgvk-hud.bmp", "wb");
    int y, x;

    if (fp == NULL)
        return;
    memset(&fh, 0, sizeof(fh));
    memset(&ih, 0, sizeof(ih));
    fh.bfType = 0x4D42;
    fh.bfOffBits = sizeof(fh) + sizeof(ih);
    fh.bfSize = fh.bfOffBits + SCREEN_W * SCREEN_H * 3;
    ih.biSize = sizeof(ih);
    ih.biWidth = SCREEN_W;
    ih.biHeight = SCREEN_H; /* bottom-up */
    ih.biPlanes = 1;
    ih.biBitCount = 24;
    fwrite(&fh, sizeof(fh), 1, fp);
    fwrite(&ih, sizeof(ih), 1, fp);
    for (y = SCREEN_H - 1; y >= 0; y--) {
        for (x = 0; x < SCREEN_W; x++) {
            unsigned p = g_screen[y * SCREEN_W + x];

            row[x * 3] = (unsigned char)((p & 31) * 255 / 31);
            row[x * 3 + 1] = (unsigned char)(((p >> 5) & 63) * 255 / 63);
            row[x * 3 + 2] = (unsigned char)(((p >> 11) & 31) * 255 / 31);
        }
        fwrite(row, sizeof(row), 1, fp);
    }
    fclose(fp);
    HgvkLog("F12: 2D image saved as hgvk-hud.bmp");
}

/* The hooks in HG.EXE fetch the full-resolution HUD interface here (hooks_hud.c). */
EXPORT const HudApi *HgvkHudApi(void)
{
    return Hud_Api();
}

/* The sim calls this once per frame after its 3D polygons and its 2D drawing. */
EXPORT void RefreshDisplay(void)
{
    if (g_hudDumpPending && g_screen != NULL) {
        g_hudDumpPending = 0;
        DumpHud();
        Hud_Dump("hgvk-hud2.bmp");
    }
    if (g_frame < 3)
        HgvkLog("RefreshDisplay");
    if (g_screen == NULL)
        return;
    if (Vk_IsReady()) {
        int w2 = 0, h2 = 0;
        const uint32_t *hud2 = Hud_Pixels(&w2, &h2);

        Vk_Present(g_screen, SCREEN_W * (int)sizeof(unsigned short), hud2, w2, h2);
    } else {
        PresentGdi();
    }
    FrameHud();
    /* Clear for the next frame, like 3DFXGEAR clears its buffer after the swap. */
    memset(g_screen, 0, SCREEN_W * SCREEN_H * sizeof(unsigned short));
}
