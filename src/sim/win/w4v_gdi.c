#include "w4v.h"
#include "../util/access.h"

typedef int (__stdcall *W4vClipCursorFn)(const void *rect);
typedef int (__stdcall *W4vStretchDIBitsFn)(void *dc, int xDest, int yDest, int wDest, int hDest, int xSrc, int ySrc,
                                            int wSrc, int hSrc, const void *bits, const void *info, uint32_t usage,
                                            uint32_t rop);

#define IAT_CLIPCURSOR 0x10ac5790u
#define IAT_STRETCHDIBITS 0x10ac556cu

// FUNCTION: HGSIM 0x1007d948
int __cdecl Mouse_Unclip(void)
{
    SIM_API(W4vClipCursorFn, IAT_CLIPCURSOR)(0);
    G32(0x10179d64) = 0;
    return 0;
}

static int StretchRect(int x0, int y0, int x1, int y1)
{
    G32(0x103729c4) = (uint32_t)SIM_API(W4vStretchDIBitsFn, IAT_STRETCHDIBITS)(
        (void *)(uintptr_t)G32(0x1017b3c0), 0, 0, GS32(0x109e3d14), GS32(0x109e3d18), x0,
        (int)(G32(0x109e3d18) - (uint32_t)y1 - 1u), (int)((uint32_t)x1 - (uint32_t)x0 + 1u),
        (int)((uint32_t)y1 - (uint32_t)y0 + 1u), (const void *)(uintptr_t)G32(0x1015e9a0), GBYTES(0x109e38c0), 1,
        0xcc0020);
    return -(int)(G32(0x103729c4) != G32(0x109e3d18));
}

// FUNCTION: HGSIM 0x10083de1
int __cdecl Sim_10083de1(int x0, int y0, int x1, int y1)
{
    return StretchRect(x0, y0, x1, y1);
}

// FUNCTION: HGSIM 0x10083e4e
int __cdecl Sim_10083e4e(int x0, int y0, int x1, int y1)
{
    return StretchRect(x0, y0, x1, y1);
}
