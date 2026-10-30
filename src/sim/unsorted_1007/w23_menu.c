#include "w23.h"
#include "../util/access.h"

typedef int32_t(__cdecl *W23mGetFn)(uint32_t ctx);
typedef void(__cdecl *W23mCtxFn)(uint32_t ctx);
typedef void(__cdecl *W23mIndexFn)(uint32_t ctx, int32_t index);
typedef void(__cdecl *W23mSetupFn)(void *menu, void *item);
typedef int(__cdecl *W23mTextFn)(uint32_t surface, int32_t x, int32_t y, uint32_t font, uint32_t text, void *pal);

#define W23M_FN(t, item, off) ((t)(uintptr_t)U32(item, off))

// FUNCTION: HGSIM 0x1007760b
void __cdecl Sim_1007760b(void *panel, void *item, int32_t row, int32_t x, int32_t y, void *menu)
{
    int32_t delta = 0, index, state, count;
    int changing = 1, digit, selected;
    uint32_t surface, text, font;

    if (panel == 0 || U32(panel, 0x48) == 0 || menu == 0)
        return;
    state = S32(menu, 0);
    surface = U32(panel, 0);
    if (surface == 0 || item == 0 || U32(item, 8) == 0)
        return;
    digit = row == GS32(0x1015dc18) - 0x31;
    if ((U32(item, 0) & 1) != 0 && U32(item, 0x14) != 0)
        index = W23M_FN(W23mGetFn, item, 0x14)(U32(item, 0xc));
    else
        index = S32(item, 4);
    selected = row == S32(menu, 0x10);
    switch (state) {
    case 1:
        if (U32(item, 0x10) != 0)
            W23M_FN(W23mSetupFn, item, 0x10)(menu, item);
        if (U32(item, 0x14) != 0) {
            U32(item, 4) = (uint32_t)W23M_FN(W23mGetFn, item, 0x14)(U32(item, 0xc));
            index = S32(item, 4);
        } else {
            U32(item, 4) = 0;
            index = 0;
        }
        break;
    case 2:
        if (selected) {
            switch (G32(0x1015dc18)) {
            case 0x20:
            case 0xc8:
                delta = 1;
                break;
            case 0xc9:
#ifdef W23M_MUTATE /* the checker's negative check only */
                delta = 1;
#else
                delta = -1;
#endif
                break;
            default:
                if (digit)
                    delta = 1;
                break;
            }
        }
        break;
    case 4:
        if (U32(item, 0x1c) != 0)
            W23M_FN(W23mIndexFn, item, 0x1c)(U32(item, 0xc), index);
        changing = 0;
        break;
    case 5:
        if (U32(item, 0x20) != 0)
            W23M_FN(W23mCtxFn, item, 0x20)(U32(item, 0xc));
        changing = 0;
        break;
    default:
        break;
    }
    count = S32(U32(item, 8), 4);
    if (count > 0) {
        index = (index + delta + count) % count;
        text = U32(U32(item, 8), 8 + index * 4);
        font = U32(panel, 0x48);
        if (G32(0x101668e8) != 0)
            SIM_FN(W23mTextFn, 0x100f2f01)(surface, x, y, font, text, GBYTES(0x109b9920));
        else if (G32(0x1017b940) != 0)
            SIM_FN(W23mTextFn, 0x100f1e10)(surface, x, y, font, text, GBYTES(0x109b9920));
        else
            SIM_FN(W23mTextFn, 0x100ecea6)(surface, x, y, font, text, GBYTES(0x109b9920));
    }
    if (selected) {
        if (((delta != 0 && S32(menu, 0x18 + row * 0x14) == 4) || (digit && S32(menu, 0x18 + row * 0x14) == 5)) &&
            U32(item, 0x1c) != 0)
            W23M_FN(W23mIndexFn, item, 0x1c)(U32(item, 0xc), index);
        if (U32(item, 0x18) != 0 && changing)
            W23M_FN(W23mIndexFn, item, 0x18)(U32(item, 0xc), index);
    }
    U32(item, 4) = (uint32_t)index;
}

// FUNCTION: HGSIM 0x1007792c
void __cdecl Sim_1007792c(void *panel, void *item, int32_t row, int32_t x, int32_t y, void *menu)
{
    uint8_t *rec, *pane;
    int32_t state;
    uint32_t surface, font, text;

    (void)row;
    (void)x;
    (void)y;
    if (panel == 0 || menu == 0)
        return;
    state = S32(menu, 0);
    surface = U32(panel, 0);
    if (surface == 0)
        return;
    font = U32(panel, 0x48);
    if (font == 0 || item == 0)
        return;
    rec = BYTES(U32(item, 8));
    if (rec == 0)
        return;
    pane = BYTES(U32(rec, 0));
    if (pane == 0)
        return;
    text = U32(rec, 4);
    if (text == 0)
        return;
    if (U32(pane, 0) == 0) {
        U32(pane, 0) = (uint32_t)(uintptr_t)GBYTES(0x109e3e20);
        SIM_FN(void(__cdecl *)(uint32_t, void *, void *), 0x1008cafb)(surface, pane, pane);
    }
    SIM_FN(void(__cdecl *)(void *, uint32_t, uint32_t), 0x1008dba2)(pane, text, font);
#ifdef W23T_MUTATE /* the checker's negative check only */
    if (U32(item, 0x18) != 0 && state != 4)
#else
    if (U32(item, 0x18) != 0 && state != 4 && state != 5)
#endif
        W23M_FN(W23mIndexFn, item, 0x18)(U32(item, 0xc), 0);
}
