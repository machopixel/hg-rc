/*
 * Round 32, F: 1.2's railgun (type 0, replaces 1.0's Sim_10091085), accelerator (2, Sim_1009201f) and laser (3,
 * Sim_10092956) beams: the systems follow a shot record (+0x366) and draw its two ends as two quads.
 */
#include "w32.h"
#include "../unsorted_1000/w2e.h"
#include "../unsorted_1000/w5x.h"
#include "../unsorted_1000/w2l.h"
#include "../unsorted_100a/w5b.h"
#include "../unsorted_100a/w5x.h"
#include "../unsorted_100a/w5y.h"
#include "../unsorted_100d/w8z.h"
#include "../util/util.h"
#include "../util/access.h"

#define W32B_NODE(rec) ((void *)(uintptr_t)U32(rec, 0x332))
/* the shot's impact, still called at its DLL address as in 1.0's railgun (1.2 calls its own, changed, version) */
#define W32B_CALL_100246b3 \
    SIM_FN(void(__cdecl *)(int32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, \
                           uint32_t, uint32_t, uint32_t), 0x100246b3)

typedef int(__cdecl *W32bDriverFn)(int32_t count, void *verts, uint32_t flags);

// FUNCTION: HGSIM12 0x10086750
void __cdecl Sim12_10086750(uint8_t *v)
{
    int32_t k = Sim_1009c929(4);

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
}

// FUNCTION: HGSIM12 0x10086810
void __cdecl Sim12_10086810(uint8_t *v, float x3, float y3, float x2, float y2, float x1, float y1, float x0, float y0,
                            uint32_t unused, uint32_t frame, uint32_t flags)
{
    (void)unused;
    Sim12_10086750(v);
    W32_F(v, 0) = x0;
    W32_F(v, 4) = y0;
    W32_F(v, 0x24) = x1;
    W32_F(v, 0x28) = y1;
    W32_F(v, 0x48) = x2;
    W32_F(v, 0x4c) = y2;
    W32_F(v, 0x6c) = x3;
    W32_F(v, 0x70) = y3;
    if ((int32_t)flags > 0x1ff)
        Sim_1000716a((int)(flags & 0x1ff), frame & 0xffffu);
    W32_PAINTER(1);
    ((W32bDriverFn)(uintptr_t)G32(0x1014cb4c))(4, v, flags);
    W32_PAINTER(0);
}

static float W32b_Add(float a, double c)
{
    return (float)((double)a + c);
}

// FUNCTION: HGSIM12 0x10079f7d
int __cdecl Sim12_10079f7d(uint8_t *rec)
{
    uint8_t *src = BYTES(U32(rec, 0x366));
    int32_t age = (int32_t)(G32(0x1015dc78) - U32(rec, 0x33e));

#ifdef W32X_MUTATE /* the checker's negative check only: one frame longer */
    if (age <= S32(rec, 0x342) + 1 && (U16(rec, 0x10) & 2) == 0) {
#else
    if (age <= S32(rec, 0x342) && (U16(rec, 0x10) & 2) == 0) {
#endif
        U32(rec, 0x12) = U32(src, 0xc);
        U32(rec, 0x16) = U32(src, 0x10);
        U32(rec, 0x1a) = U32(src, 0x14);
        Sim_100aa39f(W32B_NODE(rec));
        Sim_100a9d47(W32B_NODE(rec), U32(rec, 0x12), U32(rec, 0x22), U32(rec, 0x26));
        Sim_100aa799(W32B_NODE(rec));
        return 1;
    }
    W32B_CALL_100246b3(-2, 0, 0x49, U32(rec, 0x12), U32(rec, 0x16), U32(rec, 0x1a), 0, 0, 0, 0, 0, 0);
    Sim_100aa351(W32B_NODE(rec));
    U16(rec, 0x10) = (uint16_t)(U16(rec, 0x10) & 0xfff8);
    return 0;
}

// FUNCTION: HGSIM12 0x1007a092
int __cdecl Sim12_1007a092(uint8_t *rec)
{
    return rec != 0;
}

/* what the three beam spawns share after the system exists: the node block, the count, the start frame */
static void W32b_Start(uint8_t *rec)
{
    uint8_t *block = BYTES(U32(BYTES(U32(rec, 0x332)), 0x8c));

    U32(block, 0x50) = 0x3f800000u;
    U32(block, 0x44) = 0x3f800000u;
    U32(block, 0x48) = 0x3f800000u;
    U32(block, 0x4c) = 0x3f800000u;
    U32(block, 0x38) = 0;
    U32(block, 0x3c) = 0;
    U32(block, 0x40) = 0;
    G32(0x1017cc24) = G32(0x1017cc24) + 1;
    U32(rec, 0x33e) = G32(0x1015dc78);
#ifdef W32X_MUTATE /* the checker's negative check only */
    U16(rec, 0x12 + 0x18) = 2;
#else
    U16(rec, 0x12 + 0x18) = 1;
#endif
}

// FUNCTION: HGSIM12 0x1007a0a6
void __cdecl Sim12_1007a0a6(uint32_t arg, int unused, uint32_t param)
{
    uint8_t *rec;

    (void)unused;
    rec = W32_NewSystem((uint32_t)(uintptr_t)Sim12_10079f7d, (uint32_t)(uintptr_t)Sim12_1007a092, arg, 1, 0);
    if (rec == 0)
        return;
    U32(rec, 0x342) = param;
    U32(rec, 0x346) = 0;
    W32b_Start(rec);
    U32(rec, 0xc) = 0xd;
}

/* the accelerator's and the laser's update: the system follows its shot record's two ends */
static int W32b_Follow(uint8_t *rec, uint32_t end, int laser)
{
    uint8_t *src = BYTES(U32(rec, 0x366));

#ifdef W32X_MUTATE /* the checker's negative check only: one frame longer */
    if (S32(rec, 0x342) + 1 < (int32_t)(G32(0x1015dc78) - U32(rec, 0x33e)) || (U16(rec, 0x10) & 2) != 0) {
#else
    if (S32(rec, 0x342) < (int32_t)(G32(0x1015dc78) - U32(rec, 0x33e)) || (U16(rec, 0x10) & 2) != 0) {
#endif
        if (laser)
            Sim_1000ecb0(SIM_PTR(const char, 0x1014b9c4));        /* "AFXWLLLC" */
        Sim_100aa351(W32B_NODE(rec));
        U16(rec, 0x10) = (uint16_t)(U16(rec, 0x10) & 0xfff8);
        return 0;
    }
    U32(rec, 0x12) = U32(src, 0);
    U32(rec, 0x16) = U32(src, 4);
    U32(rec, 0x1a) = U32(src, 8);
    U32(rec, 0x1e) = U32(src, 0xc);
    U32(rec, 0x22) = U32(src, 0x10);
    U32(rec, 0x26) = U32(src, 0x14);
    Sim_100aa39f(W32B_NODE(rec));
    Sim_100a9d47(W32B_NODE(rec), U32(rec, end), U32(rec, end + 4), U32(rec, end + 8));
    Sim_100aa799(W32B_NODE(rec));
    return 1;
}

// FUNCTION: HGSIM12 0x1007a26c
int __cdecl Sim12_1007a26c(uint8_t *rec)
{
    return W32b_Follow(rec, 0x12, 0);
}

// FUNCTION: HGSIM12 0x1007aec2
int __cdecl Sim12_1007aec2(uint8_t *rec)
{
    return W32b_Follow(rec, 0x1e, 1);
}

// FUNCTION: HGSIM12 0x1007a379
int __cdecl Sim12_1007a379(uint8_t *rec)
{
    uint32_t outer, inner;
    float ax, ay, az, bx, by, bz, dx, dy;
    uint8_t *v;

    if (rec == 0)
        return 0;
    /* the frame advances twice; 1.2 adds 0x7000 after each, so a stopped animation still draws */
    outer = W32_ParticleFlags(rec, -1, (uint32_t)(S32(rec, 0x346) >> 1)) | 0x7000;
    if (outer == 0)
        return 0;
    inner = W32_ParticleFlags(rec, -1, U32(rec, 0x346)) | 0x7000;
    if (inner == 0)
        return 0;
    ax = W32_F(rec, 0x12);
    ay = W32_F(rec, 0x16);
    az = W32_F(rec, 0x1a);
    bx = W32_F(rec, 0x1e);
    by = W32_F(rec, 0x22);
    bz = W32_F(rec, 0x26);
    Sim_100d2669(&ax, &ay, &az);
    Sim_100d2669(&bx, &by, &bz);
    dx = (float)((double)ax - (double)bx);
    dy = (float)((double)ay - (double)by);
    if (!(dx >= 0.0f))                              /* fcomp 0 / test ah, 1: less, or unordered */
        dx = -dx;
    if (!(dy >= 0.0f))
        dy = -dy;
    v = rec + 0x36a;
    if (dx > dy) {
        /* across: two horizontal quads, 5 and then 2..3 high */
        Sim12_10086810(v, bx, W32b_Add(by, 5.0), ax, W32b_Add(ay, 5.0), ax, ay, bx, by, *(uint32_t *)&az, 0, outer);
        Sim12_10086810(v, bx, W32b_Add(by, 3.0), ax, W32b_Add(ay, 3.0), ax, W32b_Add(ay, 2.0), bx,
                       W32b_Add(by, 2.0), *(uint32_t *)&az, 0, inner);
    } else if (!(dx >= dy)) {                       /* test ah, 1: less, or unordered */
        /* up and down: two vertical quads */
        Sim12_10086810(v, W32b_Add(bx, 5.0), by, W32b_Add(ax, 5.0), ay, ax, ay, bx, by, *(uint32_t *)&az, 0, outer);
        Sim12_10086810(v, W32b_Add(bx, 3.0), by, W32b_Add(ax, 3.0), ay, W32b_Add(ax, 2.0), ay, W32b_Add(bx, 2.0), by,
                       *(uint32_t *)&az, 0, inner);
    } else {
#ifdef W32B_MUTATE /* the checker's negative check only: the outer quad one step thinner */
        Sim12_10086810(v, W32b_Add(bx, 4.0), W32b_Add(by, 5.0), W32b_Add(ax, 5.0), W32b_Add(ay, 5.0), ax, ay, bx, by,
                       *(uint32_t *)&az, 0, outer);
#else
        Sim12_10086810(v, W32b_Add(bx, 5.0), W32b_Add(by, 5.0), W32b_Add(ax, 5.0), W32b_Add(ay, 5.0), ax, ay, bx, by,
                       *(uint32_t *)&az, 0, outer);
#endif
        Sim12_10086810(v, W32b_Add(bx, 3.0), W32b_Add(by, 3.0), W32b_Add(ax, 3.0), W32b_Add(ay, 3.0),
                       W32b_Add(ax, 2.0), W32b_Add(ay, 2.0), W32b_Add(bx, 2.0), W32b_Add(by, 2.0), *(uint32_t *)&az, 0,
                       inner);
    }
    return 1;
}

/* 1.2 0x1007afdc is byte for byte 0x1007a379 */
// FUNCTION: HGSIM12 0x1007afdc
int __cdecl Sim12_1007afdc(uint8_t *rec)
{
    return Sim12_1007a379(rec);
}

// FUNCTION: HGSIM12 0x1007acf3
void __cdecl Sim12_1007acf3(uint32_t arg, int unused, uint32_t param, uint32_t param2)
{
    uint8_t *rec;

    (void)unused;
    rec = W32_NewSystem((uint32_t)(uintptr_t)Sim12_1007a26c, (uint32_t)(uintptr_t)Sim12_1007a379, arg, 1, 2);
    if (rec == 0)
        return;
    U32(rec, 0x342) = param;
    U32(rec, 0x346) = param2;
    W32b_Start(rec);
    U32(rec, 0xc) = 0xd;
}

// FUNCTION: HGSIM12 0x1007b956
void __cdecl Sim12_1007b956(uint32_t arg, const uint8_t *shot, uint32_t param)
{
    uint8_t *rec;

    rec = W32_NewSystem((uint32_t)(uintptr_t)Sim12_1007aec2, (uint32_t)(uintptr_t)Sim12_1007afdc, arg, 1, 3);
    if (rec == 0)
        return;
    U32(rec, 0x342) = U32(shot, 0x34) - U32(shot, 0x30);
    U32(rec, 0x346) = param;
    W32b_Start(rec);
    Sim_1000ec60(SIM_PTR(const char, 0x1014b9d0), 0xfffffc18u, 0, 5, 0x150);     /* "AFXWLLLC" */
    U32(rec, 0xc) = 0xd;
}
