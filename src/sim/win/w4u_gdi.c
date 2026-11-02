#include "w4u.h"
#include "w4u_api.h"
#include "../unsorted_1004/w2l.h"

#define WINDOW (*SIM_PTR(void *, 0x1017b35c))
#define LAST_RESULT (*SIM_PTR(int, 0x10372d8c))

// FUNCTION: HGSIM 0x1008375a
int __cdecl Sim_1008375a(void)
{
    if (*SIM_PTR(uint32_t, 0x1017b7d0) != 0) {
        *SIM_PTR(uint32_t, 0x103729c4) = SIM_API(W4uSetDIBColorTableFn, W4U_IAT_SETDIBCOLORTABLE)(
            *SIM_PTR(void *, 0x1017b7cc), 0, 0x100, SIM_PTR(void, 0x1017b3c8));
    } else {
        *SIM_PTR(void *, 0x1017b7d0) = SIM_API(W4uCreateDIBSectionFn, W4U_IAT_CREATEDIBSECTION)(
            *SIM_PTR(void *, 0x1017b3c0), SIM_PTR(void, 0x109e38c0), 1, *SIM_PTR(void **, 0x1015e698), 0, 0);
        if (*SIM_PTR(uint32_t, 0x1017b7d0) == 0 || **SIM_PTR(uint32_t *, 0x1015e698) == 0) {
            W4U_LOG(0x1014af78, SIM_API(W4uGetLastErrorFn, W4U_IAT_GETLASTERROR)());
            return 0;
        }
        *SIM_PTR(uint32_t, 0x1015e9a0) = **SIM_PTR(uint32_t *, 0x1015e698);
        *SIM_PTR(void *, 0x103729d4) = SIM_API(W4uSelectObjectFn, W4U_IAT_SELECTOBJECT)(
            *SIM_PTR(void *, 0x1017b7cc), *SIM_PTR(void *, 0x1017b7d0));
    }
    return 1;
}

// FUNCTION: HGSIM 0x10083cd5
int __cdecl Gdi_BitBltRect(int a, int b, int c, int d)
{
    uint32_t lines = *SIM_PTR(uint32_t, 0x109e3d18);

    *SIM_PTR(int, 0x103729c4) = SIM_API(W4uSetDIBitsToDeviceFn, W4U_IAT_SETDIBITSTODEVICE)(
        *SIM_PTR(void *, 0x1017b3c0), a, b, (uint32_t)(c - a + 1), (uint32_t)(d - b + 1), a,
        (int)(lines - (uint32_t)d - 1), 0, lines, *SIM_PTR(void *, 0x1015e9a0), SIM_PTR(void, 0x109e38c0), 1);
    if (*SIM_PTR(int, 0x103729c4) == 0)
        W4U_LOG(0x1014afb4, SIM_API(W4uGetLastErrorFn, W4U_IAT_GETLASTERROR)());
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
    if (*SIM_PTR(void *, 0x1017b3c4) == 0) {
        *SIM_PTR(void *, 0x1017b3c4) =
            SIM_API(W4uCreatePaletteFn, W4U_IAT_CREATEPALETTE)(SIM_PTR(void, 0x1014ab20));
        *SIM_PTR(void *, 0x103729dc) = SIM_API(W4uSelectPaletteFn, W4U_IAT_SELECTPALETTE)(
            *SIM_PTR(void *, 0x1017b3c0), *SIM_PTR(void *, 0x1017b3c4), 0);
    } else {
        SIM_API(W4uSetPaletteEntriesFn, W4U_IAT_SETPALETTEENTRIES)(*SIM_PTR(void *, 0x1017b3c4), 0, 0x100,
                                                                   SIM_PTR(void, 0x1014ab24));
    }
    SIM_API(W4uRealizePaletteFn, W4U_IAT_REALIZEPALETTE)(*SIM_PTR(void *, 0x1017b3c0));
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
