#include "w12.h"
#include "../simmem.h"
#include "../util/access.h"
#include "../util/util.h"
#include "../util/w3o.h"
#include "../render/w2n.h"
#include "../unsorted_1000/w3o.h"
#include "../unsorted_1004/fns.h"
#include "../unsorted_1008/w10.h"
#include "../unsorted_100b/w6g.h"
#include "../unsorted_100c/w2f.h"
#include "../unsorted_100c/w7q.h"
#include "../unsorted_100d/w8z.h"
#include "../unsorted_100f/w2e.h"

/* The loader, sprintf, strlen and the HUD 2D library of the DLL (left original). */
#define W12R_LOAD SIM_FN(uint32_t(__cdecl *)(uint32_t, uint32_t, uint32_t), 0x100ba7d5)
#define W12R_SPRINTF SIM_FN(int(__cdecl *)(char *, const char *, ...), 0x100fc780)
#define W12R_STRLEN SIM_FN(uint32_t(__cdecl *)(const char *), 0x100fc8f0)
#define W12R_FONT_HEIGHT SIM_FN(int32_t(__cdecl *)(uint32_t), 0x100ecce0)
typedef void(__cdecl *W12rLineFn)(uint32_t, int32_t, int32_t, int32_t, int32_t, uint32_t, uint32_t);
typedef void(__cdecl *W12rTextFn)(uint32_t, uint32_t, uint32_t, uint32_t, const char *, uint32_t);
typedef void(__cdecl *W12rPaneTextFn)(uint8_t *pane, const char *text);

typedef union {
    float f;
    uint32_t u;
} W12rBits;

static float W12R_F(uint32_t u)
{
    W12rBits b;

    b.u = u;
    return b.f;
}

static void W12R_PaneText(uint8_t *pane, uint32_t text)
{
    ((W12rPaneTextFn)(uintptr_t)U32(pane, 0x114))(pane, (const char *)(uintptr_t)text);
}

static void W12R_Line(int32_t x1, int32_t y1, int32_t x2, int32_t y2)
{
    if (G32(0x1017b940) != 0)
        SIM_FN(W12rLineFn, 0x100ef727)(0x109e3e00, x1, y1, x2, y2, 0, (uint32_t)*SIM_PTR(uint16_t, 0x101659f2));
    else
        SIM_FN(W12rLineFn, 0x100e86a1)(0x109e3e00, x1, y1, x2, y2, 0, 9);
}

/* the colour of a side: 0 -> 15, 1 -> 4, 2 -> 20, else unchanged */
static int32_t W12R_SideColour(int side, int32_t colour)
{
    return side == 0 ? 0xf : side == 1 ? 4 : side == 2 ? 0x14 : colour;
}

// FUNCTION: HGSIM 0x100c7120
void __cdecl Sim_100c7120(uint8_t *pane)
{
    char text[0x40];
    int32_t colour = 0xc, range, w, h = 0, t;
    int32_t rank = 0;                   /* start values (DECISIONS.md): other kinds, and 0x400 without its object */
    uint32_t pos[3] = {0, 0, 0}, size = 0;
    uint32_t rec, target, kind, idx, font;
    int onscreen;

    if (S16(pane, 4) == 0)
        return;
    rec = U32(PTRTAB(G32(0x1017b8c8)), 0x28);
    target = U32(U32(rec, 0), 0xf0);
    kind = target & 0xf00;
    idx = target & 0xff;
    if (kind == 0 || (target & 0x1000) != 0)
        return;
    if (G32(0x1017e58c) != target)
        G32(0x1017e588) = 0;
    if (G32(0x1017e5ac) == 2) {
        G32(0x1017e588) = G32(0x1015dc78) + 0x16a;
        W12R_PaneText(pane, G32(0x101c47c4));
    } else if (G32(0x1017e5ac) != 0) {
        G32(0x1017e588) = G32(0x1015dc78) + 0x16a;
        if (kind == 0x100)
            W12R_PaneText(pane, 0x109a867e + idx * 0x54);
        else if (kind == 0x200)
            W12R_PaneText(pane, PTRTAB(idx) + 0x13a);
        else if (kind == 0x400)
            W12R_PaneText(pane, 0x109e9bce + idx * 0x50);
        else
            W12R_PaneText(pane, 0x1017e594);
        if ((int8_t)U8(pane, 0x10) == 0)
            W12R_PaneText(pane, G32(0x101c47c0));
    }
    if (G32(0x1017e588) == 0 || (int32_t)G32(0x1015dc78) > (int32_t)G32(0x1017e588)) {
        G32(0x1017e588) = 0;
        if (kind == 0x100) {
            int16_t flags = *SIM_PTR(int16_t, 0x109a8664 + idx * 0x54);

            if ((flags & 0x20) == 0 && (flags & 0x100) != 0)
                W12R_PaneText(pane, G32(0x101c47b8));
            else if (W12R_STRLEN(SIM_PTR(const char, 0x109a8668 + idx * 0x54)) == 0)
                W12R_PaneText(pane, G32(0x101c47b4));
            else
                W12R_PaneText(pane, 0x109a8668 + idx * 0x54);
        } else if (kind == 0x400) {
            int16_t flags = *SIM_PTR(int16_t, 0x109e9ba0 + idx * 0x50);

            if ((flags & 0x20) == 0 && (flags & 0x100) != 0) {
                if (*SIM_PTR(int8_t, 0x1015da30) == 0)
                    Str_Copy(SIM_PTR(char, 0x1015da30), (const char *)(uintptr_t)G32(0x101c47b8));
                W12R_PaneText(pane, 0x1015da30);
            } else {
                if (W12R_STRLEN(SIM_PTR(const char, 0x109e9bb8 + idx * 0x50)) == 0)
                    W12R_PaneText(pane, G32(0x101c47d8));
                else
                    W12R_PaneText(pane, 0x109e9bb8 + idx * 0x50);
                if (G32(0x1017e570) != 0)
                    G32(0x1017e570) = 0;
            }
        } else if (kind == 0x200) {
            int16_t flags = S16(PTRTAB(idx), 0x1c);

            if ((flags & 0x20) == 0 && (flags & 0x100) != 0) {
                W12R_PaneText(pane, G32(0x101c47b8));
            } else {
                if (W12R_STRLEN((const char *)(uintptr_t)(PTRTAB(idx) + 0x124)) == 0)
                    W12R_PaneText(pane, G32(0x101c47d4));
                else
                    W12R_PaneText(pane, PTRTAB(idx) + 0x124);
                if (G32(0x1017e570) != 0)
                    G32(0x1017e570) = 0;
            }
        } else {
            W12R_PaneText(pane, 0x1017e598);
        }
    }
    if (kind == 0x400)
        colour = W12R_SideColour(Sim_10052c02((int)idx), colour);
    else if (kind == 0x200)
        colour = W12R_SideColour(Sim_10052be7((int)idx), colour);
    else if (kind == 0x100)
        colour = (*SIM_PTR(int16_t, 0x109a8664 + idx * 0x54) & 0x20) == 0 ? 0x18 : 0x1c;
    font = W12R_LOAD(G32(0x109b98c0), G32(0x1014cec4), 0);
    if (font == 0)
        return;
    range = Render_Ftol((double)FL(U32(rec, 0), 0xd8)) / 100;
    if (range > 1000) {
        W12R_SPRINTF(text, SIM_PTR(const char, 0x10151cfc), (double)range / (double)GFL(0x1012bbf0));
    } else {
        if (range < 0)
            range = 0;
        W12R_SPRINTF(text, SIM_PTR(const char, 0x10151d04), range);
    }
    if (kind == 0x200) {
        uint32_t u = PTRTAB(idx);

        pos[0] = U32(u, 0x58);
        pos[1] = U32(u, 0x5c);
        pos[2] = U32(u, 0x60);
        size = U32(U32(u, 0x28), 0x53c);
        rank = (int32_t)(U32(U32(u, 0x28), 0xbdf) - U32(rec, 0xbe3));
        if (rank > 4)
            rank = 4;
    } else if (kind == 0x400) {
        uint32_t obj;

        rank = -1;
        obj = Sim_100cba68();
        if (obj != 0) {
            W12rBits b;

            b.f = Sim_1000946e((void *)(uintptr_t)U32(obj, 0x8c), &pos[0], &pos[2 - 1], &pos[2]);
            size = b.u;
        }
    }
    onscreen = Sim_100d2669((float *)&pos[0], (float *)&pos[1], (float *)&pos[2]);
    G32(0x1017e580) = (uint32_t)Render_Ftol((double)W12R_F(pos[0]));
    G32(0x1017e584) = (uint32_t)Render_Ftol((double)W12R_F(pos[1]));
    if (onscreen != 0) {
        W12rBits s;
        int32_t x, y;

        s.f = (float)((double)FL(G32(0x1014cae0), 0x94) * (double)W12R_F(size) / (double)W12R_F(pos[2]));
        if ((double)(int32_t)G32(0x1015dc38) * (double)GFL(0x1012bbf4) > (double)s.f)
            s.f = (float)((double)(int32_t)G32(0x1015dc38) * (double)GFL(0x1012bbf4));
        size = s.u;
        G32(0x109e8754) = (uint32_t)Render_Ftol((double)s.f * (double)GFL(0x1012bbf8) + (double)W12R_F(pos[0]));
        G32(0x109e8758) = (uint32_t)Render_Ftol(((double)W12R_F(pos[1]) - (double)s.f * (double)GFL(0x1012bbf8)) -
                                                (double)(int32_t)(G32(0x101c64f0) + 1) * (double)GFL(0x1012bbfc));
        G32(0x109e875c) = G32(0x109e8754) + 0x8c;
        G32(0x109e8760) = G32(0x109e8758) + 0x3c;
        font = W12R_LOAD(G32(0x109b98c0), G32(0x1014cec4), 0);
        if (rank > 0 && G32(0x1014be71 + (uint32_t)rank * 0x1d) < (uint32_t)range) {
            w = Sim_100ccb0d((const char *)(uintptr_t)G32(0x101c47e4), (void *)(uintptr_t)font) + 2;
            t = Sim_100ccb0d((const char *)(uintptr_t)G32(0x101c47e4), (void *)(uintptr_t)font) + 2;
            if (w < t)
                w = Sim_100ccb0d((const char *)(uintptr_t)G32(0x101c47e4), (void *)(uintptr_t)font) + 2;
        } else {
            w = Sim_100ccb0d((const char *)(pane + 0x10), (void *)(uintptr_t)font) + 2;
            t = Sim_100ccb0d(text, (void *)(uintptr_t)font) + 2;
            if (w < t)
                w = Sim_100ccb0d(text, (void *)(uintptr_t)font) + 2;
        }
        h = W12R_FONT_HEIGHT(font) << 1;
        Sim_100bb2b0((const uint32_t *)(uintptr_t)G32(0x109b98c0), (const char *)(uintptr_t)G32(0x1014cec4));
        x = (int32_t)G32(0x109e8754);
        y = (int32_t)G32(0x109e8758);
        W12R_Line(x - 2, y - 2, x + w, y - 2);
        x = (int32_t)G32(0x109e8754);
        y = (int32_t)G32(0x109e8758);
        W12R_Line(x + w, y - 2, x + w, y + h);
        x = (int32_t)G32(0x109e8754);
        y = (int32_t)G32(0x109e8758);
        W12R_Line(x + w, y + h, x - 2, y + h);
        x = (int32_t)G32(0x109e8754);
        y = (int32_t)G32(0x109e8758);
        W12R_Line(x - 2, y + h, x - 2, y - 2);
        x = (int32_t)G32(0x109e8754);
        y = (int32_t)G32(0x109e8758);
        W12R_Line(x - 2, y + h, Render_Ftol((double)W12R_F(pos[0])),
                  Render_Ftol((double)W12R_F(pos[1])) - Render_Ftol((double)s.f / (double)GFL(0x1012bbf4)));
    } else {
        G32(0x109e8754) = (uint32_t)Render_Ftol((double)(int32_t)G32(0x109e3de8) * *SIM_PTR(const double, 0x1012bc00));
        G32(0x109e8758) = (uint32_t)Render_Ftol((double)(int32_t)G32(0x109e3da8) * *SIM_PTR(const double, 0x1012bc08));
        G32(0x109e875c) = G32(0x109e3de8);
        G32(0x109e8760) = G32(0x109e3da8);
    }
    if (G32(0x10179ca4) != 0 && Sim_1009c929(100) < 0x46) {
        Sim_1008dba2((const int32_t *)(uintptr_t)U32(pane, 0xec), SIM_PTR(const char, 0x1017e59c), (void *)(uintptr_t)font);
    } else if (rank <= 0 || G32(0x1014be71 + (uint32_t)rank * 0x1d) > (uint32_t)range) {
        W12rTextFn draw;

        U8(0x109b992e, 0) = (uint8_t)colour;
        if (G32(0x101668e8) != 0)
            draw = SIM_FN(W12rTextFn, 0x100f2f01);
        else if (G32(0x1017b940) != 0)
            draw = SIM_FN(W12rTextFn, 0x100f1e10);
        else
            draw = SIM_FN(W12rTextFn, 0x100ecea6);
        draw(U32(pane, 0xec), 0, 0, font, (const char *)(pane + 0x10), 0x109b9920);
        U8(0x109b992e, 0) = 9;
        U8(0x109b992e, 0) = 0xf;
        Sim_1008dba2((const int32_t *)(uintptr_t)U32(pane, 0xec), text, (void *)(uintptr_t)font);
        U8(0x109b992e, 0) = 9;
    } else {
        U8(0x109b992e, 0) = (uint8_t)colour;
        if (G32(0x1017b940) != 0)
            SIM_FN(W12rTextFn, 0x100f1e10)(U32(pane, 0xec), 0, 0, font, (const char *)(uintptr_t)G32(0x101c47e4),
                                           0x109b9920);
        else
            SIM_FN(W12rTextFn, 0x100ecea6)(U32(pane, 0xec), 0, 0, font, (const char *)(uintptr_t)G32(0x101c47e4),
                                           0x109b9920);
        U8(0x109b992e, 0) = 9;
    }
    Sim_100bb2b0((const uint32_t *)(uintptr_t)G32(0x109b98c0), (const char *)(uintptr_t)G32(0x1014cec4));
    G32(0x1017e58c) = U32(U32(rec, 0), 0xf0);
}