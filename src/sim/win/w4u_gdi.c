#include "w4u.h"
#include "../../platform/picture.h"
#include "w4u_api.h"
#include "../unsorted_1004/w2l.h"

#define WINDOW (*SIM_PTR(void *, 0x1017b35c))
#define LAST_RESULT (*SIM_PTR(int, 0x10372d8c))

/* Sim_1008375a (0x1008375a, the DIB section of the windowed driver) is deleted: no caller and no pointer in the DLL
   (DECISIONS.md, "The loading screen through Vulkan"). */

// FUNCTION: HGSIM 0x10083cd5
int __cdecl Gdi_BitBltRect(int a, int b, int c, int d)
{
    /* the original's SetDIBitsToDevice of the rectangle (source y counted from the bottom, lines - d - 1: the same
       rows b..d of the top-down picture): the loading-screen picture (platform/picture.h). It never fails, so the
       original's error report (GetLastError) is not reached */
    *SIM_PTR(int, 0x103729c4) = Pic_Blit(BYTES(G32(0x1015e9a0)), GS32(0x109e3d14), GS32(0x109e3d18), a, b, a, b,
                                         c - a + 1, d - b + 1);
    return *SIM_PTR(uint32_t, 0x103729c4) != *SIM_PTR(uint32_t, 0x109e3d18) ? -1 : 0;
}

// FUNCTION: HGSIM 0x10083ebb
int __cdecl Gdi_SetPalette(int start, int count, const uint8_t *rgb)
{
    int i;

    if (rgb == 0 || start < 0 || start > 0xff || count <= 0 || count > 0x100 - start)
        return 0;
    for (i = 0; i < count; i++) {
        uint8_t *to = SIM_PTR(uint8_t, 0x1015e6a0u + (uint32_t)(start + i) * 3);

        to[0] = rgb[i * 3];
        to[1] = rgb[i * 3 + 1];
        to[2] = rgb[i * 3 + 2];
    }
    for (i = 0; i < 0x100; i++) {
        const uint8_t *from = SIM_PTR(uint8_t, 0x1015e6a0u + (uint32_t)i * 3);
        uint8_t *entry = SIM_PTR(uint8_t, 0x1014ab24u + (uint32_t)i * 4);

        entry[0] = (uint8_t)(from[0] << 2);
        entry[1] = (uint8_t)(from[1] << 2);
        entry[2] = (uint8_t)(from[2] << 2);
        entry[3] = 4;
    }
    /* the window's GDI palette (CreatePalette and SelectPalette the first time, else SetPaletteEntries; then
       RealizePalette): the loading-screen picture's palette (platform/picture.h); [0x1017b3c4] keeps a nonzero
       mark for "made" */
    if (*SIM_PTR(uint32_t, 0x1017b3c4) == 0)
        *SIM_PTR(uint32_t, 0x1017b3c4) = 1;
    Pic_SetPalette(SIM_PTR(uint8_t, 0x1015e6a0));
    return 1;
}

// FUNCTION: HGSIM 0x1008400d
int __cdecl Sim_1008400d(int a, int b, const uint8_t *c, int d)
{
    (void)d;
    if (*SIM_PTR(uint32_t, 0x1017b7d8) == 0)
        return -1;
    return Gdi_SetPalette(a, b, c) != 0 ? 0 : -1;
}
