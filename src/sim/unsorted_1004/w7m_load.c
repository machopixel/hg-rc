#include "w7m.h"
#include "../unsorted_100b/w6g.h"
#include "../unsorted_1002/w2j.h"

typedef int(__stdcall *W7mHeapFreeFn)(uint32_t heap, uint32_t flags, void *block);

/* Releases a loaded record: with HeapFree when the key word is -1, else through Sim_100bb2b0 (name at key + 2). */
static void Release(const int16_t *key, void *data, uint32_t table)
{
    if (key[0] == -1)
        SIM_API(W7mHeapFreeFn, 0x10ac5634)(G32(0x1017b8f8), 1, data);
    else
        Sim_100bb2b0((const uint32_t *)(key + 1), (const char *)(uintptr_t)table);
}

// FUNCTION: HGSIM 0x100462c0
int __cdecl Sim_100462c0(const int16_t *key, float *a, float *b, float *c, float *d, float *e, float *f, float *g)
{
    uint32_t out;
    int32_t *data = W7M_CALL_100b8d6c(key, G32(0x1014cee8), G32(0x1014cf54), 7, &out, 0);

    if (data == 0)
        return 0;
    *a = (float)data[0];
    *b = (float)data[1];
    *c = (float)data[2];
    *d = (float)data[3];
    *e = (float)data[4];
    *f = (float)((double)data[5] * (double)W7M_GF(0x101115d4));
    *g = (float)data[6];
    Release(key, data, G32(0x1014cee8));
    return 1;
}

// FUNCTION: HGSIM 0x100466b6
int __cdecl Sim_100466b6(const int16_t *key)
{
    uint32_t out;
    uint32_t *data = W7M_CALL_100b8d6c(key, G32(0x1014ceec), G32(0x1014cf58), 6, &out, 0);
    uint32_t *p;
    int32_t v[4];
    int i, j;

    if (data == 0)
        return 0;
    p = data + 10;  /* the first five pairs of dwords are not used */
    for (i = 0; i < 3; i++)
        SIM_PTR(u32a, 0x1019a070)[i] = *p++;
    for (i = 0; i < 15; i++) {
        v[0] = (int32_t)*p++;
        v[1] = (int32_t)*p++;
        v[2] = (int32_t)*p++;
        v[3] = (int32_t)*p++;
        if (v[0] < 0 || v[0] > 100 || v[1] < 0 || v[1] > 100 || v[2] < 0 || v[2] > 100 || v[3] < 0 || v[3] > 100)
            v[0] = v[1] = v[2] = v[3] = 0;
        for (j = 0; j < 8; j++) {
            uint32_t at = 0x10199464 + (uint32_t)j * 0x140 + (uint32_t)i * 0x14;

            *SIM_PTR(u32a, at) = (uint32_t)v[0];
            *SIM_PTR(u32a, at + 4) = (uint32_t)v[1];
            *SIM_PTR(u32a, at + 8) = (uint32_t)v[2];
            *SIM_PTR(u32a, at + 0xc) = (uint32_t)v[3];
        }
    }
    Release(key, data, G32(0x1014ceec));
    return 1;
}

// FUNCTION: HGSIM 0x100468c3
int __cdecl Sim_100468c3(const int16_t *key, uint32_t *rects, uint32_t *pair)
{
    uint32_t out;
    uint8_t *data = W7M_CALL_100b8d6c(key, G32(0x1014cef0), G32(0x1014cf5c), 5, &out, 0);
    uint8_t *p;
    int i;

    if (data == 0)
        return 0;
    /* 19 rectangles of four signed words (x, y, width, height) into 0x14-byte records as x, y, right, bottom */
    p = data + 0x28;
    for (i = 0; i < 0x13; i++) {
        rects[1] = (uint32_t)(int32_t)S16(p, 0);
        rects[2] = (uint32_t)(int32_t)S16(p, 2);
        rects[3] = (uint32_t)((int32_t)S16(p, 0) + (int32_t)S16(p, 4) - 1);
        rects[4] = (uint32_t)((int32_t)S16(p, 2) + (int32_t)S16(p, 6) - 1);
        p += 8;
        rects += 5;
    }
    pair[0] = (uint32_t)(int32_t)S16(p, 0);
    pair[1] = (uint32_t)(int32_t)S16(p, 2);
    if (key[0] == -1)
        SIM_API(W7mHeapFreeFn, 0x10ac5634)(G32(0x1017b8f8), 1, data);
    else
        Sim_1002828e(key[0], (const void *)(uintptr_t)G32(0x1014cef0));
    return 1;
}
