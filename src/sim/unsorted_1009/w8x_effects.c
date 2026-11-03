#include "w8x.h"
#include "../fx/w32.h"
#include "w2g.h"
#include "w2j.h"
#include "../unsorted_1000/w2j.h"
#include "../unsorted_1000/w4w.h"
#include "../unsorted_1000/w5y.h"
#include "../unsorted_1008/w8x.h"
#include "../unsorted_1008/w3o.h"
#include "../unsorted_100a/w5a.h"
#include "../unsorted_100a/w5b.h"
#include "../unsorted_100a/w5x.h"
#include "../unsorted_100a/w5y.h"
#include "../unsorted_100a/w5z.h"
#include "../unsorted_100a/w6g.h"
#include "../util/access.h"
#include "../util/util.h"
#include "../util/w3o.h"

typedef void(__cdecl *W8xDriverPolyFn)(uint32_t count, void *vertices, uint32_t flags);
#define DRIVER_POLY (*SIM_PTR(W8xDriverPolyFn, 0x1014cb4c))
#define ONE 0x3f800000u
#define W8X_ADDR(a) ((uint32_t)(uintptr_t)SIM_PTR(uint8_t, (a)))

/* (float)(a + b) of two floats, rounded once like the x87 add and float store. */
static float W8x_Add(float a, float b)
{
    return (float)((double)a + (double)b);
}

/* The fields of a 36-byte driver vertex that these effects set (+0x14 and +0x18 stay unset). */
static void W8x_Vertex(uint32_t *v, float x, float y)
{
    v[0] = Bits_OfFloat(x);
    v[1] = Bits_OfFloat(y);
    v[4] = 0xf;
    v[7] = 0;
    v[8] = ONE;
    v[3] = ONE;
}

/* A screen quad (x, y) .. (x + dx, y + dy) in the original's vertex order; with `z` the depth is set too. */
static void W8x_Quad(uint32_t *quad, float x, float y, float x1, float y1, const float *z)
{
    W8x_Vertex(quad + 27, x, y);
    W8x_Vertex(quad + 18, x1, y);
    W8x_Vertex(quad + 9, x1, y1);
    W8x_Vertex(quad, x, y1);
    if (z != 0) {
        quad[27 + 2] = Bits_OfFloat(*z);
        quad[18 + 2] = Bits_OfFloat(*z);
        quad[9 + 2] = Bits_OfFloat(*z);
        quad[2] = Bits_OfFloat(*z);
    }
}

// FUNCTION: HGSIM 0x100908b3
void __cdecl Sim_100908b3(void)
{
    uint16_t i;
    int failed = 0;

    G32(0x1017cc2c) = 0;
    G32(0x1017cc28) = G32(0x1017cc2c);
    G32(0x1017cc24) = 0;
    for (i = 0; i < 1000 && !failed; i++) {
        uint32_t d, e;
        void *obj = Sim_10008f1c(3, 1, 0xc, &d);
        void *rec, *node;
        int j;

        if (obj == 0) {
            failed = 1;
            continue;
        }
        G32(0x1017cc20) += 1;
        Sim_10009136(obj, 0, 0, 0, 0, 0, 0, 0, 0, 0, ONE, 0);
        Sim_10009136(obj, 0, 0x42c80000, 0, 0x42c80000, 0, 0, 0, 0, 0, ONE, 0);
        Sim_10009136(obj, 0x42c80000, 0, 0, 0, 0x42c80000, 0, 0, 0, 0, ONE, 0);
        rec = Sim_100091f7(obj, (unsigned short)(i | 0x8000), d);
        U32(rec, 0x20) = ONE;
        U32(rec, 0x14) = ONE;
        Sim_10009278(obj, rec, 0);
        Sim_10009278(obj, rec, 1);
        Sim_10009278(obj, rec, 2);
        node = Sim_100a9830(0, 0xc);
        Sim_100a9b4c(node, (uint32_t)(uintptr_t)obj);
        Sim_100096f9(obj, (uint32_t)(uintptr_t)node);
        Sim_10009408(obj, 0x60);
        Sim_100094ba(obj);
        Sim_100a9d47(node, 0, 0, 0);
        Sim_100aa742(node);
        Sim_10089e91(obj);
        Sim_100aa351(node);
        Sim_100aa406(node);
        SIM_PTR(u32a, 0x10216b80)[i] = i;
        e = W8X_ADDR(0x10217c20) + (uint32_t)i * W32_REC_SIZE;
        U32(e, 8) = i;
        U32(e, 0xc) = 0;
        U32(e, W32_REC_OBJ) = (uint32_t)(uintptr_t)node;
        U32(e, W32_REC_UPDATE) = 0;
        U32(e, W32_REC_DRAW) = 0;
        U32(e, 0) = 0;
        U32(e, 4) = 0;
        U32(e, W32_REC_ARG) = 0;
        /* 1.2 (0x1007986e): particle records, 16 particles of 0x32 bytes and the dword +0x3fa -1 */
#ifndef W32S_MUTATE /* the checker's negative check only: +0x3fa left as it was */
        if (W32_PARTICLES())
            U32(e, 0x3fa) = 0xffffffffu;
#endif
        for (j = 0; j < (W32_PARTICLES() ? 0x10 : 0x20); j++) {
            uint32_t p = e + (uint32_t)j * (W32_PARTICLES() ? 0x32u : 0x2au);

            U32(p, 0x1a) = 0;
            U32(p, 0x16) = 0;
            U32(p, 0x12) = 0;
            U16(p, 0x2a) = 0;
        }
    }
    G32(0x10215ae8) = G32(0x1017cc20) - 1;
    Sim_100905e8();
    if (W32_PARTICLES())
        Sim12_100790be();   /* 1.2: the particle types' textures */
}

// FUNCTION: HGSIM 0x10091223
int __cdecl Sim_10091223(uint8_t *obj)
{
    uint32_t bx, by, bz;
    uint32_t line[2 * 9];
    int i;

    if (obj == 0)
        return 0;
    Sim_100a9cfb((void *)(uintptr_t)U32(obj, 0x552), &bx, &by, &bz);
    for (i = 0; i < 8; i++) {
        uint8_t *p = obj + i * 0x2a + 0x12;
        uint8_t *q = obj + (i + 1) * 0x2a + 0x12;
        float ax, ay, az, jx, jy, jz, cx, cy, cz;
        int32_t r;

        ax = W8x_Add(Float_OfBits(bx), FL(p, 0));
        ay = W8x_Add(Float_OfBits(by), FL(p, 4));
        az = W8x_Add(Float_OfBits(bz), FL(p, 8));
        r = Sim_1009c929(100);
        jx = (float)((double)r + (double)W8x_Add(Float_OfBits(bx), FL(p, 0)) - (double)GFL(0x1012b93c));
        r = Sim_1009c929(100);
        jy = (float)((double)r + (double)W8x_Add(Float_OfBits(by), FL(p, 4)) - (double)GFL(0x1012b93c));
        r = Sim_1009c929(100);
        jz = (float)((double)r + (double)W8x_Add(Float_OfBits(bz), FL(p, 8)) - (double)GFL(0x1012b93c));
        cx = W8x_Add(Float_OfBits(bx), FL(q, 0));
        cy = W8x_Add(Float_OfBits(by), FL(q, 4));
        cz = W8x_Add(Float_OfBits(bz), FL(q, 8));
        if (W8X9_CALL_100d2669(&ax, &ay, &az) == 0)
            continue;
        W8X9_CALL_100d2669(&cx, &cy, &cz);
        W8X9_CALL_100d2669(&jx, &jy, &jz);
        W8x_Vertex(line + 9, jx, jy);
        line[9 + 2] = Bits_OfFloat(jz);
        W8x_Vertex(line, ax, ay);
        line[2] = Bits_OfFloat(az);
        DRIVER_POLY(2, line, U32(obj, 0x566) | 0xa000);
        W8x_Vertex(line + 9, cx, cy);
        line[9 + 2] = Bits_OfFloat(cz);
        W8x_Vertex(line, jx, jy);
        line[2] = Bits_OfFloat(jz);
        DRIVER_POLY(2, line, U32(obj, 0x566) | 0xa000);
    }
    return 1;
}

// FUNCTION: HGSIM 0x10092bbb
int __cdecl Sim_10092bbb(uint8_t *obj)
{
    uint32_t bx, by, bz, colour;
    uint32_t quad[4 * 9];
    float px, py, pz;
    int i;

    if (obj == 0)
        return 0;
    Sim_100a9cfb((void *)(uintptr_t)U32(obj, 0x552), &bx, &by, &bz);
    px = 0.0f;
    pz = Float_OfBits(bz);
    colour = U32(obj, 0x566);
    W8X9_CALL_100d2669(&px, &px, &pz);
    for (i = 0; i < 0x10; i++) {
        uint8_t *p = obj + i * 0x2a;
        int32_t size;
        float x1, y1;

        px = W8x_Add(Float_OfBits(bx), FL(p, 0x12));
        py = W8x_Add(Float_OfBits(by), FL(p, 0x16));
        pz = W8x_Add(Float_OfBits(bz), FL(p, 0x1a));
        if (W8X9_CALL_100d2669(&px, &py, &pz) == 0)
            continue;
        size = Render_Ftol((double)(int32_t)(U32(obj, 0x56a) * 0x7d0u) / (double)pz);
        if (size < 1)
            size = 1;
        x1 = (float)((double)size + (double)px);
        y1 = (float)((double)size + (double)py);
        W8x_Quad(quad, px, py, x1, y1, &pz);
        DRIVER_POLY(4, quad, colour);
    }
    return 1;
}

// FUNCTION: HGSIM 0x10093e5b
int __cdecl Sim_10093e5b(uint8_t *obj)
{
    uint32_t bx, by, bz;
    uint32_t quad[4 * 9];
    float px, py, pz;
    int i;

    if (obj == 0)
        return 0;
    Sim_100a9cfb((void *)(uintptr_t)U32(obj, 0x552), &bx, &by, &bz);
    for (i = 0; i < 0x10; i++) {
        uint8_t *p = obj + i * 0x2a;
        uint32_t colour;
        int32_t size;
        float x1, y1;

        px = W8x_Add(FL(p, 0x12), Float_OfBits(bx));
        py = W8x_Add(FL(p, 0x16), Float_OfBits(by));
        pz = W8x_Add(FL(p, 0x1a), Float_OfBits(bz));
        colour = U32(p, 0x30);
        if (W8X9_CALL_100d2669(&px, &py, &pz) == 0)
            continue;
        size = Render_Ftol((double)(int32_t)(U32(obj, 0x56a) * 0x7d0u) / (double)pz);
        if (size < 1)
            size = 1;
        x1 = (float)((double)size + (double)px);
        y1 = (float)((double)size + (double)py);
        W8x_Quad(quad, px, py, x1, y1, 0);
        DRIVER_POLY(4, quad, colour);
    }
    return 1;
}

// FUNCTION: HGSIM 0x10096163
int __cdecl Sim_10096163(uint8_t *obj)
{
    uint32_t bx, by, bz, colour;
    uint32_t quad[4 * 9];
    float px, py, pz, size = 1.0f;
    int i, step;

    if (obj == 0)
        return 0;
    Sim_100a9cfb((void *)(uintptr_t)U32(obj, 0x552), &bx, &by, &bz);
    px = 0.0f;
    pz = Float_OfBits(bz);
    W8X9_CALL_100d2669(&px, &px, &pz);
    colour = U32(obj, 0x566);
    step = (pz > GFL(0x1012b990)) ? 2 : 1;
    for (i = 0; i < 0x10; i += step) {
        uint8_t *p = obj + i * 0x2a;

        px = W8x_Add(FL(p, 0x12), Float_OfBits(bx));
        py = W8x_Add(FL(p, 0x16), Float_OfBits(by));
        pz = W8x_Add(FL(p, 0x1a), Float_OfBits(bz));
        if (W8X9_CALL_100d2669(&px, &py, &pz) == 0)
            continue;
        W8x_Quad(quad, px, py, W8x_Add(px, size), W8x_Add(py, size), 0);
        DRIVER_POLY(4, quad, colour);
    }
    return 1;
}

// FUNCTION: HGSIM 0x10096869
int __cdecl Sim_10096869(uint8_t *obj)
{
    uint32_t bx, by, bz, colour;
    uint32_t quad[4 * 9];
    float px, py, pz;
    int i;

    if (obj == 0)
        return 0;
    Sim_100a9cfb((void *)(uintptr_t)U32(obj, 0x552), &bx, &by, &bz);
    px = 0.0f;
    pz = Float_OfBits(bz);
    W8X9_CALL_100d2669(&px, &px, &pz);
    colour = U32(obj, 0x566);
    for (i = 0; i < 10; i++) {
        uint8_t *p = obj + i * 0x2a;

        px = W8x_Add(FL(p, 0x12), Float_OfBits(bx));
        py = W8x_Add(FL(p, 0x16), Float_OfBits(by));
        pz = W8x_Add(FL(p, 0x1a), Float_OfBits(bz));
        if (W8X9_CALL_100d2669(&px, &py, &pz) == 0)
            continue;
        W8x_Quad(quad, px, py, W8x_Add(px, GFL(0x1012b970)), W8x_Add(py, GFL(0x1012b970)), 0);
        DRIVER_POLY(4, quad, colour);
    }
    return 1;
}

// FUNCTION: HGSIM 0x10092f9d
void __cdecl Sim_10092f9d(float x, float y, float z, float vx, float vy, float vz, int32_t k)
{
    uint8_t *e, *blk;
    float sx = 0.0f, sy = 0.0f, sz = 0.0f, speed;
    const float step = 0.0f;
    double dist;
    int32_t r;
    int i;

    /* 1.2 (round 32): the shockwave particles instead */
    if (W32_PARTICLES()) {
        Sim12_1007c1a1(x, y, z, vx, vy, vz, k);
        return;
    }
    if (!((int32_t)(G32(0x1017cc24) + 1) < GS32(0x1017cc20)) || k == 0)
        return;
    dist = Sim_10084a94((float)((double)x - (double)FL(G32(0x1014cae0), 0)),
                        (float)((double)y - (double)FL(G32(0x1014cae0), 4)),
                        (float)((double)z - (double)FL(G32(0x1014cae0), 8)));
    if (dist > (double)FL(G32(0x1014cae0), 0x40))
        return;
    e = Sim_10090c7d(W8X_ADDR(0x10002248), W8X_ADDR(0x10002ff4), 0, 0);
    if (e == 0)
        return;
    U16(e, 0x10) = 1;
    blk = BYTES(U32(U32(e, 0x552), 0x8c));
    FL(blk, 0x50) = (float)Sim_10084a94(vx, vy, vz);
    FL(blk, 0x44) = x;
    FL(blk, 0x48) = y;
    FL(blk, 0x4c) = z;
    U32(blk, 0x38) = 0;
    U32(blk, 0x3c) = 0;
    U32(blk, 0x40) = 0;
    U32(e, 0x56e) = 0;
    U32(e, 0x572) = 0;
    U32(e, 0x576) = 0;
    Sim_100aa39f((void *)(uintptr_t)U32(e, 0x552));
    Sim_100a9d47((void *)(uintptr_t)U32(e, 0x552), Bits_OfFloat(x), Bits_OfFloat(y), Bits_OfFloat(z));
    Sim_100aa799((void *)(uintptr_t)U32(e, 0x552));
    G32(0x1017cc24) += 1;
    r = Sim_1009c929((int)((uint32_t)k * 0x50));
    speed = (float)(int32_t)((uint32_t)k * 0x50 + (uint32_t)r);
    U32(e, 0x55e) = G32(0x1015dc78);
    U32(e, 0x562) = (uint32_t)k * 0xb5;
    for (i = 0; i < 0x10; i++) {
        uint8_t *p = e + i * 0x2a + 0x12;
        float angle;

        U16(p, 0x18) = 1;
        sx = W8x_Add(sx, step);
        sy = W8x_Add(sy, step);
        sz = W8x_Add(sz, step);
        r = Sim_1009c929(10);
        FL(p, 0) = (float)((double)r + (double)sx - (double)GFL(0x1012b948));
        r = Sim_1009c929(0x1e);
        FL(p, 4) = (float)((double)r + (double)sy - (double)GFL(0x1012b954));
        r = Sim_1009c929(10);
        FL(p, 8) = (float)((double)r + (double)sz - (double)GFL(0x1012b948));
        r = Sim_1009c929(0x168);
        angle = (float)r;
        FL(p, 0xc) = (float)(W8X9_CALL_10084766((float)((double)r + (double)GFL(0x1012b940))) * (double)speed);
        U32(p, 0x10) = 0;
        FL(p, 0x14) = (float)(W8X9_CALL_10084766(angle) * (double)speed);
    }
    U32(e, 0xc) = 1;
}
