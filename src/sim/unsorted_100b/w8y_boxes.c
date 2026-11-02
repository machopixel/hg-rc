#include "w8y.h"
#include "../util/access.h"
#include "../util/w3o.h"
#include "../view/w3r.h"
#include "../unsorted_1000/w2l.h"
#include "../unsorted_1000/w3s.h"
#include "../unsorted_1002/w2h.h"
#include "../unsorted_1002/w2j.h"
#include "../unsorted_1006/w5y.h"
#include "../unsorted_100d/fns.h"
#include "../unsorted_100e/w5b.h"
#include "../unsorted_100e/w6h.h"
#include "../unsorted_100f/w2e.h"
#include "../crt/hg_winheap.h"

/*
 * Floating point as in util/w3o.h: double intermediates (the x87 runs with a 53-bit mantissa), float where the
 * original stores a float.
 */

/* The size of one axis: |d| (sign flipped when not >= 0; d comes from a float subtraction, so a NaN is quiet). */
static float box_abs(float d)
{
    if (!(d >= 0.0f))
        d = -d;
    return d;
}

/* The resource key { word key; char name[12]; byte 0 } for "unitbox" (text at addr). */
static void unitbox_key(uint8_t *key, uint32_t addr)
{
    U16(key, 0) = 0xffff;
    Str_CopyN((char *)key + 2, SIM_PTR(const char, addr), 0xc);
    key[0xe] = 0;
}

static void free_data(const uint8_t *key, uint32_t data)
{
    if (S16(key, 0) == -1)
        Hg_HeapFree(G32(0x1017b8f8), 1, BYTES(data));
    else
        Sim_1002828e(S16(key, 0), (const void *)(uintptr_t)G32(0x1014cedc));
}

// FUNCTION: HGSIM 0x100b5502
int __cdecl Sim_100b5502(void)
{
    uint8_t key[0x10];
    float m[12];
    uint32_t size;
    uint32_t data;
    int32_t count;
    int result = 0;
    int32_t i;

    if (G32(0x101a1164) == 0) {
        G32(0x101a1164) = 1;
        unitbox_key(key, 0x10150874);
        data = W8Y_CALL_100b8d6c(key, G32(0x1014cedc), G32(0x1014cf48), 2, &size, 0);
        if (data == 0)
            return result;
        for (i = 0; i < GS32(0x1017d220); i++) {
            uint8_t *e = GBYTES(0x101a1180) + (uint32_t)i * 0x7c;
            float dx, dy, dz, sx, sy, sz;
            uint32_t box;

            if (!(FL(e, 0) < FL(e, 0xc) || FL(e, 0) > FL(e, 0xc)))     /* equal, or a NaN */
                continue;
            dx = box_abs((float)((double)FL(e, 0xc) - FL(e, 0)));
            dy = box_abs((float)((double)FL(e, 0x10) - FL(e, 4)));
            dz = box_abs((float)((double)FL(e, 0x14) - FL(e, 8)));
            sx = (float)((((double)dx + GFL(0x1012bacc)) - GFL(0x1012bad0)) / GFL(0x1012bacc));
            sy = (float)((((double)dy + GFL(0x1012bacc)) - GFL(0x1012bad0)) / GFL(0x1012bacc));
            sz = (float)((((double)dz + GFL(0x1012bacc)) - GFL(0x1012bad0)) / GFL(0x1012bacc));
            Sim_100e2776(sx, sy, sz);
            Sim_100e282a(0);
            count = 0;
            G32(0x101a10e0 + (uint32_t)i * 4) = (uint32_t)(uintptr_t)Sim_100e2a31(data, &count, (int32_t)size, 0);
            box = G32(0x101a10e0 + (uint32_t)i * 4);
            if (box == 0)
                continue;
            result = 1;
            Sim_1002d890((void *)(uintptr_t)box, 4);
            Sim_10060656((uint32_t)(uintptr_t)m, 0, 0, 0, U32(e, 0x18), U32(e, 0x1c), U32(e, 0x20));
            Mat3x4_Multiply((const float *)(e + 0x48), m, m);
            Sim_10009c55((void *)(uintptr_t)box, m);
            W8Y_CALL_10089e91(box);
        }
        free_data(key, data);
        return result;
    }
    G32(0x101a1164) = 0;
    result = 1;
    for (i = 0; i < GS32(0x1017d220); i++) {
        if (G32(0x101a10e0 + (uint32_t)i * 4) != 0) {
            Sim_10009787((void *)(uintptr_t)G32(0x101a10e0 + (uint32_t)i * 4));
            G32(0x101a10e0 + (uint32_t)i * 4) = 0;
        }
    }
    return result;
}

// FUNCTION: HGSIM 0x100b581d
void __cdecl Sim_100b581d(void *root)
{
    uint8_t key[0x10];
    uint32_t size;
    uint32_t data;

    unitbox_key(key, 0x1015087c);
    data = W8Y_CALL_100b8d6c(key, G32(0x1014cedc), G32(0x1014cf48), 2, &size, 0);
    if (data == 0)
        return;
    Sim_100b58c5(root, data, (int32_t)size);
    free_data(key, data);
}

// FUNCTION: HGSIM 0x100b58c5
void __cdecl Sim_100b58c5(void *node, uint32_t data, int32_t size)
{
    float dx, dy, dz, sx, sy, sz;
    float m[12];
    int32_t count;
    uint32_t box;
    int i;

    if (node == 0)
        return;
    dx = box_abs((float)((double)FL(node, 4) - FL(node, 0)));
    dy = box_abs((float)((double)FL(node, 0xc) - FL(node, 8)));
    dz = box_abs((float)((double)FL(node, 0x14) - FL(node, 0x10)));
    sx = (float)((((double)dx + GFL(0x1012bacc)) - GFL(0x1012bad0)) / GFL(0x1012bacc));
    sy = (float)((((double)dy + GFL(0x1012bacc)) - GFL(0x1012bad0)) * GFL(0x1012bab8) / GFL(0x1012bacc));
    sz = (float)((((double)dz + GFL(0x1012bacc)) - GFL(0x1012bad0)) / GFL(0x1012bacc));
    Sim_100e2776(sx, sy, sz);
    Sim_100e282a(4);
    count = 0;
    box = (uint32_t)(uintptr_t)Sim_100e2a31(data, &count, size, 0);
    if (box != 0) {
        float cx = (float)(((double)FL(node, 0) + FL(node, 4)) / GFL(0x1012bab8));
        float cy = (float)(((double)FL(node, 8) + FL(node, 0xc)) / GFL(0x1012bab8));
        float cz = (float)(((double)FL(node, 0x10) + FL(node, 0x14)) / GFL(0x1012bab8));

        Sim_1002d890((void *)(uintptr_t)box, 4);
        Sim_10060656((uint32_t)(uintptr_t)m, 0, 0, 0, Bits_OfFloat(cx), Bits_OfFloat(cy), Bits_OfFloat(cz));
        Sim_10009c55((void *)(uintptr_t)box, m);
        W8Y_CALL_10089e91(box);
    }
    for (i = 0; i < 4; i++)
        Sim_100b58c5((void *)(uintptr_t)U32(node, 0x1c + i * 4), data, size);
}
