#include "w37.h"
#include "w37_int.h"
#include "../simmem.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x100ecedd
uint32_t __cdecl Sim_100ecedd(const uint8_t *pane, int32_t y, const uint8_t *line, uint32_t count)
{
    const uint8_t *window = (const uint8_t *)(uintptr_t)U32(pane, 0);
    int32_t pitch, cx0, cy0, cx1, cy1, x, d;
    uint32_t n = count, k;
    uint8_t *dst;

    if (S32(window, 4) < 0 || S32(window, 8) < 0)
        return 0xffffffffu;
    pitch = ADD(S32(window, 4), 1);
    cx0 = S32(pane, 4) > 0 ? S32(pane, 4) : 0;
    cy0 = S32(pane, 8) > 0 ? S32(pane, 8) : 0;
    cx1 = S32(pane, 0xc) < S32(window, 4) ? S32(pane, 0xc) : S32(window, 4);
    cy1 = S32(pane, 0x10) < S32(window, 8) ? S32(pane, 0x10) : S32(window, 8);
    if (cx1 < cx0 || cy1 < cy0)
        return 0xfffffffeu;
    x = S32(pane, 4);
    y = ADD(y, S32(pane, 8));
    d = DIFF(DIFF(cx1, x) + 1, n);
    if (d < 0) {
        if (SUM_LE0(n, d))
            return (uint32_t)d;
        n += (uint32_t)d;
    }
    d = DIFF(x, cx0);
    if (d < 0) {
        if (SUM_LE0(n, d))
            return (uint32_t)d;
        n += (uint32_t)d;
        line -= d;
        x = DIFF(x, d);
    }
    d = DIFF(cy1, y);
    if (d < 0)
        return (uint32_t)d;
    d = DIFF(y, cy0);
    if (d < 0)
        return (uint32_t)d;
    dst = (uint8_t *)(uintptr_t)(U32(window, 0) + (uint32_t)y * (uint32_t)pitch + (uint32_t)x);
    for (k = 0; k < n; k++)
        dst[k] = line[k];
    return (uint32_t)(uintptr_t)dst;
}

/*
 * The GIF decoder's state record (the caller's, 0x502e bytes): +0 the next free code, +4 the code count at which the
 * code size grows, +8 the pixels in the line buffer, +0xc the current row, +0x10 the bytes left in the data
 * sub-block, +0x14 / +0x18 the bit buffer and its bit count, +0x1c the code size, +0x20 the pixels left in the row,
 * +0x24 / +0x28 the image's width and height, byte +0x2c interlaced (0x40), byte +0x2d the interlace pass, +0x2e the
 * output string, +0x102e each code's first byte, +0x202e its last byte, +0x302e (words) its prefix code (0xffff
 * none, 0xfffe unused). The original passes the record in EDI and the data pointer in ESI to its helpers.
 */
#define LINE GBYTES(0x10153b95)                 /* the decoded row */
#define FIRST(st, c) U8(st, 0x102e + (c))
#define LAST(st, c) U8(st, 0x202e + (c))
#define PREFIX(st, c) U16(st, 0x302e + 2 * (c))

/* 0x100ed327 (record in EDI, clear code in ECX): the table reset. */
static void W37_GifReset(uint8_t *st, int32_t clear)
{
    int32_t k;

    U32(st, 0) = (uint32_t)clear + 2;
    U32(st, 4) = (uint32_t)clear << 1;
    for (k = 0; k < clear; k++) {
        FIRST(st, k) = (uint8_t)k;
        LAST(st, k) = (uint8_t)k;
        PREFIX(st, k) = 0xffff;
    }
    for (; k < 0x1000; k++)
        PREFIX(st, k) = 0xfffe;
}

/* 0x100ed36f (record in EDI, data in ESI): the next data byte, past the sub-block length bytes. */
static uint32_t W37_GifByte(uint8_t *st, const uint8_t **src)
{
    if (U32(st, 0x10) == 0)
        U32(st, 0x10) = *(*src)++;
    U32(st, 0x10)--;
    return *(*src)++;
}

/* 0x100ed388 (record in EDI, data in ESI, bit count in EDX, at most 8): the next `bits` bits, low first. */
static uint32_t W37_GifBits(uint8_t *st, const uint8_t **src, int32_t bits)
{
    uint32_t v;

    if (U32(st, 0x18) == 0) {
        U32(st, 0x14) = W37_GifByte(st, src);
        U32(st, 0x18) = 8;
    }
    if (S32(st, 0x18) < bits) {
        U32(st, 0x14) |= W37_GifByte(st, src) << (U32(st, 0x18) & 31);
        U32(st, 0x18) += 8;
    }
    v = U32(st, 0x14) & U8(GBYTES(0x10154e95), bits);
    U32(st, 0x18) -= (uint32_t)bits;
    U32(st, 0x14) >>= bits & 31;
    return v;
}

/* 0x100ed3ce (record in EDI, code in EBX, previous code in ECX): a new code, previous + first byte of `code`. */
static void W37_GifAdd(uint8_t *st, uint32_t code, uint32_t prev)
{
    uint32_t next = U32(st, 0);

    PREFIX(st, next) = (uint16_t)prev;
    LAST(st, next) = FIRST(st, code);
    FIRST(st, next) = FIRST(st, prev);
    U32(st, 0)++;
    if (U32(st, 0) == U32(st, 4) && S32(st, 0x1c) < 0xc) {
        U32(st, 0x1c)++;
        U32(st, 4) <<= 1;
    }
}

/* 0x100ed414 (record in EDI, pixel in AL): the pixel into the row; a full row goes to the pane (0x100ecedd) at the
 * current row, which then steps by the interlace tables 0x10154e9e (steps) / 0x10154ea3 (starts) or by one. */
static void W37_GifPixel(uint8_t *st, uint8_t pixel)
{
    LINE[U32(st, 8)] = pixel;
    U32(st, 8)++;
    U32(st, 0x20)--;
    if (U32(st, 0x20) != 0)
        return;
    Sim_100ecedd((const uint8_t *)(uintptr_t)G32(0x10154ea8), S32(st, 0xc), LINE, U32(st, 0x24));
    U32(st, 8) = 0;
    U32(st, 0x20) = U32(st, 0x24);
    if (U8(st, 0x2c) != 0) {
        U32(st, 0xc) += U8(GBYTES(0x10154e9e), U8(st, 0x2d));
        if (S32(st, 0xc) >= S32(st, 0x28)) {
            U8(st, 0x2d)++;
            U32(st, 0xc) = U8(GBYTES(0x10154ea3), U8(st, 0x2d));
        }
    } else {
        U32(st, 0xc)++;
        if (S32(st, 0xc) >= S32(st, 0x28))
            U32(st, 0xc) = 0;
    }
}

// FUNCTION: HGSIM 0x100ed491
uint32_t __cdecl Sim_100ed491(const uint8_t *pane, const uint8_t *gif, uint8_t *st)
{
    const uint8_t *src = gif;
    uint32_t background, flags, clear, eoi, code, old, done, k, c;
    uint32_t minimum;

    G32(0x10154ea8) = (uint32_t)(uintptr_t)pane;
    for (k = 0; k < 0x2e; k++)
        st[k] = 0;
    background = gif[0xb];
    flags = gif[0xa];
    src = gif + 0xd;
    if (flags & 0x80)
        src += (1u << ((flags & 7) + 1)) * 3;
    U32(st, 0x24) = U16(src, 5);
    U32(st, 0x28) = U16(src, 7);
    flags = src[9];
    U8(st, 0x2c) = (uint8_t)(flags & 0x40);
    src += 0xa;
    if (flags & 0x80)
        src += (1u << ((flags & 7) + 1)) * 3;
    U32(st, 0x10) = 0;
    minimum = *src++;
    clear = 1u << (minimum & 31);
    eoi = clear + 1;
    U32(st, 0x1c) = minimum + 1;
    W37_GifReset(st, (int32_t)clear);
    old = 0xffff;
    done = 0;
    U8(st, 0x2d) = 0;
    U32(st, 0x20) = U32(st, 0x24);
    U32(st, 8) = 0;
    U32(st, 0xc) = 0;
    do {
        if (S32(st, 0x1c) <= 8) {
            code = W37_GifBits(st, &src, S32(st, 0x1c));
        } else {
            uint32_t low = W37_GifBits(st, &src, 8);

            code = W37_GifBits(st, &src, S32(st, 0x1c) - 8) << 8 | low;
        }
        if (code == clear) {
            W37_GifReset(st, (int32_t)clear);
            U32(st, 0x1c) = minimum + 1;
            old = 0xffff;
        } else if (code == eoi) {
            do {
                while (U32(st, 0x10) != 0) {
                    src++;
                    U32(st, 0x10)--;
                }
                U32(st, 0x10) = *src++;
            } while (U32(st, 0x10) != 0);
            done = 0xffff;
        } else {
            if (PREFIX(st, code) != 0xfffe) {
                if (old != 0xffff)
                    W37_GifAdd(st, code, old);
            } else {
                W37_GifAdd(st, old, old);
            }
            /* the string of `code`, last byte first, then put out in order (the original's 1-bit branch, for a
               pixel width of 1 in EDX, is never taken: EDX is always 8 there) */
            k = 0;
            c = code;
            do {
                st[0x2e + k] = LAST(st, c);
                k++;
                c = PREFIX(st, c);
            } while (c != 0xffff);
            do {
                k--;
                W37_GifPixel(st, st[0x2e + k]);
            } while (k != 0);
            old = code;
        }
    } while (done == 0);
    return background;
}
