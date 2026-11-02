#include "w5b.h"
#include "../../platform/picture.h"
#include "../win/w4u_api.h"
#include "../win/w4u.h"
#include "../unsorted_1004/w2l.h"

typedef float HG_MAY_ALIAS w5b_f32;
#define W5B_F32(p, off) (*(w5b_f32 *)(BYTES(p) + (off)))

/* The C library's _ftol: truncation toward zero to 64 bits, of which the low 32 are used. */
static int32_t Ftol(double value)
{
    return (int32_t)(int64_t)value;
}

// FUNCTION: HGSIM 0x10083c45
int __cdecl Sim_10083c45(void)
{
    if ((G32(0x10179ca0) != 0 || G32(0x10179ca4) != 0) && G32(0x1017b92c) == 0) {
        SIM_CALL_10083801();
    } else {
        /* the original's SetDIBitsToDevice of the whole picture: the loading-screen picture (platform/picture.h),
           which never fails (the original's GetLastError report is not reached) */
        GS32(0x103729c4) = Pic_Blit(BYTES(G32(0x1015e9a0)), GS32(0x109e3d14), GS32(0x109e3d18), 0, 0, 0, 0,
                                    GS32(0x109e3d14), GS32(0x109e3d18));
    }
    return G32(0x103729c4) != G32(0x109e3d18) ? -1 : 0;
}

// FUNCTION: HGSIM 0x1008404b
int __cdecl Sim_1008404b(const uint8_t *rgb)
{
    int i;

    for (i = 0; i < 0x100; i++) {
        const uint8_t *from = rgb + i * 3;
        uint8_t *copy = SIM_PTR(uint8_t, 0x109b8c00u + (uint32_t)i * 3u);

        copy[0] = from[0];
        copy[1] = from[1];
        copy[2] = from[2];
        Sim_1004b804(from, SIM_PTR(uint8_t, 0x1015e6a0u + (uint32_t)i * 3u));
    }
    Gdi_SetPalette(0, 0x100, SIM_PTR(uint8_t, 0x1015e6a0));
    Sim_10083c45();
    U32(G32(0x1015e698), 0) = G32(0x1015e9a0);
    return 0;
}

static uint32_t Pack565(const uint8_t *p)
{
    return ((uint32_t)(p[0] >> 1) << 11) | ((uint32_t)p[1] << 5) | (uint32_t)(p[2] >> 1);
}

// FUNCTION: HGSIM 0x100878bb
int __cdecl Sim_100878bb(int start, int count, const uint8_t *rgb)
{
    int i;

    for (i = start; i < start + count; i++)
        SIM_CALL_100f01cc(i, Pack565(rgb + i * 3));
    return 0;
}

// FUNCTION: HGSIM 0x1008792c
int __cdecl Sim_1008792c(const uint8_t *rgb)
{
    int i;

    for (i = 0; i < 0x100; i++)
        SIM_CALL_100f01cc(i, Pack565(rgb + i * 3));
    return 0;
}

// FUNCTION: HGSIM 0x1008e687
int __cdecl Sim_1008e687(const void *rect, int arg)
{
    int32_t width = (int32_t)(U32(rect, 0xc) - U32(rect, 4) + 1u);
    int32_t w2 = width >> 1;
    int32_t h2 = (int32_t)(U32(rect, 0x10) - U32(rect, 8) + 1u) >> 1;
    int32_t radius = (int32_t)((uint32_t)width - (uint32_t)(width >> 31)) >> 1;
    int32_t scaled;

    radius = (int32_t)((uint32_t)radius - 1u);
    scaled = Ftol((double)radius * (double)W5B_F32(G32(0x1014cae0), 0x44));
    return SIM_CALL_100eb6e8(rect, w2, h2, radius, scaled, arg);
}
