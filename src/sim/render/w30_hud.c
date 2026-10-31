#include <string.h>

#include "w30.h"
#include "../util/access.h"
#include "../../plugins/hgvk/hud_api.h"

/*
 * The HUD at full resolution (docs/renderer/hud.md), moved here from hooks_hud.c in round 30: the drawing
 * library's entry points below run the converted routine and, when the pane draws into the renderer's HUD buffer,
 * let it draw into a scratch buffer instead and hand the same drawing to the renderer (HGVK.DLL's HudApi). The
 * hook layer supplies the interface and the scratch memory (g_w30HudApi, g_w30Scratch); without them (no HGVK,
 * "nohudhi", the checker) each entry point is just the routine.
 */
const HudApi *(*g_w30HudApi)(void);
uint16_t *(*g_w30Scratch)(uint32_t bytes);
uint16_t *(*g_w30Cells)(uint32_t cells);

static const HudApi *g_api;
static uint16_t *g_scratch;

typedef struct {
    uint32_t *window;  /* {buffer, row pitch in bytes - 1, height - 1} */
    int32_t originX, originY;
    HudRect clip;
} W30Pane;

/* True when the pane draws into the renderer's HUD buffer (a pane inside a diverted call does not match). */
static int W30_Match(const void *pane, W30Pane *info)
{
    uint32_t *window;
    const void *target;
    int32_t w, h;

    g_api = g_w30HudApi != 0 ? g_w30HudApi() : 0;
    if (g_api == 0 || pane == 0 || (window = (uint32_t *)(uintptr_t)U32(pane, 0)) == 0)
        return 0;
    target = g_api->Target();
    if (target == 0 || window[0] != (uint32_t)(uintptr_t)target)
        return 0;
    /* the second field is the row pitch in bytes - 1; the height follows the window (1439 at 1440 lines) */
    w = (int32_t)window[1] + 1;
    h = (int32_t)window[2] + 1;
    if (w != HUD_CELLS_W * 2 || h < HUD_CELLS_H)
        return 0;
    g_scratch = g_w30Scratch((uint32_t)w * (uint32_t)h);
    if (g_scratch == 0)
        return 0;
    info->window = window;
    info->originX = S32(pane, 4);
    info->originY = S32(pane, 8);
    info->clip.x0 = S32(pane, 4) > 0 ? S32(pane, 4) : 0;
    info->clip.y0 = S32(pane, 8) > 0 ? S32(pane, 8) : 0;
    info->clip.x1 = S32(pane, 0xc) < HUD_CELLS_W - 1 ? S32(pane, 0xc) : HUD_CELLS_W - 1;
    info->clip.y1 = S32(pane, 0x10) < HUD_CELLS_H - 1 ? S32(pane, 0x10) : HUD_CELLS_H - 1;
    return info->clip.x1 >= info->clip.x0 && info->clip.y1 >= info->clip.y0;
}

static void W30_Divert(const W30Pane *info)
{
    info->window[0] = (uint32_t)(uintptr_t)g_scratch;
}

static void W30_Undivert(const W30Pane *info)
{
    info->window[0] = (uint32_t)(uintptr_t)g_api->Target();
}

/* a bitmap buffer for the renderer (the hook layer's memory) */
static uint16_t *W30_Bitmap(uint32_t cells)
{
    return g_w30Cells(cells);
}

/* a shape's run-length rows decoded into RGB565 cells (0 transparent) and handed over */
static void W30_HiShape(const W30Pane *info, const uint8_t *shape, int32_t x, int32_t y)
{
    int32_t x0 = S32(shape, 8), y0 = S32(shape, 0xc), w = S32(shape, 0x10) - x0 + 1, h = S32(shape, 0x14) - y0 + 1;
    const uint8_t *p = shape + 0x18;
    uint16_t *bmp;
    int32_t row, col;

    if (w <= 0 || h <= 0 || w > 2048 || h > 2048 || (bmp = W30_Bitmap((uint32_t)(w * h))) == 0)
        return;
    memset(bmp, 0, (size_t)(w * h) * sizeof(uint16_t));
    for (row = 0; row < h; row++) {
        col = 0;
        for (;;) {
            uint32_t code = *p++, n;

            if (code == 0)
                break;
            if (code == 1) {
                col += *p++;
                continue;
            }
            n = code >> 1;
            if (code & 1) {
                for (; n > 0; n--, col++, p++)
                    if (col >= 0 && col < w)
                        bmp[row * w + col] = SIM_PTR(u16a, 0x10156dcc)[*p];
            } else {
                uint16_t v = SIM_PTR(u16a, 0x10156dcc)[*p++];

                for (; n > 0; n--, col++)
                    if (col >= 0 && col < w)
                        bmp[row * w + col] = v;
            }
        }
    }
    g_api->Bitmap(&info->clip, x + x0, y + y0, w, h, bmp);
}

/* a glyph's bytes through the lookaside (0xff transparent) and the colour table, handed over */
static void W30_HiGlyph(const W30Pane *info, int32_t x, int32_t y, const uint8_t *font, int32_t code,
                        const uint8_t *lookaside)
{
    const uint8_t *glyph = font + U32(font, 0x10 + (uint32_t)code * 4);
    int32_t w = S32(glyph, 0), h = S32(font, 8), i;
    const uint8_t *src = glyph + 4;
    uint16_t *bmp;

    if (w <= 0 || h <= 0 || w > 1024 || h > 1024 || (bmp = W30_Bitmap((uint32_t)(w * h))) == 0)
        return;
    for (i = 0; i < w * h; i++) {
        uint32_t k = lookaside[src[i]];

        bmp[i] = k == 0xff ? 0 : SIM_PTR(u16a, 0x10156dcc)[k];
    }
    g_api->Bitmap(&info->clip, x, y, w, h, bmp);
}

// FUNCTION: HGSIM 0x100ef727
int32_t __cdecl W30_Line(void *pane, int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t mode, uint32_t color)
{
    W30Pane info;
    int32_t r, bx0, by0, bx1, by1, x, y, w, h;
    uint16_t *bmp;

    if (mode != 0 || !W30_Match(pane, &info))
        return Sim_100ef727(pane, x0, y0, x1, y1, mode, color);
    /* the cells come from the routine itself on the cleared scratch, so slanted lines look as the original */
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
    if (w <= 0 || h <= 0 || (color & 0xffff) == 0) {
        W30_Divert(&info);
        r = Sim_100ef727(pane, x0, y0, x1, y1, mode, color);
        W30_Undivert(&info);
        return r;
    }
    for (y = by0; y <= by1; y++)
        memset(&g_scratch[y * HUD_CELLS_W + bx0], 0, (size_t)w * sizeof(uint16_t));
    W30_Divert(&info);
    r = Sim_100ef727(pane, x0, y0, x1, y1, mode, color);
    W30_Undivert(&info);
    if ((r == 0 || r == 1) && (bmp = W30_Bitmap((uint32_t)(w * h))) != 0) {
        for (y = 0; y < h; y++)
            for (x = 0; x < w; x++)
                bmp[y * w + x] = g_scratch[(by0 + y) * HUD_CELLS_W + bx0 + x];
        g_api->Bitmap(&info.clip, bx0, by0, w, h, bmp);
    }
    return r;
}

// FUNCTION: HGSIM 0x100f0205
int32_t __cdecl W30_Shape(void *pane, const uint8_t *table, int32_t number, int32_t x, int32_t y)
{
    W30Pane info;
    int32_t r;

    if (!W30_Match(pane, &info))
        return Sim_100f0205(pane, table, number, x, y);
    W30_Divert(&info);
    r = Sim_100f0205(pane, table, number, x, y);
    W30_Undivert(&info);
    if (r == 0)
        W30_HiShape(&info, table + U32(table, 8 + (uint32_t)number * 8), x + info.originX, y + info.originY);
    return r;
}

// FUNCTION: HGSIM 0x100f0727
int32_t __cdecl W30_ShapeRaw(void *pane, const uint8_t *shape, int32_t x, int32_t y, int32_t slot)
{
    W30Pane info;
    int32_t r;

    if (!W30_Match(pane, &info))
        return Sim_100f0727(pane, shape, x, y, slot);
    W30_Divert(&info);
    r = Sim_100f0727(pane, shape, x, y, slot);
    W30_Undivert(&info);
    if (r == 0)
        W30_HiShape(&info, shape, x + info.originX, y + info.originY);
    return r;
}

// FUNCTION: HGSIM 0x100f0865
int32_t __cdecl W30_Wipe(void *pane, uint32_t color)
{
    W30Pane info;
    int32_t r;

    if (!W30_Match(pane, &info))
        return Sim_100f0865(pane, color);
    W30_Divert(&info);
    r = Sim_100f0865(pane, color);
    W30_Undivert(&info);
    if (r == 0)
        g_api->Fill(&info.clip, color & 0xffff);
    return r;
}

// FUNCTION: HGSIM 0x100f1c93
int32_t __cdecl W30_Char(void *pane, int32_t x, int32_t y, const uint8_t *font, int32_t code,
                         const uint8_t *lookaside)
{
    W30Pane info;
    int32_t r;

    if (!W30_Match(pane, &info))
        return Sim_100f1c93(pane, x, y, font, code, lookaside);
    W30_Divert(&info);
    r = Sim_100f1c93(pane, x, y, font, code, lookaside);
    W30_Undivert(&info);
    if (r >= 0)
        W30_HiGlyph(&info, x + info.originX, y + info.originY, font, code, lookaside);
    return r;
}
