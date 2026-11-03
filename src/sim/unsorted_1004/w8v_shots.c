#include "w8v.h"
#include "w2k.h"
#include "w5c.h"
#include "w5z.h"
#include "w7m.h"
#include "../util/access.h"
#include "../unsorted_1003/w2k.h"
#include "../unsorted_100f/w2e.h"
#include "../unsorted_100f/w6h.h"

/*
 * Writes a 16-bit uncompressed TGA (type 2, 16 bits per pixel, top-left origin) of width x height pixels;
 * every pixel word goes through Sim_10037680 (to 8:8:8) and is packed again as 5:5:5. stride is in bytes.
 */
// FUNCTION: HGSIM 0x100472af
int __cdecl Screenshot_WriteTga(const char *name, int32_t width, int32_t height, int32_t stride, const uint8_t *pixels)
{
    uint8_t hdr[0x12];
    uint32_t pixel;
    const uint8_t *p;
    void *f;
    int32_t x, y;

    f = Sim_100fd850(name, SIM_PTR(char, 0x1013b2cc));
    if (f == 0)
        return -1;
    Mem_Fill(hdr, 0, 0x12);
    hdr[0] = 0;
    hdr[7] = 0x10;
    U16(hdr, 8) = 0;
    U16(hdr, 0xa) = 0;
    U16(hdr, 0xc) = (uint16_t)width;
    U16(hdr, 0xe) = (uint16_t)height;
    hdr[0x10] = 0x10;
    hdr[0x11] = 0x20;
    hdr[2] = 2;
    W8V_FWRITE(hdr, 1, 0x12, f);
    for (y = 0; y < height; y++) {
        p = pixels;
        for (x = 0; x < width; x++) {
            pixel = Sim_10037680(U16(p, 0));
            pixel = ((pixel >> 19) & 0x1f) << 10 | ((pixel >> 11) & 0x1f) << 5 | ((pixel >> 3) & 0x1f);
            W8V_FWRITE(&pixel, 2, 1, f);
            p += 2;
        }
        pixels += stride;
    }
    W8V_FCLOSE(f);
    return 0;
}

/*
 * Writes a GIF image descriptor (x, y, width, height; a negative width or height means the whole screen of
 * 0x1015f218 x 0x1015f21c from 0) and the LZW image data; the pixels come from pixelFn (kept at 0x1015f334).
 */
// FUNCTION: HGSIM 0x1004deb3
int __cdecl Gif_WriteImage(int32_t x, int32_t y, int32_t width, int32_t height, uint32_t pixelFn)
{
    uint8_t rec[0x12];
    int bits, r;

    if (width < 0) {
        width = GS32(0x1015f218);
        x = 0;
    }
    if (height < 0) {
        height = GS32(0x1015f21c);
        y = 0;
    }
    if (x < 0)
        x = 0;
    if (y < 0)
        y = 0;
    if (G32(0x1015f34c) != 0) {
        if (Sim_1004db5b(BYTES(G32(0x1015f360)), G32(0x1015f34c) * 3) != 0)
            return 2;
    }
    rec[0] = 0x2c;
    G32(0x1015f338) = (uint32_t)x;
    U32(rec, 1) = G32(0x1015f338);
    G32(0x1015f220) = (uint32_t)y;
    U32(rec, 5) = G32(0x1015f220);
    G32(0x1015f33c) = (uint32_t)width;
    U32(rec, 9) = G32(0x1015f33c);
    G32(0x1015f330) = (uint32_t)height;
    U32(rec, 0xd) = G32(0x1015f330);
    /* the original clears the five bit fields of this byte one by one: no local table, not interlaced */
    rec[0x11] = 0;
    if (Sim_1004e719(rec) != 0)
        return 2;
    bits = Sim_1004db86(G32(0x1015f34c));
    if (bits == 1)
        bits++;
    if (Sim_1004dcaa((uint8_t)bits) != 0)
        return 2;
    G32(0x1015f224) = 0;
    G32(0x1015f228) = G32(0x1015f224);
    G32(0x1015f334) = pixelFn;
    r = Lzw_Compress(bits, SIM_FN(W7mPixelFn, 0x1004e6a2));
    if (r != 0)
        return r;
    if (Sim_1004dcaa(0) != 0)
        return 2;
    return 0;
}
