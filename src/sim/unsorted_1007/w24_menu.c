#include "w24.h"
#include "w2i.h"
#include "w2j.h"
#include "w9e.h"
#include "../util/access.h"
#include "../util/util.h"
#include "../view/view.h"
#include "../render/w2n.h"
#include "../unsorted_1000/w5x.h"
#include "../unsorted_1008/w8x.h"
#include "../unsorted_100f/w2e.h"

int32_t g_w24MenuKey;

#define W24M_FONT_HEIGHT SIM_FN(int32_t(__cdecl *)(void *), 0x100ecce0)
#define W24M_CHAR_WIDTH SIM_FN(int32_t(__cdecl *)(void *, uint32_t), 0x100eccf3)
#define W24M_SHAPE_ACCEL SIM_FN(void(__cdecl *)(uint32_t, uint32_t, int32_t, int32_t, int32_t), 0x100f0205)
#define W24M_SHAPE_SOFT SIM_FN(void(__cdecl *)(uint32_t, uint32_t, int32_t, int32_t, int32_t), 0x100e91dc)
#define W24M_ITOA SIM_FN(char *(__cdecl *)(int, char *, int), 0x100ffde0)
#define W24M_STRRCHR SIM_FN(char *(__cdecl *)(const char *, int), 0x100ffdb0)
typedef int(__cdecl *W24mTextFn)(uint32_t pane, int32_t x, int32_t y, uint32_t font, const char *text, void *pal);
typedef void(__cdecl *W24mItemFn)(uint8_t *s, uint32_t arg, int32_t row, int32_t x, int32_t y, uint8_t *menu);

#define W24M_ITEM(menu, i) ((menu) + 0x18 + (uint32_t)(i) * 0x14u)

/* one text line through the display's text routine */
static void W24m_Text(uint32_t pane, int32_t x, int32_t y, uint32_t font, const char *text)
{
    if (G32(0x101668e8) != 0)
        SIM_FN(W24mTextFn, 0x100f2f01)(pane, x, y, font, text, GBYTES(0x109b9920));
    else
        SIM_FN(W24mTextFn, 0x100f1e10)(pane, x, y, font, text, GBYTES(0x109b9920));
}

static int32_t W24m_Width(uint32_t font, uint32_t c)
{
    if (G32(0x101668e8) != 0)
        return (int32_t)Sim_100f2cec((void *)(uintptr_t)font, c);
    return W24M_CHAR_WIDTH((void *)(uintptr_t)font, c);
}

// FUNCTION: HGSIM 0x1007581a
void __cdecl Sim_1007581a(uint8_t *s)
{
    char line[0x104], number[0x24];
    uint8_t *menu, *item;
    uint32_t font, pane;
    int32_t sep = 0x10, hasSep = 0, step = 0, sel, last = -1, code, digit, count, i;
    int32_t titleX = 0, titleY = 0;     /* unset in the original without a title, which no menu has (round 24) */
    int32_t x, y, ix, iy, cx, cy, gap, num, height, numWidth, width, savedX, savedY;
    const char *label;

    /* the key code the original finds in its [ebp-0x128] when this call does not store one (round 24) */
    code = g_w24MenuKey;
    /* The wrap's strrchr reads past its copy for a label of width + 2 or more characters, into what the earlier labels
       of this call (or, for the first one, the stack) left in the line: kept within the call, zero at its start. No
       shipped label is that long (longest 25 against widths 25 / 27; DECISIONS.md, round 24). */
    Mem_Fill(line, 0, sizeof(line));
    if (s == 0)
        return;
    font = U32(s, 0x48);
    if (font == 0)
        return;
    menu = BYTES(Sim_100765f6(s));
    if (menu == 0)
        return;
    pane = U32(s, 0);
    if (pane == 0)
        return;
    sel = S32(menu, 0x10);
    count = S32(menu, 0xc);
    for (i = 0; i < count; i++) {
        if (S32(W24M_ITEM(menu, i), 0) == 2 || S32(W24M_ITEM(menu, i), 0) == 6)
            last = i;
        if (S32(W24M_ITEM(menu, i), 0) == 3) {
            sep = i;
            hasSep = 1;
        }
    }
    if (last == -1)
        hasSep = 0;
    if (S32(menu, 0) == 0) {
        U32(menu, 0) = 1;
    } else {
        if (S32(menu, 0) == 1)
            U32(menu, 0) = 2;
        if (G32(0x1015dc18) != 0) {
            code = GS32(0x1015dc18);
            g_w24MenuKey = code;
            if (GS32(0x1015dc18) >= 0x30 && GS32(0x1015dc18) <= 0x39) {
                digit = GS32(0x1015dc18) - 0x30;
                if (digit == 0) {
                    if (last != -1) {
                        sel = last;
                        code = 0xd;
                        g_w24MenuKey = code;
                    }
                } else if (digit >= 1 && digit < S32(menu, 0xc) - hasSep) {
                    if (digit <= sep) {
                        sel = digit - 1;
                    } else {
                        sel = digit;
                        GS32(0x1015dc18) += 1;
                        if (GS32(0x1015dc18) > 0x39)
                            G32(0x1015dc18) = 0x30;
                    }
                    code = 0xd;
                    g_w24MenuKey = code;
                }
            }
            Sim_10075753(code, U32(W24M_ITEM(menu, sel), 0), (uint32_t *)menu, (uint32_t *)&step);
            if (step != 0)
                sel = (sel + step + S32(menu, 0xc)) % S32(menu, 0xc);
            if (sel != S32(menu, 0x10) && U8(s, 0x34) != 0)
                Sim_1000ed52(0, 0, 0, (uint32_t)(uintptr_t)(s + 0x34), 0, 0);
        }
    }
    if (step == 0)
        step = 1;
    if (S32(W24M_ITEM(menu, sel), 0) == 3) {
        sel = (sel + step + S32(menu, 0xc)) % S32(menu, 0xc);
        if (GS32(0x1015dc18) >= 0x30 && GS32(0x1015dc18) <= 0x39) {
            GS32(0x1015dc18) += 1;
            if (GS32(0x1015dc18) > 0x39)
                G32(0x1015dc18) = 0x30;
#ifdef W24M_MUTATE /* the checker's negative check only */
            code = 0;
#endif
            Sim_10075753(code, U32(W24M_ITEM(menu, sel), 0), (uint32_t *)menu, (uint32_t *)&step);
            if (step == 0)
                step = 1;
        }
    }
    U32(menu, 0x10) = (uint32_t)sel;
    *GBYTES(0x109b992e) = U8(s, 0x4c);
    if (U32(menu, 4) != 0) {
        titleX = S32(s, 0x60);
        titleY = S32(s, 0x64);
        W24m_Text(pane, titleX, titleY, font, (const char *)BYTES(U32(menu, 4)));
    }
    if ((U32(s, 4) & 4) != 0)
        Sim_1008d65d((void *)(uintptr_t)pane, (uint32_t)titleX, titleY, (void *)(uintptr_t)font, 0x1c);
    else
        Sim_1008d6f7((void *)(uintptr_t)pane, BYTES(U32(menu, 4)), titleX, titleY, (void *)(uintptr_t)font, 0x1c);
    height = W24M_FONT_HEIGHT((void *)(uintptr_t)font);
    cx = S32(s, 0x68);
    cy = S32(s, 0x6c) + height / 2;
    x = S32(s, 0x70);
    y = S32(s, 0x74);
    ix = S32(s, 0x78);
    iy = S32(s, 0x7c);
    numWidth = W24m_Width(font, 0x30) * 2;
    numWidth += W24m_Width(font, 0x2e);
    num = 0;
    gap = 0;
    for (i = 0; i < S32(menu, 0xc); i++) {
        item = W24M_ITEM(menu, i);
        *GBYTES(0x109b992e) = i == sel ? U8(s, 0x50) : U8(s, 0x4c);
        if ((U32(s, 4) & 8) == 0 && i == last)
            gap += (S32(s, 0x54) - i - 1) * S32(s, 0x5c);
        y += gap;
        iy += gap;
        cy += gap;
        gap = S32(s, 0x5c);
        if (i == sel && U32(s, 0x20) != 0) {
            W24M_SHAPE_ACCEL(pane, U32(s, 0x20), 0, cx, cy);
        }
        if (S32(item, 0) != 3) {
            if (i == last)
                num = 0;
            else
                num++;
            W24M_ITOA(num, number, 10);
            W24m_Text(pane, x, y, font, number);
        }
        height = W24M_FONT_HEIGHT((void *)(uintptr_t)font);
        width = GS32(0x10149950 + G32(0x101c64f0) * 4u);
        label = (const char *)BYTES(U32(item, 4));
        if (label != 0) {
            if (Str_Length(label) < (uint32_t)width) {
                W24m_Text(pane, x + numWidth, y, font, label);
            } else {
                savedX = x;
                savedY = y;
                Str_CopyN(line, label, (uint32_t)width + 2u);
                if (line[width + 1] != ' ')
                    width = (int32_t)(W24M_STRRCHR(line, ' ') - line);
                Str_CopyN(line, label, (uint32_t)width + 1u);
                line[width + 1] = 0;
                W24m_Text(pane, x + numWidth, y, font, line);
                x = savedX;
                y = Render_Ftol((double)y + (double)height / *SIM_PTR(double, 0x1012b7f8));
                Str_Copy(line, label + width + 1);
                W24m_Text(pane, x + numWidth, y, font, line);
                x = savedX;
                y = savedY;
            }
        }
        if (U32(item, 8) != 0)
            ((W24mItemFn)(uintptr_t)U32(item, 8))(s, U32(item, 0xc), i, ix, iy, menu);
    }
    *GBYTES(0x109b992e) = 9;
    switch (S32(menu, 0)) {
    case 3:
        U32(menu, 0) = 2;
        if (U32(W24M_ITEM(menu, sel), 0x10) != 0) {
            Sim_10076624(s, S32(W24M_ITEM(menu, sel), 0x10));
            if (U8(s, 0x24) != 0)
                Sim_1000ed52(0, 0, 0, (uint32_t)(uintptr_t)(s + 0x24), 0, 0);
        }
        break;
    case 4:
    case 5:
        U32(menu, 0) = 0;
        U32(menu, 0x10) = 0;
        Sim_100765a7(s);
        break;
    default:
        break;
    }
}
