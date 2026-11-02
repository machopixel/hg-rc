#include "w4w.h"
#include "../../platform/picture.h"
#include "w2i.h"
#include "../win/w4w_api.h"
#include "../util/access.h"
#include "../crt/hg_winheap.h"

typedef void(__cdecl *W4wReport)(const void *format, uint32_t value);

// FUNCTION: HGSIM 0x10083692
int __cdecl Sim_10083692(void)
{
    G32(0x1017b7d8) = 0;
    if (G32(0x1017b3c0) != 0) {
        if (G32(0x1015e9a0) != 0)
            Hg_HeapFree(G32(0x1017b8f8), 1, BYTES(G32(0x1015e9a0)));
        /* the palette's SelectObject / DeleteObject and the window's ReleaseDC: the loading-screen picture ends
           (platform/picture.h) */
        Pic_Close();
    }
    G32(0x1017b3c0) = 0;
    G32(0x1017b3c4) = 0;
    G32(0x1017b7c8) = 0;
    G32(0x1017b7cc) = 0;
    G32(0x1017b7d0) = 0;
    G32(0x1015e9a0) = 0;
    U32(G32(0x1015e698), 0) = G32(0x1015e9a0);
    G32(0x1017b7d4) = 0;
    return 0;
}

/* Sim_10083d5b (0x10083d5b, a windowed-driver blit below the menu bar: GetSystemMetrics, SetDIBitsToDevice) is
   deleted: no caller and no pointer in the DLL (DECISIONS.md, "Windows calls: the screen"). */

// FUNCTION: HGSIM 0x1008c6b6
int __cdecl Sim_1008c6b6(void *record)
{
    if (U32(record, 4) != 0) {
        Hg_HeapFree(G32(0x1017b8f8), 1, BYTES(U32(record, 4)));
        U32(record, 4) = 0;
    }
    return ((int(__cdecl *)(uint32_t))(uintptr_t)U32(G32(0x1015e688), 0x18))(U32(record, 0));
}
