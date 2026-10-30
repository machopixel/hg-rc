#include "w7n.h"
#include "w2i.h"
#include "w3r.h"
#include "w4w.h"
#include "w5b.h"
#include "w5x.h"
#include "../unsorted_1001/w4u.h"
#include "../unsorted_1001/w5y.h"
#include "../unsorted_1003/fns.h"
#include "../unsorted_1005/w2j.h"
#include "../unsorted_1005/w2k.h"
#include "../unsorted_100f/w2e.h"
#include "../unsorted_1010/w6i.h"
#include "../unsorted_1010/w6k.h"
#include "../win/w4u.h"
#include "../util/util.h"
#include "../util/w3o.h"

typedef void(__cdecl *W7nDriverFn)(void);
typedef uint32_t(__stdcall *W7nGetDCFn)(uint32_t window);
typedef uint32_t(__stdcall *W7nHeapAllocFn)(uint32_t heap, uint32_t flags, uint32_t size);
typedef int(__stdcall *W7nShowCursorFn)(int show);

// FUNCTION: HGSIM 0x10083530
int __cdecl Sim_10083530(uint32_t *out, int width, int height)
{
    int i;

    if (G32(0x1017b3c0) != 0)
        return 0;
    if (U32(G32(0x1015e688), 0) != 2) {
        ((W7nDriverFn)(uintptr_t)U32(G32(0x1015e688), 0x10))();
        if (G32(0x1015e9bc) != G32(0x1014af2c))
            W7N8_CALL_100381d0(SIM_PTR(void, 0x1014af28));
        G32(0x1015e688) = G32(0x1013a2a0);
    }
    G32(0x1017b3c0) = SIM_API(W7nGetDCFn, 0x10ac57ac)(G32(0x1017b8f0));
    Dib_InitHeader((uint32_t)width, height);
    for (i = 0; i < 0x100; i++)
        SIM_PTR(u16a, 0x109e38e8)[i] = (uint16_t)i;
    if (Gdi_SetPalette(0, 0x100, GBYTES(0x1015e6a0)) == 0)
        return -1;
    out[0] = SIM_API(W7nHeapAllocFn, 0x10ac5658)(G32(0x1017b8f8), 9, (uint32_t)width * (uint32_t)height);
    if (out[0] == 0)
        return 2;
    G32(0x1015e9a0) = out[0];
    out[1] = (uint32_t)width - 1u;
    out[2] = (uint32_t)height - 1u;
    out[4] = 0;
    out[3] = (uint32_t)(uintptr_t)SIM_PTR(void, 0x109e38c0);
    Mem_Fill((void *)(uintptr_t)out[0], 0, (uint32_t)width * (uint32_t)height);
    if (Sim_10083c45() != 0) {
        Sim_10083692();
        return 1;
    }
    G32(0x1017b7d8) = 1;
    return 0;
}

// FUNCTION: HGSIM 0x10084cee
void __cdecl Sim_10084cee(float *x, float *y, float *z)
{
    double length = W7N8_CALL_100fca04((double)*x * *x + (double)*y * *y + (double)*z * *z);
    float stored = (float)length;

    if (length > GFL(0x1012b870)) {
        *x = (float)((double)*x / stored);
        *y = (float)((double)*y / stored);
        *z = (float)((double)*z / stored);
    }
}

// FUNCTION: HGSIM 0x10084f10
int __cdecl Heap_ReportCheck(const char *where)
{
    uint32_t format;

    switch (Heap_Check()) {
    case -6:
        format = 0x1014b128; /* "HEAPCHK: Bad Pointer at %s\n" */
        break;
    case -4:
        format = 0x1014b10c; /* "HEAPCHK: Bad Node at %s\n" */
        break;
    case -3:
        format = 0x1014b0f0; /* "HEAPCHK: Bad Begin at %s\n" */
        break;
    case -2:
        format = 0x1014b15c; /* "HEAPCHK: OK at %s\n" */
        break;
    case -1:
        format = 0x1014b144; /* "HEAPCHK: EMPTY at %s\n" */
        break;
    default:
        format = 0x1014b170; /* "HEAPCHK: UNKNOWN at %s\n" */
        break;
    }
    return W7N8_CALL_100fc780(SIM_PTR(char, 0x101655c0), SIM_PTR(char, format), where);
}

// FUNCTION: HGSIM 0x10086c32
void __cdecl Sim_10086c32(void)
{
    if (G32(0x1017b930) != 0) {
        if (G32(0x1017b92c) != 0 && *SIM_PTR(s16a, 0x1015fd4c) != 0) {
            Sim_1008b773();
            *SIM_PTR(u16a, 0x1015fd4c) = 0;
            G32(0x1017b930) = 0;
        } else if (G32(0x1015ecd0) != 0 || W7N8_CALL_100766ca(5) != 0) {
            G32(0x1017b930) = 0;
        }
    }
    if (W7N8_CALL_100766ca(5) == 0 && G32(0x1017b920) != 0 && G32(0x1017b930) == 0) {
        if (G32(0x1017b92c) != 0) {
            while (SIM_API(W7nShowCursorFn, 0x10ac5768)(0) >= 0)
                ;
            G32(0x1017b93c) = 0;
            Sim_10083495(); /* "WinMain(3): pause_timer(false)": the debug print is empty */
            W7N8_CALL_100e46d0(0x80, 0);
            Sim_1001f4d3();
            Sim_1005571d();
            G32(0x1017b92c) = 0;
        }
        return;
    }
    if (G32(0x1017b92c) != 0 || G32(0x1015ecd0) != 0)
        return;
    if (G32(0x1017b930) != 0) {
        Sim_1008b757();
        *SIM_PTR(u16a, 0x1015fd4c) = 0;
    }
    if (W7N8_CALL_100766ca(5) == 0 && G32(0x1015e9bc) != 1) {
        while (SIM_API(W7nShowCursorFn, 0x10ac5768)(1) < 0)
            ;
        G32(0x1017b93c) = 1;
    }
    Sim_10083495(); /* "WinMain(4): pause_timer(TRUE)" */
    W7N8_CALL_100e46d0(0x80, 1);
    Sim_1001f4ba();
    Sim_1005570e();
    G32(0x1017b92c) = 1;
}

// FUNCTION: HGSIM 0x10088804
uint8_t *__cdecl Sim_10088804(uint8_t *obj)
{
    union {
        float f;
        uint32_t u;
    } x, y, z;
    int32_t column, row;
    uint8_t *cell;

    if ((U16(obj, 0) & 0x1000) != 0)
        return GBYTES(0x1017b9d8);
    if (U32(obj, 0x28) != 0) {
        Sim_100a9cfb(BYTES(U32(obj, 0x28)), &x.u, &y.u, &z.u);
    } else {
        x.u = U32(obj, 0x44);
        z.u = U32(obj, 0x4c);
    }
    column = Render_Ftol(((double)x.f + GFL(0x1014b8e0)) / GFL(0x1012b8e0));
    column = Render_Ftol((double)column + GFL(0x1012b8e4));
    row = Render_Ftol(((double)z.f + GFL(0x1014b8e4)) / GFL(0x1012b8e0));
    row = Render_Ftol((double)row + GFL(0x1012b8e4));
    S32(obj, 0x8c) = column;
    S32(obj, 0x90) = row;
    if (column >= 0 && column < 9 && row >= 0 && row < 9)
        cell = GBYTES(0x1017ba28) + column * 0x18c + row * 0x2c;
    else
        cell = GBYTES(0x1017b9b0);
    if (cell == GBYTES(0x1017b9b0))
        return cell;
    if (Sim_1010d240(obj + 0xa0, SIM_PTR(uint8_t, 0x1014b8e8), 4) != 0)
        return cell;
    U32(GBYTES(0x1017ba50) + column * 0x18c + row * 0x2c, 0) = (uint32_t)(uintptr_t)obj;
    if ((int8_t)U8(obj, 0xa6) == 'B' && (int8_t)U8(obj, 0xa7) == 'O')
        Sim_10087b00(-x.f, -z.f);
    return cell;
}
