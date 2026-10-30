/*
 * The sim's HUD at the window's full resolution (docs/renderer/hud.md).
 *
 * HGSIM.DLL draws its HUD with a small statically linked 2D library: lines,
 * pane wipes, RLE shapes and font glyphs (the 16-bit clone, 0x100ef545 to
 * 0x100f2f01), all into one 640x480 RGB565 buffer that HGVK.DLL hands out
 * through LockDisplay. These hooks wrap the drawing calls. Each wrapper lets the
 * original run, so the game sees exactly the same results, but with the pane's
 * window pointing at a scratch buffer. The same drawing is then repeated at the
 * window's resolution through the interface HGVK.DLL exports (hud_api.h).
 * Whatever is not wrapped still lands in the real buffer, which HGVK.DLL keeps
 * showing underneath, so nothing the sim draws can go missing.
 *
 * hgrc.cfg word "nohudhi" leaves these hooks out (the old 640x480 HUD).
 * Only active when the Vulkan renderer (first word "hgvk" or HG_RENDERER=hgvk) is in use.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../launcher/debug.h"
#include "../plugins/hgvk/hud_api.h"
#include "hook.h"

#define RVA_LUT 0x156DCC /* RGB565 value for each palette index, word per entry */

static unsigned char *g_sim;
static const HudApi *g_api;

/*
 * What the sim draws into, instead of the real buffer, while a wrapped call runs.
 * The sim's window record gives the window's height (1440 lines at 1440p), not
 * the picture's 480, and the drawing code clips to that, so the scratch buffer
 * has as many rows as the window says. It also has slack before its start: a
 * crash.log entry showed a variable 20 bytes before the old fixed-size buffer
 * overwritten, and another one right after it, during a mission.
 */
#define SCRATCH_SLACK 0x10000
#define SCRATCH_GUARD_CHECK 256 /* bytes before the start checked after each call */

static unsigned char *g_scratchMem;
static size_t g_scratchBytes;
static unsigned short *g_scratch;
static unsigned g_guardHits;

static int EnsureScratch(size_t bytes)
{
    unsigned char *mem;

    if (bytes <= g_scratchBytes)
        return 1;
    mem = (unsigned char *)calloc(bytes + 2 * SCRATCH_SLACK, 1);
    if (mem == NULL)
        return 0;
    free(g_scratchMem);
    g_scratchMem = mem;
    g_scratchBytes = bytes;
    g_scratch = (unsigned short *)(mem + SCRATCH_SLACK);
    DebugPrintf("HUD: scratch buffer of %u bytes\n", (unsigned)bytes);
    return 1;
}

static int ConfigHasWord(const char *word, int firstOnly)
{
    char buf[32];
    FILE *fp = fopen("hgrc.cfg", "r");
    int found = 0;

    if (fp == NULL)
        return 0;
    while (!found && fscanf(fp, "%31s", buf) == 1) {
        found = _stricmp(buf, word) == 0;
        if (firstOnly)
            break;
    }
    fclose(fp);
    return found;
}

static int VulkanRendererWanted(void)
{
    const char *env = getenv("HG_RENDERER");

    if (env != NULL)
        return _stricmp(env, "hgvk") == 0;
    return ConfigHasWord("hgvk", 1);
}

/* HGVK.DLL is loaded by the sim after these hooks go in. */
static int FindApi(void)
{
    if (g_api == NULL) {
        HMODULE plugin = GetModuleHandleA("HGVK.DLL");
        const HudApi *(__cdecl * get)(void);

        if (plugin == NULL)
            return 0;
        get = (const HudApi *(__cdecl *)(void))(void *)GetProcAddress(plugin, "HgvkHudApi");
        if (get == NULL)
            return 0;
        g_api = get();
        if (g_api == NULL || g_api->version != HUD_API_VERSION) {
            DebugPrintf("HUD: HGVK.DLL has a different HUD interface, full-resolution HUD off\n");
            g_api = NULL;
            return 0;
        }
        DebugPrintf("HUD: full-resolution interface found\n");
    }
    return 1;
}

typedef struct Pane {
    int *window; /* {buffer, row pitch in bytes - 1, height - 1} */
    int x0, y0, x1, y1;
} Pane;

typedef struct PaneInfo {
    int *window;
    int originX, originY; /* where the pane's (0, 0) is in the window */
    HudRect clip;
} PaneInfo;

static unsigned g_unmatched, g_diag;

/*
 * True when `pane` draws into the sim's HUD buffer. Panes inside a wrapped
 * call already see the scratch buffer and so do not match, which also keeps a
 * wrapped function that calls another one from drawing twice.
 */
static int MatchPane(const void *paneArg, PaneInfo *info)
{
    const Pane *pane = (const Pane *)paneArg;
    const void *target;
    UINT_PTR buffer;
    int w, h;

    if (pane == NULL || pane->window == NULL || !FindApi())
        return 0;
    target = g_api->Target();
    if (g_diag < 40) {
        g_diag++;
        DebugPrintf("HUD: call %u pane %p window %p buffer %08x pitch-1 %d h-1 %d target %p\n", g_diag,
                    (const void *)pane, (const void *)pane->window, (unsigned)pane->window[0],
                    pane->window[1], pane->window[2], target);
    }
    if (target == NULL)
        return 0;
    buffer = (UINT_PTR)(unsigned)pane->window[0];
    if (buffer != (UINT_PTR)target) {
        /* Log a window that sits inside the HUD buffer without being all of it. */
        if (buffer > (UINT_PTR)target &&
            buffer < (UINT_PTR)target + HUD_CELLS_W * HUD_CELLS_H * sizeof(unsigned short) &&
            g_unmatched < 20) {
            g_unmatched++;
            DebugPrintf("HUD: a window inside the HUD buffer is not the whole buffer: "
                        "offset %u, %dx%d\n",
                        (unsigned)(buffer - (UINT_PTR)target), pane->window[1] + 1,
                        pane->window[2] + 1);
        }
        return 0;
    }
    /*
     * The window's second field is the row pitch in bytes minus one: LockDisplay
     * returns bytes, and the sim stores that as "width - 1" and addresses rows
     * with it. The drawing code clips to it, so a pane is only ever as wide as
     * the picture.
     */
    w = pane->window[1] + 1;
    h = pane->window[2] + 1;
    /* The height field follows the window size (1439 at 1440 lines), not the 480-line picture. */
    if (w != HUD_CELLS_W * 2 || h < HUD_CELLS_H)
        return 0;
    if (!EnsureScratch((size_t)w * (size_t)h))
        return 0;
    info->window = pane->window;
    info->originX = pane->x0;
    info->originY = pane->y0;
    info->clip.x0 = pane->x0 > 0 ? pane->x0 : 0;
    info->clip.y0 = pane->y0 > 0 ? pane->y0 : 0;
    info->clip.x1 = pane->x1 < HUD_CELLS_W - 1 ? pane->x1 : HUD_CELLS_W - 1;
    info->clip.y1 = pane->y1 < HUD_CELLS_H - 1 ? pane->y1 : HUD_CELLS_H - 1;
    return info->clip.x1 >= info->clip.x0 && info->clip.y1 >= info->clip.y0;
}

static void Divert(const PaneInfo *info)
{
    info->window[0] = (int)(UINT_PTR)g_scratch;
}

static void Undivert(const PaneInfo *info)
{
    const unsigned char *guard = (const unsigned char *)g_scratch - SCRATCH_GUARD_CHECK;
    int i;

    info->window[0] = (int)(UINT_PTR)g_api->Target();
    /* Report drawing that landed before the buffer's start, then clear it. */
    for (i = 0; i < SCRATCH_GUARD_CHECK; i++) {
        if (guard[i] != 0)
            break;
    }
    if (i < SCRATCH_GUARD_CHECK) {
        if (g_guardHits < 10) {
            g_guardHits++;
            DebugPrintf("HUD: drawing landed %d bytes before the buffer's start\n",
                        SCRATCH_GUARD_CHECK - i);
        }
        memset((void *)guard, 0, SCRATCH_GUARD_CHECK);
    }
}

/* ---- Bitmaps ------------------------------------------------------------- */

static unsigned short *g_bitmap;
static size_t g_bitmapCap;

static unsigned short *BitmapBuffer(size_t cells)
{
    if (cells > g_bitmapCap) {
        free(g_bitmap);
        g_bitmap = (unsigned short *)malloc(cells * sizeof(unsigned short));
        g_bitmapCap = g_bitmap != NULL ? cells : 0;
    }
    return g_bitmap;
}

/*
 * A VFX shape: bounds at +8 (x0, y0, x1, y1, relative to the origin), then
 * 0x18: one run-length coded row after another. Each code byte b: 0 ends the
 * row, 1 skips the next byte's count of cells, an even b repeats the next
 * byte (palette index) b/2 times, an odd b > 1 copies the next b/2 bytes.
 */
static void DrawShape(const PaneInfo *info, const unsigned char *shape, int x, int y)
{
    const int *bounds = (const int *)(shape + 8);
    const unsigned short *lut = (const unsigned short *)(g_sim + RVA_LUT);
    int w = bounds[2] - bounds[0] + 1, h = bounds[3] - bounds[1] + 1, row;
    const unsigned char *p = shape + 0x18;
    unsigned short *bmp;

    if (w <= 0 || h <= 0 || w > 2048 || h > 2048)
        return;
    bmp = BitmapBuffer((size_t)w * (size_t)h);
    if (bmp == NULL)
        return;
    memset(bmp, 0, (size_t)w * (size_t)h * sizeof(unsigned short));
    for (row = 0; row < h; row++) {
        int col = 0;

        for (;;) {
            unsigned code = *p++, n;

            if (code == 0)
                break;
            if (code == 1) {
                col += *p++;
                continue;
            }
            n = code >> 1;
            if (code & 1) {
                for (; n > 0; n--, col++, p++) {
                    if (col >= 0 && col < w)
                        bmp[(size_t)row * (size_t)w + (size_t)col] = lut[*p];
                }
            } else {
                unsigned short value = lut[*p++];

                for (; n > 0; n--, col++) {
                    if (col >= 0 && col < w)
                        bmp[(size_t)row * (size_t)w + (size_t)col] = value;
                }
            }
        }
    }
    g_api->Bitmap(&info->clip, x + bounds[0], y + bounds[1], w, h, bmp);
}

/*
 * A font glyph: the width (dword) then width x font height bytes of palette
 * indices, each run through the caller's 256-byte lookaside; 0xFF is
 * transparent. The font gives its height at +8 and each glyph's offset at
 * +0x10 + 4 * code.
 */
static void DrawGlyph(const PaneInfo *info, int x, int y, const unsigned char *font, int code,
                      const unsigned char *lookaside)
{
    const unsigned short *lut = (const unsigned short *)(g_sim + RVA_LUT);
    const unsigned char *glyph = font + *(const int *)(font + 0x10 + (size_t)code * 4);
    int w = *(const int *)glyph, h = *(const int *)(font + 8), i;
    const unsigned char *src = glyph + 4;
    unsigned short *bmp;

    if (w <= 0 || h <= 0 || w > 1024 || h > 1024)
        return;
    bmp = BitmapBuffer((size_t)w * (size_t)h);
    if (bmp == NULL)
        return;
    for (i = 0; i < w * h; i++) {
        unsigned index = lookaside[src[i]];

        bmp[i] = index == 0xFF ? 0 : lut[index];
    }
    g_api->Bitmap(&info->clip, x, y, w, h, bmp);
}

/* ---- The wrapped functions ----------------------------------------------- */

/* VFX_line_draw(pane, x0, y0, x1, y1, mode, color); only mode 0 (draw) exists in this build. */
typedef int(__cdecl *LineFn)(void *pane, int x0, int y0, int x1, int y1, int mode, int color);
static LineFn g_line;

// FUNCTION: HGSIM 0x100ef727
static int __cdecl HookedLine(void *pane, int x0, int y0, int x1, int y1, int mode, int color)
{
    PaneInfo info;
    int result, bx0, by0, bx1, by1, x, y, w, h;
    unsigned short *bmp;

    if (mode != 0 || !MatchPane(pane, &info))
        return g_line(pane, x0, y0, x1, y1, mode, color);
    /*
     * The cells a line covers come from the game's own line routine: it runs
     * on the cleared scratch buffer and the cells it set are handed over as a
     * bitmap, so slanted lines (the radar's field of view) look exactly as before.
     * It returns 0 for a line drawn as given and 1 for one drawn after clipping
     * (the radar's rays run out of their pane); 2 and below mean nothing drawn.
     */
    bx0 = (x0 < x1 ? x0 : x1) + info.originX;
    bx1 = (x0 < x1 ? x1 : x0) + info.originX;
    by0 = (y0 < y1 ? y0 : y1) + info.originY;
    by1 = (y0 < y1 ? y1 : y0) + info.originY;
    if (bx0 < info.clip.x0) bx0 = info.clip.x0;
    if (by0 < info.clip.y0) by0 = info.clip.y0;
    if (bx1 > info.clip.x1) bx1 = info.clip.x1;
    if (by1 > info.clip.y1) by1 = info.clip.y1;
    w = bx1 - bx0 + 1;
    h = by1 - by0 + 1;
    if (w <= 0 || h <= 0 || (unsigned)(color & 0xFFFF) == 0) {
        Divert(&info);
        result = g_line(pane, x0, y0, x1, y1, mode, color);
        Undivert(&info);
        return result;
    }
    for (y = by0; y <= by1; y++)
        memset(&g_scratch[y * HUD_CELLS_W + bx0], 0, (size_t)w * sizeof(unsigned short));
    Divert(&info);
    result = g_line(pane, x0, y0, x1, y1, mode, color);
    Undivert(&info);
    if ((result == 0 || result == 1) && (bmp = BitmapBuffer((size_t)w * (size_t)h)) != NULL) {
        for (y = 0; y < h; y++) {
            for (x = 0; x < w; x++)
                bmp[y * w + x] = g_scratch[(by0 + y) * HUD_CELLS_W + bx0 + x];
        }
        g_api->Bitmap(&info.clip, bx0, by0, w, h, bmp);
    }
    return result;
}

/* VFX_shape_draw(pane, shape table, shape number, x, y), clipped to the pane. */
typedef int(__cdecl *ShapeFn)(void *pane, const unsigned char *table, int number, int x, int y);
static ShapeFn g_shape;

// FUNCTION: HGSIM 0x100f0205
static int __cdecl HookedShape(void *pane, const unsigned char *table, int number, int x, int y)
{
    PaneInfo info;
    int result;

    if (!MatchPane(pane, &info))
        return g_shape(pane, table, number, x, y);
    Divert(&info);
    result = g_shape(pane, table, number, x, y);
    Undivert(&info);
    if (result == 0) {
        DrawShape(&info, table + *(const int *)(table + 8 + (size_t)number * 8),
                  x + info.originX, y + info.originY);
    }
    return result;
}

/*
 * The same without clipping; takes the shape itself instead of a table and a
 * number. It uses a fifth argument slot as a local variable, so callers push
 * five words (the clipped version does), and so does this wrapper.
 */
typedef int(__cdecl *ShapeRawFn)(void *pane, const unsigned char *shape, int x, int y, int slot);
static ShapeRawFn g_shapeRaw;

// FUNCTION: HGSIM 0x100f0727
static int __cdecl HookedShapeRaw(void *pane, const unsigned char *shape, int x, int y, int slot)
{
    PaneInfo info;
    int result;

    if (!MatchPane(pane, &info))
        return g_shapeRaw(pane, shape, x, y, slot);
    Divert(&info);
    result = g_shapeRaw(pane, shape, x, y, slot);
    Undivert(&info);
    if (result == 0)
        DrawShape(&info, shape, x + info.originX, y + info.originY);
    return result;
}

/* VFX_pane_wipe(pane, color): fills the whole pane. */
typedef int(__cdecl *WipeFn)(void *pane, int color);
static WipeFn g_wipe;

// FUNCTION: HGSIM 0x100f0865
static int __cdecl HookedWipe(void *pane, int color)
{
    PaneInfo info;
    int result;

    if (!MatchPane(pane, &info))
        return g_wipe(pane, color);
    Divert(&info);
    result = g_wipe(pane, color);
    Undivert(&info);
    if (result == 0)
        g_api->Fill(&info.clip, (unsigned)color & 0xFFFF);
    return result;
}

/* VFX_character_draw(pane, x, y, font, code, lookaside): returns the glyph's width. */
typedef int(__cdecl *CharFn)(void *pane, int x, int y, const unsigned char *font, int code,
                             const unsigned char *lookaside);
static CharFn g_char;

// FUNCTION: HGSIM 0x100f1c93
static int __cdecl HookedChar(void *pane, int x, int y, const unsigned char *font, int code,
                              const unsigned char *lookaside)
{
    PaneInfo info;
    int result;

    if (!MatchPane(pane, &info))
        return g_char(pane, x, y, font, code, lookaside);
    Divert(&info);
    result = g_char(pane, x, y, font, code, lookaside);
    Undivert(&info);
    if (result >= 0)
        DrawGlyph(&info, x + info.originX, y + info.originY, font, code, lookaside);
    return result;
}

static const unsigned char kProloguePushRawShape[] = {0x55, 0x8B, 0xEC, 0x53, 0x56};

/* Each prologue is push ebp; mov ebp,esp; add esp,-N with a different N. */
static const unsigned char kPrologueLine[] = {0x55, 0x8B, 0xEC, 0x83, 0xC4, 0xA4};
static const unsigned char kPrologueShape[] = {0x55, 0x8B, 0xEC, 0x83, 0xC4, 0xAC};
static const unsigned char kPrologueWipe[] = {0x55, 0x8B, 0xEC, 0x83, 0xC4, 0xDC};
static const unsigned char kPrologueChar[] = {0x55, 0x8B, 0xEC, 0x83, 0xC4, 0xCC};

static const HookDesc g_hooks[] = {
    {.module = "HGSIM.DLL", .rva = 0xEF727, .name = "VfxLineDraw", .expect = kPrologueLine,
     .stolen = sizeof(kPrologueLine), .replacement = (void *)HookedLine,
     .original = (void **)&g_line},
    {.module = "HGSIM.DLL", .rva = 0xF0205, .name = "VfxShapeDraw", .expect = kPrologueShape,
     .stolen = sizeof(kPrologueShape), .replacement = (void *)HookedShape,
     .original = (void **)&g_shape},
    {.module = "HGSIM.DLL", .rva = 0xF0727, .name = "VfxShapeDrawRaw",
     .expect = kProloguePushRawShape, .stolen = sizeof(kProloguePushRawShape),
     .replacement = (void *)HookedShapeRaw, .original = (void **)&g_shapeRaw},
    {.module = "HGSIM.DLL", .rva = 0xF0865, .name = "VfxPaneWipe", .expect = kPrologueWipe,
     .stolen = sizeof(kPrologueWipe), .replacement = (void *)HookedWipe,
     .original = (void **)&g_wipe},
    {.module = "HGSIM.DLL", .rva = 0xF1C93, .name = "VfxCharacterDraw", .expect = kPrologueChar,
     .stolen = sizeof(kPrologueChar), .replacement = (void *)HookedChar,
     .original = (void **)&g_char},
};

void HooksHud_OnSimLoaded(HMODULE sim)
{
    int count;

    g_api = NULL; /* HGVK.DLL is loaded again, maybe elsewhere, with every mission */
    if (ConfigHasWord("nohudhi", 0)) {
        DebugPrintf("HUD: full-resolution HUD left out by hgrc.cfg\n");
        return;
    }
    if (!VulkanRendererWanted())
        return;
    g_sim = (unsigned char *)sim;
    count = Hook_InstallModule(sim, "HGSIM.DLL", g_hooks, sizeof(g_hooks) / sizeof(g_hooks[0]));
    DebugPrintf("HUD: %d of %d drawing hooks installed\n", count,
                (int)(sizeof(g_hooks) / sizeof(g_hooks[0])));
}
