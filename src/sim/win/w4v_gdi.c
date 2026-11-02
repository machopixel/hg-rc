#include "w4v.h"
#include "../../platform/picture.h"
#include "../util/access.h"

typedef int (__stdcall *W4vClipCursorFn)(const void *rect);

#define IAT_CLIPCURSOR 0x10ac5790u

// FUNCTION: HGSIM 0x10083de1
int __cdecl Sim_10083de1(int x0, int y0, int x1, int y1)
{
    /* the original's StretchDIBits of x0..x1, y0..y1 (source y counted from the bottom: the same rows of the
       top-down picture) over the whole window: the loading-screen picture (platform/picture.h) */
    G32(0x103729c4) = (uint32_t)Pic_Stretch(BYTES(G32(0x1015e9a0)), GS32(0x109e3d14), GS32(0x109e3d18), x0, y0,
                                            x1 - x0 + 1, y1 - y0 + 1);
    return -(int)(G32(0x103729c4) != G32(0x109e3d18));
}

/* Sim_10083e4e (0x10083e4e, the same stretch) is deleted: no caller and no pointer in the DLL (DECISIONS.md, "The
   loading screen through Vulkan"). */
