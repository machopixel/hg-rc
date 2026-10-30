#include "w8u.h"
#include "w7m.h"
#include "../unsorted_1008/w2i.h"
#include "../unsorted_100f/w2e.h"
#include "../util/access.h"

#define W8U_GSM(i) SIM_API(W8uGetSystemMetricsFn, 0x10ac577c)(i)

// FUNCTION: HGSIM 0x1003795b
int __cdecl Display_SetMode(int mode, uint32_t a, uint32_t b, int width, int height, int menu)
{
    uint32_t drv, table;
    int sw, sh;

    Mem_Fill(GBYTES(0x109e3880), 0, 0x30);
    if (mode == -1)
        drv = G32(0x1013a2ac);
    else if (mode >= 0 && mode < 7)
        drv = G32(0x1013a2a8 + 4 * mode);
    else
        return 0;
    table = G32(0x1013a298 + 4 * U32(drv, 4));
    G32(0x1015e694) = a;
    if (width <= 0 || height <= 0) {
        sw = W8U_GSM(0);
        width = sw;
        sh = W8U_GSM(1);
        height = sh;
    } else {
        sw = W8U_GSM(0);
        sh = W8U_GSM(1);
    }
    G32(0x109e3780) = 0;
    G32(0x109e3784) = 0;
    G32(0x109e3788) = (uint32_t)width;
    G32(0x109e378c) = (uint32_t)height;
    if (sw <= width && sh <= height) {
        G32(0x1014af30) = 0x80000000u;
        G32(0x1015e9bc) = 1;
    } else {
        G32(0x1014af30) = 0xca0000;
        SIM_API(W8uAdjustWindowRectFn, 0x10ac57d0)(GBYTES(0x109e3780), G32(0x1014af30), menu);
        G32(0x109e3788) = G32(0x109e3788) - G32(0x109e3780);
        G32(0x109e378c) = G32(0x109e378c) - G32(0x109e3784);
        G32(0x109e3784) = (uint32_t)((int32_t)((uint32_t)sh - G32(0x109e378c)) / 2);
        G32(0x109e3780) = (uint32_t)((int32_t)((uint32_t)sw - G32(0x109e3788)) / 2);
        G32(0x1015e9bc) = U32(table, 4);
    }
    if (U32(table, 4) == 1) {
        /* the original stores the screen size in locals it never reads */
        W8U_GSM(0);
        W8U_GSM(1);
    }
    if (G32(0x1013a2c8) != 0) {
        G32(0x109e3d14) = (uint32_t)width;
        G32(0x109e3d18) = (uint32_t)height;
        Sim_100381d0((const uint32_t *)(uintptr_t)table);
        G32(0x1015e688) = table;
    } else if (G32(0x1015e68c) != drv || (uint32_t)width != G32(0x109e3d14) || (uint32_t)height != G32(0x109e3d18)) {
        Sim_100381d0((const uint32_t *)(uintptr_t)table);
        ((W8uDriverCloseFn)(uintptr_t)U32(G32(0x1015e68c), 0x14))();
    }
    G32(0x1015e68c) = drv;
    G32(0x109e3d14) = (uint32_t)width;
    G32(0x109e3d18) = (uint32_t)height;
    G32(0x109e3ce8) = (uint32_t)width * (uint32_t)height;
    G32(0x1015e698) = b;
    while (U32(G32(0x1015e68c), 8) != 0) {
        if (((W8uDriverModeFn)(uintptr_t)U32(G32(0x1015e68c), 0x10))(b, width, height) == 0)
            break;
        Sim_10083495();     /* the original passes ("RefreshMode %d not available\n", driver +0): an empty print */
        U32(G32(0x1015e68c), 8) = 0;
        if (G32(0x1015e694) == 0 || U32(G32(0x1015e68c), 0) == 6)
            return 0;
        G32(0x1015e68c) = G32(0x1013a2ac + 4 * U32(G32(0x1015e68c), 0));
    }
    if (G32(0x1013a2c8) != 0 && U32(table, 0) != 2) {
        SIM_API(W8uShowWindowFn, 0x10ac57dc)(G32(0x1017b8f0), 10);
        SIM_API(W8uUpdateWindowFn, 0x10ac57e0)(G32(0x1017b8f0));
    }
    G32(0x1013a2c8) = 0;
    return 1;
}
