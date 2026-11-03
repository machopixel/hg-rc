#include "w8x.h"
#include "w6f.h"
#include "../unsorted_1004/fns.h"
#include "../unsorted_1008/w5a.h"
#include "../unsorted_1008/w6f.h"
#include "../util/access.h"
#include "../util/util.h"
#include "../util/w3o.h"
#include "../view/view.h"

#define W8X_ADDR(a) ((uint32_t)(uintptr_t)SIM_PTR(uint8_t, (a)))
#define CAMERA BYTES(G32(0x1014cae0))
#define VIEW_OF(mode) (SIM_PTR(u32a, 0x1014c4c8)[(mode)])

typedef int(__cdecl *W8xDriverFn)(void);
typedef void(__cdecl *W8xViewEndFn)(uint32_t pane, uint32_t value);

// FUNCTION: HGSIM 0x100993a3
int32_t __cdecl Sim_100993a3(uint32_t a, uint32_t b, uint32_t c)
{
    int32_t r = W8X9_CALL_10098f8c(a, b, c, 0xf, 0);

    if (r > 0x3c)
        W8X9_CALL_100d3510(0x11, 0xb5, 1);
    return r;
}

// FUNCTION: HGSIM 0x10099c36
void __cdecl Sim_10099c36(void)
{
    Sim_100999fc();
    if (S16(G32(0x109e88e0), 6) != 0)
        Sim_1009bba2();
    else
        Sim_10099c59();
}

/*
 * The part Sim_10099c59 and Sim_10099ef3 share: the pane copied to the view slot, the camera position and
 * heading, the render set-up. Returns the needle angle and the player's record.
 */
static float W8x_ViewSetup(uint32_t view, uint32_t pane, uint32_t *me, int32_t *slotOut)
{
    uint32_t pos[6];
    uint32_t dst, src = pane;
    int32_t slot, mode, n, depth;
    float h, v;
    int k;

    *me = PTRTAB(G32(0x1017b8c8));
    slot = S32(view, 8);
    *slotOut = slot;
    dst = W8X_ADDR(0x10185400) + (uint32_t)slot * 0x14;
    for (k = 0; k < 5; k++)
        U32(dst, 4 * k) = U32(src, 4 * k);
    h = FL(U32(*me, 0x28), 0xc);
    v = FL(*me, 0x68);
    pos[4] = 0x42b40000;
    pos[5] = 0;
    mode = GS32(0x1015d1ac);
    if (mode > 0 && mode <= 2) {
        pos[3] = Bits_OfFloat(v);
    } else if (mode == 4) {
        pos[3] = 0;
        h = (float)((double)h + (double)v);
    } else {
        pos[3] = 0;
        h = Float_OfBits(0);
    }
    if (G32(0x1015da24) == 0) {
        uint32_t a = 0, b = 0, c = 0;
        uint32_t node = U32(*me, 0x4c);

        if (node != 0)
            Sim_100a9cfb(BYTES(node), &a, &b, &c);
        pos[0] = a;
        pos[2] = c;
    } else {
        pos[0] = U32(G32(0x1014cae0), 0);
        pos[2] = U32(G32(0x1014cae0), 8);
    }
    n = S32(view, 0x18);
    pos[1] = Bits_OfFloat((float)n);
    depth = GS32(0x1017cc58) < 0 ? (int32_t)((uint32_t)n - G32(0x1017cc58)) : n;
    Sim_1008fb20(pos, slot, n, depth);
    return h;
}

/* The display routines of the scene pass and the level-of-detail update between the two state switches. */
static void W8x_SceneSetup(void)
{
    int s;

    G32(0x1014cb40) = W8X_ADDR(0x10001f8c);
    G32(0x1014cb48) = W8X_ADDR(0x1000144c);
    G32(0x1014cb4c) = W8X_ADDR(0x10002cc5);
    s = Sim_100448ff();
    Sim_100448cd(6);
    W8X9_CALL_1003b534();
    Sim_100448cd(s);
}

// FUNCTION: HGSIM 0x10099c59
void __cdecl Sim_10099c59(void)
{
    uint32_t view, me, pane;
    int32_t slot;
    float h;

    if (G32(0x1015d1ac) == 0)
        return;
    view = VIEW_OF(G32(0x1015d1ac));
    if (view == 0)
        return;
    pane = U32(view, 0);
    G32(0x1017cca0) = 0;
    h = W8x_ViewSetup(view, pane, &me, &slot);
    if (G32(0x1015d1ac) == 4) {
        W8x_SceneSetup();
        Sim_1008fce3(0);
        if (G32(0x1017b920) != 0)
            (*(W8xDriverFn *)(BYTES(G32(0x1015e688)) + 0x20))();
        G32(0x1017b948) = 0;
    }
    if (G32(0x1015d1ac) != 4 || (S16(me, 0x1c) & 0x16) == 0)
        Sim_1009ace7(BYTES(view), h);
    W8X9_CALL_1009a137(BYTES(view));
    Sim_1008fd5a();
    if (U32(view, 0x7c) != 0)
        (*(W8xViewEndFn *)(BYTES(view) + 0x7c))(pane, U32(U32(view, 0x74), 0x30));
    if (G32(0x1015d1c8) == 4 && (G32(0x1015d1ac) != 4 || GS32(0x1017cc50) <= 1))
        W8X9_CALL_1009b03d(BYTES(view));
}

// FUNCTION: HGSIM 0x10099ef3
void __cdecl Sim_10099ef3(void *pane)
{
    uint32_t view, me, saved = G32(0x1015d1ac);
    int32_t slot;
    float h;

    G32(0x1015d1ac) = 4;
    view = VIEW_OF(G32(0x1015d1ac));
    if (view == 0)
        return;
    G32(0x1017cca0) = (uint32_t)(uintptr_t)pane;
    h = W8x_ViewSetup(view, (uint32_t)(uintptr_t)pane, &me, &slot);
    if (G32(0x1015d1ac) == 4) {
        W8x_SceneSetup();
        W8X9_CALL_100eb070(pane, G32(0x10138c78));
        Sim_1008fce3(0);
    }
    if (G32(0x1015d1ac) != 4 || (S16(me, 0x1c) & 0x16) == 0)
        Sim_1009ae96(pane, BYTES(view), h);
    W8X9_CALL_1009a137(BYTES(view));
    Sim_1008fd5a();
    G32(0x1015d1ac) = saved;
    G32(0x1017cca0) = 0;
}

// FUNCTION: HGSIM 0x1009ba57
void __cdecl Sim_1009ba57(void)
{
    uint32_t view, d;
    int32_t r;

    view = VIEW_OF(G32(0x1015d1ac));
    if (view == 0)
        return;
    d = U32(view, 0x14);
    if (d == 0)
        return;
    if (W8X9_CALL_100766ca(5) != 0 || G32(0x1015da28) != 0) {
        G32(0x1015da28) = 0;
        Sim_10099c59();
        return;
    }
    switch (G32(0x1017cc50)) {
    case 0:
        r = Sim_1009c929((int)U32(U32(d, 4), 0));
        G32(0x1017cc48) = G32(0x1015dc78) + (uint32_t)r;
        G32(0x1017cc50) = 1;
        /* fall through */
    case 1:
        if (GS32(0x1015dc78) < GS32(0x1017cc48)) {
            Sim_10099c59();
            return;
        }
        G32(0x1017cc50) = 2;
        r = Sim_1009c929((int)U32(U32(d, 4), 0));
        G32(0x1017cc4c) = G32(0x1015dc78) + (uint32_t)r;
        Sim_1008f630((void **)(uintptr_t)d);
        /* fall through */
    case 2:
        G32(0x10144fa0) = 0;
        r = Sim_1009c929((int)(U32(U32(d, 4), 0) - G32(0x1015dc80) - 1));
        U32(U32(d, 0), 8) = (uint32_t)r;
        Sim_1009b8d8(1, (uint32_t *)(uintptr_t)view, (const uint32_t *)(uintptr_t)d, 1);
        if (GS32(0x1015dc78) > GS32(0x1017cc4c))
            G32(0x1017cc50) = 0;
        break;
    default:
        break;
    }
}

// FUNCTION: HGSIM 0x1009bba2
void __cdecl Sim_1009bba2(void)
{
    uint32_t view, target, pane;

    view = VIEW_OF(G32(0x1015d1ac));
    if (view == 0)
        return;
    if (G32(0x1015d1ac) == 4) {
        Sim_1009ba57();
        return;
    }
    target = U32(U32(view, 0x78), 8);
    if (target == 0xffffffffu) {
        Sim_10099c59();
        return;
    }
    if (S16(G32(0x109e88e0), 6) == 1) {
        int32_t limit = G32(0x1017cca4) != 0 ? 7 : 3;

        G32(0x1017cca4) = Sim_1009c929(10) < limit ? 1 : 0;
    } else if (S16(G32(0x109e88e0), 6) == 2) {
        G32(0x1017cca4) = 1;
    }
    Sim_10099c59();
    if (G32(0x1017cca4) != 0) {
        pane = U32(view, 0);
        W8X9_CALL_10048287((void *)(uintptr_t)pane, target, 0, 0);
        if (U32(view, 0x7c) != 0)
            (*(W8xViewEndFn *)(BYTES(view) + 0x7c))(pane, U32(U32(view, 0x74), 0x30));
    }
}
