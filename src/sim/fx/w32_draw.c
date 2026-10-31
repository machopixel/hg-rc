#include "w32.h"
#include "../unsorted_1000/w2l.h"
#include "../util/util.h"
#include "../util/access.h"

typedef float HG_MAY_ALIAS w32f;
typedef int(__cdecl *W32DriverFn)(int32_t count, void *verts, uint32_t flags);

#define W32_FL(p, off) (*(w32f *)((uint8_t *)(p) + (off)))
/* the C library qsort, still original */
#define W32_CALL_QSORT SIM_FN(void(__cdecl *)(void *, uint32_t, uint32_t, int(__cdecl *)(const void *, const void *)), \
                              0x100fdcc0)

/* 1.2 0x1011e998: the corners' texture coordinates (u, v), read from a random start (1.2's image) */
const uint32_t g_w32_1011e998[16] = {
    0x00000000, 0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000, 0x00000000, 0x3f800000, 0x3f800000,
};

// FUNCTION: HGSIM12 0x10078be0
int __cdecl Sim12_10078be0(const void *a, const void *b)
{
    const uint8_t *pa = (const uint8_t *)a, *pb = (const uint8_t *)b;

    if (U32(pa, 0xc) == 0 || U32(pb, 0xc) == 0)
        return 0;
    /* fcomp / test ah, 0x41: -1 when b <= a or unordered */
    return W32_FL(pb, 8) > W32_FL(pa, 8) ? 1 : -1;
}

// FUNCTION: HGSIM12 0x10078c29
void __cdecl Sim12_10078c29(uint8_t *rec, uint8_t *points, uint32_t flags, int32_t count)
{
    uint8_t *v = rec + 0x36a, *p;
    float x, y, half;
    uint32_t frame;
    int32_t i, k;

    if ((flags & 0x1ff) != 0)
        W32_CALL_QSORT(points, (uint32_t)count, 0x10, Sim12_10078be0);
    for (i = 0; i < count; i++) {
        p = points + i * 0x10;
        if (U32(p, 0xc) == 0)
            continue;
        frame = U32(rec + i * 0x32, 0x30);
        half = (float)((double)S32(rec + i * 0x32, 0x38) / 2.0);
        y = W32_FL(p, 4);
        x = W32_FL(p, 0);
        k = Sim_1009c929(4);
        U32(v, 0x1c) = g_w32_1011e998[k * 2];
        U32(v, 0x20) = g_w32_1011e998[k * 2 + 1];
        k++;
        U32(v, 0x40) = g_w32_1011e998[k * 2];
        U32(v, 0x44) = g_w32_1011e998[k * 2 + 1];
        k++;
        U32(v, 0x64) = g_w32_1011e998[k * 2];
        U32(v, 0x68) = g_w32_1011e998[k * 2 + 1];
        k++;
        U32(v, 0x88) = g_w32_1011e998[k * 2];
        U32(v, 0x8c) = g_w32_1011e998[k * 2 + 1];
        W32_FL(v, 0) = (float)((double)x - (double)half);
        W32_FL(v, 4) = (float)((double)y + (double)half);
        W32_FL(v, 0x24) = (float)((double)x + (double)half);
        W32_FL(v, 0x28) = (float)((double)y + (double)half);
        W32_FL(v, 0x48) = (float)((double)x + (double)half);
        W32_FL(v, 0x4c) = (float)((double)y - (double)half);
        W32_FL(v, 0x6c) = (float)((double)x - (double)half);
        W32_FL(v, 0x70) = (float)((double)y - (double)half);
#ifdef W32D_MUTATE /* the checker's negative check only: the frame is not set */
        if (0)
#else
        if ((int32_t)flags > 0x1ff)
#endif
            Sim_1000716a((int)(flags & 0x1ff), frame & 0xffffu);
        W32_PAINTER(1);
        ((W32DriverFn)(uintptr_t)G32(0x1014cb4c))(4, v, flags);
        W32_PAINTER(0);
    }
}
