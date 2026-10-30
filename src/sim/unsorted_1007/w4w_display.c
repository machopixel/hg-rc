#include "w4w.h"
#include "../win/w4w_api.h"
#include "../util/access.h"
#include "../unsorted_1000/w2e.h"
#include "../unsorted_1008/w2i.h"
#include "../unsorted_100f/w2e.h"

typedef void(__cdecl *W4wReport)(const void *format, uint32_t value);

static void report(uint32_t format, uint32_t value)
{
    ((W4wReport)(void *)Sim_10083495)(GBYTES(format), value);
}

// FUNCTION: HGSIM 0x10074b80
int __cdecl Sim_10074b80(uint32_t value)
{
    uint8_t *entry = (uint8_t *)SIM_API(W4wHeapAllocFn, W4W_IAT_HEAPALLOC)(W4W_GAME_HEAP, 1, 0x18);

    if (entry == 0)
        return 0;
    Mem_Fill(entry, 0, 0x18);
    U32(entry, 0) = value;
    if (G32(0x1015dc14) == 0)
        G32(0x1015dc14) = (uint32_t)(uintptr_t)entry;
    else
        U32(G32(0x1015dc10), 0x14) = (uint32_t)(uintptr_t)entry;
    G32(0x1015dc10) = (uint32_t)(uintptr_t)entry;
    return 1;
}

typedef struct {
    uint32_t dwData;
    uint32_t cbData;
    uint32_t lpData;
} W4wCopyData;

// Sends the message and keeps the result in 0x10372d8c, as the three callers do.
static void send_copydata(uint32_t dwData, uint32_t cbData, uint32_t lpData)
{
    W4wCopyData cds;

    cds.dwData = dwData;
    cds.cbData = cbData;
    cds.lpData = lpData;
    G32(0x10372d8c) = (uint32_t)SIM_API(W4wSendMessageFn, W4W_IAT_SENDMESSAGE)((void *)(uintptr_t)G32(0x1017b35c), 0x4a, 0, &cds);
}

static void send_pixels(uint32_t pixels, uint32_t format)
{
    uint32_t size = G32(0x109e3ce8);

    send_copydata(0x401u | (0x240u << 16), pixels != 0 ? size : 0, pixels);
    if (G32(0x10372d8c) != 0)
        report(format, G32(0x10372d8c));
    U32(G32(0x1015e698), 0) = 0;
}

// FUNCTION: HGSIM 0x1007ef27
uint32_t __cdecl Sim_1007ef27(void)
{
    send_pixels(G32(0x1015e9a0), 0x10149fd4);
    return G32(0x10372d8c);
}

// FUNCTION: HGSIM 0x1007efb4
uint32_t __cdecl Sim_1007efb4(int x1, int y1, int x2, int y2)
{
    G32(0x10372d8c) = (uint32_t)SIM_API(W4wStretchBltFn, W4W_IAT_STRETCHBLT)(
        (void *)(uintptr_t)G32(0x1017b368), 0, 0, GS32(0x109e3d14), GS32(0x109e3d18), (void *)(uintptr_t)G32(0x1017b364),
        x1, y1, x2 - x1 + 1, y2 - y1 + 1, 0xcc0020);
    if (G32(0x10372d8c) == 0) {
        G32(0x10372d8c) = SIM_API(W4wGetLastErrorFn, W4W_IAT_GETLASTERROR)();
        report(0x10149ff4, G32(0x10372d8c));
    }
    send_pixels(G32(0x1017b374), 0x1014a008);
    return G32(0x10372d8c);
}

// FUNCTION: HGSIM 0x1007f0c8
int __cdecl Sim_1007f0c8(int first, int count, const uint8_t *rgb)
{
    int i;

    if (G32(0x1017b378) == 0)
        return -1;
    if (rgb == 0 || first < 0 || first > 0xff || count <= 0 || count > 0x100 - first)
        return -1;
    for (i = 0; i < count; i++) {
        uint8_t *dst = GBYTES(0x1015e6a0) + (first + i) * 3;
        const uint8_t *src = rgb + i * 3;
        uint8_t *entry = GBYTES(0x109e38e8) + (first + i) * 4;

        U16(dst, 0) = U16(src, 0);
        dst[2] = src[2];
        entry[2] = (uint8_t)(rgb[i * 3] << 2);
        entry[1] = (uint8_t)(rgb[i * 3 + 1] << 2);
        entry[0] = (uint8_t)(rgb[i * 3 + 2] << 2);
    }
    send_copydata(0x400, 0x428, (uint32_t)(uintptr_t)GBYTES(0x109e38c0));
    if (G32(0x10372d8c) != 0) {
        report(0x1014a03c, G32(0x10372d8c));
        return -1;
    }
    return 0;
}
