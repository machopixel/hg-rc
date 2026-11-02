#include "w37.h"
#include "../simmem.h"
#include "../util/access.h"

#define COLOURS SIM_PTR(u16a, 0x10156dcc)   /* the 256-entry colour table */

// FUNCTION: HGSIM 0x100ecce0
int32_t __cdecl Sim_100ecce0(const uint8_t *font)
{
    return S32(font, 8);
}

// FUNCTION: HGSIM 0x100ed8c3
int32_t __cdecl Sim_100ed8c3(const uint8_t *table)
{
    return S32(table, 4);
}

// FUNCTION: HGSIM 0x100eccf3
int32_t __cdecl Sim_100eccf3(const uint8_t *font, int32_t code)
{
    return S32(font + U32(font, 0x10 + 4 * code), 0);
}

// FUNCTION: HGSIM 0x100ed743
uint32_t __cdecl Sim_100ed743(const uint8_t *table, int32_t number)
{
    return U32(table + U32(table, 8 + 8 * number), 0);
}

// FUNCTION: HGSIM 0x100f1f86
uint32_t __cdecl Sim_100f1f86(const uint8_t *table, int32_t number)
{
    const uint8_t *shape = table + U32(table, 8 + 8 * number);

    return U32(shape, 8) << 16 | U16(shape, 0xc);
}

// FUNCTION: HGSIM 0x100ed7bc
uint32_t __cdecl Sim_100ed7bc(const uint8_t *table, int32_t number)
{
    const uint8_t *shape = table + U32(table, 8 + 8 * number);

    return U32(shape, 8) << 16 | U16(shape, 0xc);
}

// FUNCTION: HGSIM 0x100f1f52
uint32_t __cdecl Sim_100f1f52(const uint8_t *table, int32_t number)
{
    const uint8_t *shape = table + U32(table, 8 + 8 * number);
    uint32_t w = U32(shape, 0x10) - U32(shape, 8) + 1;
    uint32_t h = U32(shape, 0x14) - U32(shape, 0xc) + 1;

    return w << 16 | (h & 0xffff);
}

// FUNCTION: HGSIM 0x100ed788
uint32_t __cdecl Sim_100ed788(const uint8_t *table, int32_t number)
{
    const uint8_t *shape = table + U32(table, 8 + 8 * number);
    uint32_t w = U32(shape, 0x10) - U32(shape, 8) + 1;
    uint32_t h = U32(shape, 0x14) - U32(shape, 0xc) + 1;

    return w << 16 | (h & 0xffff);
}

// FUNCTION: HGSIM 0x100ed7e6
void __cdecl Sim_100ed7e6(const uint8_t *table, int32_t number, uint8_t *rgb)
{
    uint32_t off = U32(table, 8 + 8 * number + 4);
    const uint8_t *p;
    uint32_t n;

    if (off == 0)
        return;
    p = table + off;
    n = U32(p, 0);
    p += 4;
    do {
        uint32_t k = (uint32_t)p[0] * 3;

        rgb[k] = p[1];
        U16(rgb, k + 1) = U16(p, 2);
        p += 4;
    } while (--n != 0);
}

// FUNCTION: HGSIM 0x100f01ea
uint32_t __cdecl Sim_100f01ea(int32_t index)
{
    return ((uint32_t)index & 0xffff0000u) | COLOURS[index];
}

// FUNCTION: HGSIM 0x100f01cc
int32_t __cdecl Sim_100f01cc(int32_t index, uint32_t colour)
{
    COLOURS[index] = (uint16_t)colour;
    return index;
}

// FUNCTION: HGSIM 0x100f01ad
void __cdecl Sim_100f01ad(const uint32_t *table)
{
    u32a *dst = (u32a *)COLOURS;
    int k;

    for (k = 0; k < 0x80; k++)
        dst[k] = ((const u32a *)table)[k];
}

// FUNCTION: HGSIM 0x100f9eb7
uint32_t __cdecl Sim_100f9eb7(uint32_t *start, uint32_t *segment)
{
    uint16_t selector;

#ifdef _MSC_VER
    __asm mov ax, cs
    __asm mov selector, ax
#else
    __asm__ volatile("mov %%cs, %0" : "=r"(selector));
#endif
    *segment = selector;
    *start = (uint32_t)(uintptr_t)SIM_PTR(uint8_t, 0x100f4fd4);
    return 0x100f9e9a - 0x100f4fd4;
}

/* The GIF colour table size of a flags byte: 3 * 2^(bits 0..2 + 1). */
#define GIF_TABLE(flags) ((1u << (((flags) & 7) + 1)) * 3)

// FUNCTION: HGSIM 0x100ed70b
uint32_t __cdecl Sim_100ed70b(const uint8_t *gif)
{
    const uint8_t *image = gif + 0xd;

    if (gif[0xa] & 0x80)
        image += GIF_TABLE(gif[0xa]);
    return (uint32_t)U16(image, 5) << 16 | U16(image, 7);
}

// FUNCTION: HGSIM 0x100ed6aa
void __cdecl Sim_100ed6aa(const uint8_t *gif, uint8_t *rgb)
{
    const uint8_t *p = gif + 0xd;
    uint32_t n, k;

    if (gif[0xa] & 0x80) {
        n = GIF_TABLE(gif[0xa]);
        for (k = 0; k < n; k++)
            rgb[k] = (uint8_t)(*p++ >> 2);
    }
    if (p[9] & 0x80) {
        const uint8_t *local = p + 0xa;

        n = GIF_TABLE(p[9]);
        for (k = 0; k < n; k++)
            rgb[k] = (uint8_t)(local[k] >> 2);
    }
}
