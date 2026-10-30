#include "w8x.h"
#include "../util/access.h"
#include "../util/w3o.h"

/* The compass needles, in a file of their own so the compiler does not inline them into their callers in
 * w8x_views.c (the checker stubs them there). */
#define CAMERA BYTES(G32(0x1014cae0))

typedef void(__cdecl *W8xNeedleFn)(void *pane, float angle, int32_t *end);

/* The pane colour: through the word table 0x101659e0 in the high-colour mode ([0x1017b940]). */
static void W8x_Line(void *pane, uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1, uint32_t colour)
{
    if (G32(0x1017b940) != 0)
        W8X9_CALL_100ef727(pane, x0, y0, x1, y1, 0, SIM_PTR(u16a, 0x101659e0)[colour]);
    else
        W8X9_CALL_100e86a1(pane, x0, y0, x1, y1, 0, colour);
}

/* The two needles of the compass: the angle turned to the pane, half the spread (lookup of 1.0 and camera +0x18). */
static void W8x_Needles(void *pane, void *view, float angle)
{
    uint32_t info = U32(view, 0x74);
    int32_t c[2], end[2];
    float a, half;

    a = (float)(W8X9_CALL_100fcb30(W8X9_CALL_100fcb30((double)angle, 360.0) + GF64(0x1012b9c8), 360.0));
    c[0] = (int32_t)(U32(pane, 0xc) - U32(pane, 4) + 1) >> 1;
    c[1] = (int32_t)(U32(pane, 0x10) - U32(pane, 8) + 1) >> 1;
    half = (float)W8X9_CALL_1008486e(1.0f, FL(CAMERA, 0x18));
    a = (float)((double)GFL(0x1012b9d0) - (double)a);
    if (U32(view, 0x88) == 0)
        return;
    (*(W8xNeedleFn *)(BYTES(view) + 0x88))(pane, (float)((double)a - (double)half), end);
    W8x_Line(pane, (uint32_t)c[0], (uint32_t)c[1], (uint32_t)end[0], (uint32_t)end[1], U32(info, 0x2c));
    (*(W8xNeedleFn *)(BYTES(view) + 0x88))(pane, (float)((double)a + (double)half), end);
    W8x_Line(pane, (uint32_t)c[0], (uint32_t)c[1], (uint32_t)end[0], (uint32_t)end[1], U32(info, 0x2c));
}

// FUNCTION: HGSIM 0x1009ace7
void __cdecl Sim_1009ace7(void *view, float angle)
{
    W8x_Needles((void *)(uintptr_t)U32(view, 0), view, angle);
}

// FUNCTION: HGSIM 0x1009ae96
void __cdecl Sim_1009ae96(void *pane, void *view, float angle)
{
    W8x_Needles(pane, view, angle);
}

