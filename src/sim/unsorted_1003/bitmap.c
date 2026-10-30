#include "fns.h"
#include "../simmem.h"

/*
 * The header at 0x109e38c0 has the BITMAPINFOHEADER layout: size 40, width,
 * height (negative = top-down rows), planes 1, 8 bits per pixel, no
 * compression, all remaining fields 0.
 */
// FUNCTION: HGSIM 0x1003815f
void __cdecl Dib_InitHeader(uint32_t width, int height)
{
    unsigned char *h = SIM_PTR(unsigned char, 0x109E38C0);

    *(uint32_t *)(h + 0x00) = 0x28;
    *(uint32_t *)(h + 0x04) = width;
    *(uint32_t *)(h + 0x08) = 0u - (uint32_t)height;
    *(uint16_t *)(h + 0x0c) = 1;
    *(uint16_t *)(h + 0x0e) = 8;
    *(uint32_t *)(h + 0x10) = 0;
    *(uint32_t *)(h + 0x14) = 0;
    *(uint32_t *)(h + 0x20) = 0;
    *(uint32_t *)(h + 0x18) = 0;
    *(uint32_t *)(h + 0x1c) = 0;
    *(uint32_t *)(h + 0x24) = 0;
}
