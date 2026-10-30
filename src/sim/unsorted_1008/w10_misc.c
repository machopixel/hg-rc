#include "w10.h"
#include "w8w.h"
#include "../simmem.h"
#include "../render/w2n.h"
#include "../util/access.h"
#include "../view/view.h"
#include "../unsorted_100f/w2e.h"

#define GF(addr) (*SIM_PTR(float, addr))

// FUNCTION: HGSIM 0x1008ec46
int32_t *__cdecl Sim_1008ec46(const void *rect, const int32_t *pt, uint32_t mode, float angle, int32_t *out)
{
    int32_t x = pt[0], y = pt[1];
    int32_t h = (int32_t)(U32(rect, 0xc) - U32(rect, 4) + 1u) / 2 - 1;     /* wraps like the original */
    /* x87 at 53-bit precision: the int times the float rounds to double, as here */
    int32_t v = Render_Ftol((double)h * (double)*(float *)(BYTES(G32(0x1014cae0)) + 0x44));
    int32_t rx, ry;

    h--;
    v--;
    switch (mode) {
    case 0:
    case 1:
    case 2:
    case 3: {
        float fh = (float)h, fv = (float)v;
        /* float times float is exact in double */
        rx = x - Render_Ftol((double)Sim_10084766(angle + GF(0x1012b920)) * (double)fh);
        ry = y - Render_Ftol((double)Sim_10084766(angle) * (double)fv);
        break;
    }
    case 5:
        rx = x;
        ry = y + v;
        break;
    case 7:
        rx = x;
        ry = y - v;
        break;
    default:
        rx = 0;
        ry = 0;
        break;
    }
    out[0] = rx;
    out[1] = ry;
    return out;
}

typedef double(__cdecl *W10_8FmodFn)(double, double);
typedef double(__cdecl *W10_8FabsFn)(double);

// FUNCTION: HGSIM 0x1008eaaf
int32_t *__cdecl Sim_1008eaaf(const void *rect, float angle, int32_t *out)
{
    int32_t centre[2];
    uint32_t mode = 0;
    float a, s, c;

    a = (float)SIM_FN(W10_8FmodFn, 0x100fcb30)(SIM_FN(W10_8FmodFn, 0x100fcb30)((double)angle, 360.0) +
                                                   *SIM_PTR(double, 0x1012b918), 360.0);
    /* sar, not a division: rounds down */
    centre[0] = (int32_t)(U32(rect, 0xc) - U32(rect, 4) + 1u) >> 1;
    centre[1] = (int32_t)(U32(rect, 0x10) - U32(rect, 8) + 1u) >> 1;
    if (a >= GF(0x1012b920) && !(a > GF(0x1012b924)))
        mode |= 1;
    if (!(a > GF(0x1012b928)))
        mode |= 2;
    s = Sim_10084766(a + GF(0x1012b920));
    c = Sim_10084766(a);
    c = c * *(float *)(BYTES(G32(0x1014cae0)) + 0x44);
    if (!((float)SIM_FN(W10_8FabsFn, 0x100fcb70)((double)s) >= GF(0x1012b914)))
        mode |= 4;
    return Sim_1008ec46(rect, centre, mode, Sim_1008486e(c, -s), out);
}

typedef void *(__stdcall *W10_8HeapAllocFn)(void *heap, uint32_t flags, uint32_t size);
typedef int(__stdcall *W10_8HeapFreeFn)(void *heap, uint32_t flags, void *block);
typedef int32_t(__cdecl *W10_8FontFn)(void *font);
typedef int32_t(__cdecl *W10_8WidthFn)(void *font, int ch);
typedef void(__cdecl *W10_8TextFn)(const int32_t *pane, int32_t x, int32_t y, void *font, const char *text,
                                     void *colours);

// FUNCTION: HGSIM 0x1008dba2
void __cdecl Sim_1008dba2(const int32_t *pane, const char *text, void *font)
{
    uint32_t len;
    char *copy, *end, *p, *start, *save;
    int32_t pos[2], w, h, lineh, x;

    if (pane == 0 || text == 0 || font == 0)
        return;
    len = Str_Length(text);
    if (len == 0)
        return;
    copy = (char *)SIM_API(W10_8HeapAllocFn, 0x10ac5658)((void *)(uintptr_t)G32(0x1017b8f8), 1, len + 1);
    p = copy;
    end = copy + len;
    Str_Copy(copy, text);
    Rect_ScalePoint(pane, SIM_PTR(const int32_t, 0x1014b9a8), pos);
    w = (int32_t)((uint32_t)pane[3] - (uint32_t)pane[1]);
    h = (int32_t)((uint32_t)pane[4] - (uint32_t)pane[2]);
    lineh = SIM_FN(W10_8FontFn, 0x100ecce0)(font);
    while (p <= end && pos[1] < h) {
        start = p;
        x = pos[0];
        while (p <= end && *p != '\n') {
            if (G32(0x101668e8) != 0)
                x += SIM_FN(W10_8WidthFn, 0x100f2cec)(font, (int)(signed char)*p);
            else
                x += SIM_FN(W10_8WidthFn, 0x100eccf3)(font, (int)(signed char)*p);
            if (x > w)
                break;
            p++;
        }
        if (p <= end) {
            if (*p == '\n') {
                *p = 0;
            } else {
                save = p;
                while (p >= start && *p != ' ' && *p != '\t')
                    p--;
                if (p < start)
                    p = save;
                *p = 0;
            }
        }
        if (*start != 0) {
            if (G32(0x101668e8) != 0)
                SIM_FN(W10_8TextFn, 0x100f2f01)(pane, pos[0], pos[1], font, start, SIM_PTR(void, 0x109b9920));
            else if (G32(0x1017b940) != 0)
                SIM_FN(W10_8TextFn, 0x100f1e10)(pane, pos[0], pos[1], font, start, SIM_PTR(void, 0x109b9920));
            else
                SIM_FN(W10_8TextFn, 0x100ecea6)(pane, pos[0], pos[1], font, start, SIM_PTR(void, 0x109b9920));
        }
        pos[1] += lineh;
        p++;
    }
    SIM_API(W10_8HeapFreeFn, 0x10ac5634)((void *)(uintptr_t)G32(0x1017b8f8), 1, copy);
}
