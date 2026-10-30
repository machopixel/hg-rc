#include "w13.h"
#include "w12.h"
#include "w8w.h"
#include "w9d.h"
#include "../simmem.h"
#include "../util/access.h"
#include "../util/w3o.h"
#include "../render/w2n.h"
#include "../unsorted_1004/fns.h"
#include "../unsorted_100b/w6g.h"
#include "../unsorted_100c/w2f.h"
#include "../unsorted_100c/w7q.h"
#include "../unsorted_100d/w8z.h"
#include "../unsorted_100f/w2e.h"
#include "../unsorted_100f/w2g.h"

/* The loader, sprintf, strlen and the HUD 2D library of the DLL (left original). */
#define W13M_LOAD SIM_FN(uint32_t(__cdecl *)(uint32_t, uint32_t, uint32_t), 0x100ba7d5)
#define W13M_SPRINTF SIM_FN(int(__cdecl *)(char *, const char *, ...), 0x100fc780)
#define W13M_STRLEN SIM_FN(uint32_t(__cdecl *)(const char *), 0x100fc8f0)
#define W13M_FONT_HEIGHT SIM_FN(int32_t(__cdecl *)(uint32_t), 0x100ecce0)
typedef void(__cdecl *W13mLineFn)(uint32_t, int32_t, int32_t, int32_t, int32_t, uint32_t, uint32_t);
typedef void(__cdecl *W13mTextFn)(uint32_t, int32_t, int32_t, uint32_t, const char *, uint32_t);

#define W13M_SURFACE 0x109e3e00u
#define W13M_CORNER 0x109e87f0u

static void W13M_Text(uint32_t surface, int32_t x, int32_t y, uint32_t font, const char *text)
{
    if (G32(0x101668e8) != 0)
        SIM_FN(W13mTextFn, 0x100f2f01)(surface, x, y, font, text, 0x109b9920);
    else if (G32(0x1017b940) != 0)
        SIM_FN(W13mTextFn, 0x100f1e10)(surface, x, y, font, text, 0x109b9920);
    else
        SIM_FN(W13mTextFn, 0x100ecea6)(surface, x, y, font, text, 0x109b9920);
}

static void W13M_Line(int32_t x1, int32_t y1, int32_t x2, int32_t y2)
{
    if (G32(0x1017b940) != 0)
        SIM_FN(W13mLineFn, 0x100ef727)(W13M_SURFACE, x1, y1, x2, y2, 0, (uint32_t)*SIM_PTR(uint16_t, 0x101659f8));
    else
        SIM_FN(W13mLineFn, 0x100e86a1)(W13M_SURFACE, x1, y1, x2, y2, 0, 0xc);
}

/* The waypoint's name line: the language text [0x101c4ed0] alone, or followed by the entry's name. */
static void W13M_NameText(char *text, uint32_t idx, uint32_t fmt)
{
    const char *name = SIM_PTR(const char, 0x109a8668 + idx * 0x54);

    if (W13M_STRLEN(name) == 0)
        W13M_SPRINTF(text, (const char *)(uintptr_t)G32(0x101c4ed0));
    else
        W13M_SPRINTF(text, SIM_PTR(const char, fmt), (const char *)(uintptr_t)G32(0x101c4ed0), name);
}

/* The range line: [0x1015e4c4] / 100 in metres, or in thousands above 1000 (compared before rounding to a float). */
static void W13M_RangeText(char *text, uint32_t fmt_k, uint32_t fmt_m)
{
    double q = (double)GFL(0x1015e4c4) / (double)GFL(0x1012b5b4);
    float range = (float)q;

    if (q > (double)GFL(0x1012b5b8))
        W13M_SPRINTF(text, SIM_PTR(const char, fmt_k), (const char *)(uintptr_t)G32(0x101c4ea4),
                     (double)range / *SIM_PTR(double, 0x1012b5c0));
    else
        W13M_SPRINTF(text, SIM_PTR(const char, fmt_m), (const char *)(uintptr_t)G32(0x101c4ea4), (double)range);
}

/* The waypoint marker on screen: the marker, the two texts in a box and a line to the point. */
static void W13M_Boxed(uint32_t idx, float px, float py, char *name)
{
    char text[0x100];
    uint32_t font;
    int32_t x, y, w, h, t;

    t = Render_Ftol((double)py);
    Sim_1006519d((uint32_t)Render_Ftol((double)px), (uint32_t)t, name);
    font = W13M_LOAD(G32(0x109b98c0), G32(0x1014cec4), 0);
    if (font == 0)
        return;
    x = Render_Ftol((double)px + (double)GFL(0x1012b5ac));
    y = Render_Ftol((double)py - (double)GFL(0x1012b5b0));
    *GBYTES(0x109b992e) = 0x1c;
    W13M_NameText(text, idx, 0x10144fbc);
    W13M_Text(W13M_SURFACE, x, y, font, text);
    w = Sim_100ccb0d(text, (void *)(uintptr_t)font) + 2;
    h = W13M_FONT_HEIGHT(font);
    *GBYTES(0x109b992e) = 0xf;
    W13M_RangeText(text, 0x10144fc4, 0x10144fd0);
    W13M_Text(W13M_SURFACE, x, y + h, font, text);
    if (w < Sim_100ccb0d(text, (void *)(uintptr_t)font) + 2)
        w = Sim_100ccb0d(text, (void *)(uintptr_t)font) + 2;
    h = h * 2 + 2;
    x -= 2;
    y -= 2;
    W13M_Line(x, y, x + w, y);
    W13M_Line(x + w, y, x + w, y + h);
    W13M_Line(x + w, y + h, x, y + h);
    W13M_Line(x, y + h, x, y);
    t = Render_Ftol((double)py) - 10;
    W13M_Line(x, y + h, Render_Ftol((double)px), t);
    *GBYTES(0x109b992e) = 9;
    Sim_100bb2b0((const uint32_t *)(uintptr_t)G32(0x109b98c0), (const char *)(uintptr_t)G32(0x1014cec4));
}

/* The waypoint off screen (or close to the aim point): the two texts in the corner. */
static void W13M_Corner(uint32_t idx)
{
    char text[0x100];
    uint32_t font = W13M_LOAD(G32(0x109b9900), G32(0x1014cec4), 0);

    if (font == 0)
        return;
    *GBYTES(0x109b992e) = 0xf;
    W13M_NameText(text, idx, 0x10144fdc);
    W13M_Text(W13M_CORNER, 0, 0, font, text);
    W13M_RangeText(text, 0x10144fe4, 0x10144ff0);
    W13M_Text(W13M_CORNER, 0, 0xf, font, text);
    *GBYTES(0x109b992e) = 9;
    Sim_100bb2b0((const uint32_t *)(uintptr_t)G32(0x109b9900), (const char *)(uintptr_t)G32(0x1014cec4));
}

// FUNCTION: HGSIM 0x10063529
void __cdecl Sim_10063529(void *holder)
{
    uint32_t obj = U32(holder, 0);
    uint32_t target = U32(obj, 0x118);
    uint32_t idx;

    if (target != 0 && (target & 0x1000) == 0) {
        char name[16]; /* the original's buffer size, as in Sim_10063037 */
        int32_t counter = GS32(0x10176954);
        float px, py, pz;

        idx = target & 0xff;
        G32(0x10176954) = (uint32_t)counter + 1;
        Str_Copy(name, (const char *)(uintptr_t)U32(GBYTES(0x109e857c), (counter % 7) * 4));
        px = FL(obj, 0x104);
        py = FL(obj, 0x108);
        pz = FL(obj, 0x10c);
        Sim_100ca996();
        if (G32(0x109a8640 + idx * 0x54) != 0) {
            if (Sim_100d2669(&px, &py, &pz) != 0 &&
                (Math_AbsInt_100fd100(Render_Ftol((double)px - (double)GS32(0x1017e580))) > 0x1e ||
                 Math_AbsInt_100fd100(Render_Ftol((double)py - (double)GS32(0x1017e584))) > 0x1e))
                W13M_Boxed(idx, px, py, name);
            else
                W13M_Corner(idx);
        }
    }
    target = U32(obj, 0xf0);
    if (target != 0 && (target & 0x1000) == 0) {
        idx = target & 0xff;
        target &= 0xf00;
        if (target == 0x200) {
            int kind = Sim_10052be7((int)idx);

            Sim_100648a0(BYTES(PTRTAB(idx)), kind);
        } else if (target == 0x400) {
            int kind = Sim_10052c02((int)idx);

            Sim_10064d05(idx, BYTES(Sim_100cba68()), kind);
        }
    }
    if (target != 0x100)
        Sim_10063037(holder);
}
