#include "w3t.h"
#include "../util/w3t.h"
#include "../view/view.h"
#include "../unsorted_1008/w3t.h"

#define DISPLAY_RECT 0x109e3e20u

static void ScaleChild(int mode, int32_t *rect, const int32_t *area)
{
    U32(rect, 0) = (uint32_t)(uintptr_t)GBYTES(DISPLAY_RECT);
    if (mode == 4) {
        Sim_1008ce40(rect, FL32(G32(0x1014cae0), 0x44));
        View_CenterRect(GBYTES(DISPLAY_RECT), rect, rect);
    } else {
        Rect_ScaleToRect(area, rect, rect);
    }
}

// FUNCTION: HGSIM 0x100996c0
void __cdecl Sim_100996c0(int mode, void *view)
{
    int32_t *rect;
    uint32_t child;
    int i;

    if (view == 0)
        return;
    rect = (int32_t *)BYTES(U32(view, 0));
    G32(0x1015d1ac) = (uint32_t)mode;
    if (GS32(0x1015d1ac) == 1) {
        for (i = 0; i < 5; i++)
            rect[i] = GS32(0x109e8660 + i * 4);
    } else {
        rect[0] = (int32_t)(uintptr_t)GBYTES(DISPLAY_RECT);
        View_ScaleRect(GBYTES(DISPLAY_RECT), rect, rect);
    }
    if (mode == 1) {
        for (i = 0; i < 5; i++)
            GS32(0x109e8660 + i * 4) = rect[i];
    }
    for (i = 0; i < 5; i++)
        S32(GBYTES(0x10185400u + U32(view, 8) * 0x14u), i * 4) = rect[i];
    Rect_ScalePoint(rect, (int32_t *)(BYTES(view) + 0x58), (int32_t *)(BYTES(view) + 0x58));
    Rect_ScalePoint(rect, (int32_t *)(BYTES(view) + 0x60), (int32_t *)(BYTES(view) + 0x60));
    Rect_ScalePoint(rect, (int32_t *)(BYTES(view) + 0x68), (int32_t *)(BYTES(view) + 0x68));
    child = U32(view, 0x14);
    if (child != 0) {
        uint32_t table = U32(child, 4);
        int32_t *r = (int32_t *)BYTES(U32(table, 4));

        ScaleChild(mode, r, rect);
        r = (int32_t *)BYTES(U32(table, 8));
        ScaleChild(mode, r, rect);
        r = (int32_t *)BYTES(U32(table, 0xc));
        U32(r, 0) = (uint32_t)(uintptr_t)GBYTES(DISPLAY_RECT);
    }
    for (i = 0; i < 4; i++)
        U32(view, 0x7c + i * 4) = G32(0x1014bf08u + U32(view, 0x7c + i * 4) * 4u);
    Sim_1009b3a1(0);
}

// FUNCTION: HGSIM 0x10099928
void __cdecl Sim_10099928(void)
{
    int i;
    int32_t *point;

    for (i = 0; i < 6; i++)
        Sim_100996c0(i, BYTES(G32(0x1014c4c8u + (uint32_t)i * 4u)));
    G32(0x1015d1ac) = 0;
    G32(0x1015d1cc) = 1;
    G32(0x1015d098) = 0;
    point = (int32_t *)(BYTES(G32(0x1014c06c)) + 0x14);
    Rect_ScalePoint((const int32_t *)GBYTES(0x109e3e00), point, point);
    GS32(0x1014bf00) = (int32_t)((uint32_t)GS32(0x1014befc) - G32(0x1017cc58));
}

// FUNCTION: HGSIM 0x1009b3a1
void __cdecl Sim_1009b3a1(int mode)
{
    uint32_t view = G32(0x1014c4c8u + G32(0x1015d1ac) * 4u);
    int32_t quotient;

    if (view == 0)
        return;
    if (U8(G32(0x1017b8c4), 9) == 0)
        return;
    if (GS32(0x1015d1ac) == 4)
        return;
    if (mode == 0) {
        S32(view, 0x18) = S32(view, 0x20);
    } else if (mode == 1) {
        S32(view, 0x18) = S32(view, 0x18) / 2;
        if (S32(view, 0x18) < S32(view, 0x24))
            S32(view, 0x18) = S32(view, 0x28);
    } else if (mode == 2) {
        U32(view, 0x18) = U32(view, 0x18) << 1;
        if (S32(view, 0x18) > S32(view, 0x28))
            S32(view, 0x18) = S32(view, 0x24);
    }
    quotient = S32(view, 0x20) / S32(view, 0x18);
    FL32(view, 0x2c) = (float)quotient;
}
