#include "w7m.h"
#include "w2j.h"
#include "w2k.h"
#include "w5b.h"
#include "w5c.h"
#include "w5z.h"
#include "w6e.h"
#include "../unsorted_100f/w6i.h"

#define STREAM G32(0x1015f344)

// FUNCTION: HGSIM 0x1004dd2a
int __cdecl Stream_PutWord(uint32_t value)
{
    int c;

    U32(STREAM, 4) = U32(STREAM, 4) - 1;
    if (S32(STREAM, 4) >= 0) {
        *BYTES(U32(STREAM, 0)) = (uint8_t)value;
        c = *BYTES(U32(STREAM, 0));
        U32(STREAM, 0) = U32(STREAM, 0) + 1;
    } else {
        c = W7M_CALL_100ff030(value & 0xff, BYTES(STREAM));
    }
    if (c == -1)
        return 2;
    U32(STREAM, 4) = U32(STREAM, 4) - 1;
    if (S32(STREAM, 4) >= 0) {
        *BYTES(U32(STREAM, 0)) = (uint8_t)(value >> 8);
        c = *BYTES(U32(STREAM, 0));
        U32(STREAM, 0) = U32(STREAM, 0) + 1;
    } else {
        c = W7M_CALL_100ff030(value >> 8, BYTES(STREAM));
    }
    if (c == -1)
        return 2;
    return 0;
}

// FUNCTION: HGSIM 0x1004d980
int __cdecl Gif_Begin(const char *name, uint32_t width, uint32_t height, uint32_t colors, int bits)
{
    uint32_t rec[3];
    uint8_t flags;
    int32_t size, i;
    uint8_t *p;

    G32(0x1015f34c) = colors != 0 ? 1u << (Sim_1004db86(colors) & 31) : 0;
    G32(0x1015f22c) = (uint32_t)bits;
    G32(0x1015f21c) = height;
    G32(0x1015f218) = width;
    if (Sim_1004db2e(name) != 0)
        return 1;
    if (Sim_1004db5b(SIM_PTR(void, 0x1013b998), 6) != 0)
        return 2;
    rec[0] = width;
    rec[1] = height;
    /* screen descriptor flags: global colour table (bit 7) and its size (bits 0..2), colour resolution (4..6) */
    if (G32(0x1015f34c) != 0)
        flags = (uint8_t)(((Sim_1004db86(G32(0x1015f34c)) - 1) & 7) | 0x80);
    else
        flags = 0;
    flags = (uint8_t)((flags & 0x87) | (((uint32_t)bits - 1) & 7) << 4);
    rec[2] = flags;
    if (Sim_1004dbc5(rec) != 0)
        return 2;
    if (G32(0x1015f360) != 0) {
        Heap_FreeBlock(G32(0x1015f360));
        G32(0x1015f360) = 0;
    }
    if (G32(0x1015f34c) != 0) {
        size = (int32_t)(G32(0x1015f34c) * 3);
        G32(0x1015f360) = (uint32_t)(uintptr_t)Heap_Malloc((uint32_t)size);
        if (G32(0x1015f360) == 0)
            return 3;
        p = BYTES(G32(0x1015f360));
        for (i = 0; i < size; i++)
            *p++ = 0;
    }
    return 0;
}

// FUNCTION: HGSIM 0x1004e719
int __cdecl Sim_1004e719(const uint8_t *rec)
{
    if (Sim_1004dcaa(rec[0]) != 0)
        return 2;
    if (Stream_PutWord(U32(rec, 1)) != 0)
        return 2;
    if (Stream_PutWord(U32(rec, 5)) != 0)
        return 2;
    if (Stream_PutWord(U32(rec, 9)) != 0)
        return 2;
    if (Stream_PutWord(U32(rec, 0xd)) != 0)
        return 2;
    /* the original packs the bit fields 7, 6, 5, 4..3 and 2..0 of this byte again, which gives the same byte */
    if (Sim_1004dcaa(rec[0x11]) != 0)
        return 2;
    return 0;
}

// FUNCTION: HGSIM 0x1004e04a
int __cdecl Lzw_Compress(int bits, W7mPixelFn next)
{
    uint32_t prefix = 0xffff;
    int32_t clear = (int32_t)(1u << (bits & 31));
    int32_t end = clear + 1;
    int32_t width = bits + 1;
    uint32_t limit = (1u << (width & 31)) - 1;
    uint32_t code;
    int r, c;

    Sim_1004e1a1();
    r = Sim_1004e441();
    if (r != 0)
        return r;
    Sim_1004e61a((uint32_t)bits);
    Sim_1004e24a(clear, width);
    while ((c = next()) != -1) {
        code = Sim_1004e582(prefix, (uint8_t)c);
        if (code != 0xffff) {
            prefix = code;
            continue;
        }
        Sim_1004e24a((int32_t)prefix, width);
        if (Sim_1004e4c2(prefix, (uint8_t)c) > limit) {
            width++;
            if (width > 12) {
                Sim_1004e24a(clear, width - 1);
                Sim_1004e61a((uint32_t)bits);
                width = bits + 1;
            }
            limit = (1u << (width & 31)) - 1;
        }
        prefix = (uint32_t)c;
    }
    if (prefix != 0xffff)
        Sim_1004e24a((int32_t)prefix, width);
    Sim_1004e24a(end, width);
    Sim_1004e1c6();
    Sim_1004e3d7();
    return 0;
}
