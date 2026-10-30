#include "w7o.h"
#include "w2n.h"
#include "w3p.h"
#include "w3q.h"
#include "../util/access.h"
#include "../util/w3o.h"
#include "../util/util.h"
#include "../unsorted_100a/w2h.h"
#include "../unsorted_100a/w5z.h"

typedef void(__cdecl *W7oDriverPolyFn)(uint32_t count, void *vertices, uint32_t flags);
#define DRIVER_POLY (*SIM_PTR(W7oDriverPolyFn, 0x1014cb4c))
#define CAMERA BYTES(G32(0x1014cae0))
#define ONE 0x3f800000u

/* The fields of a 36-byte driver vertex that the particle quads set (+8, +0x14 and +0x18 stay unset). */
static void QuadVertex(uint32_t *v, uint32_t x, uint32_t y)
{
    v[0] = x;
    v[1] = y;
    v[4] = 0xf;
    v[7] = 0;
    v[8] = ONE;
    v[3] = ONE;
}

// FUNCTION: HGSIM 0x10089907
void __cdecl Sim_10089907(float x, uint32_t y, float z, float s, uint32_t flags)
{
    /* corners in the original's order: (x - s, z + s), (x + s, z + s), (x + s, z - s), (x - s, z - s) */
    static const signed char corner[4][2] = { { -1, 1 }, { 1, 1 }, { 1, -1 }, { -1, -1 } };
    unsigned char scene[4 * 0x58];
    uint32_t out[16 * 9];
    unsigned n, i;

    for (i = 0; i < 4; i++) {
        unsigned char *v = scene + i * 0x58;

        FL(v, 0xc) = (float)(corner[i][0] < 0 ? (double)x - (double)s : (double)x + (double)s);
        FL(v, 0x14) = (float)(corner[i][1] < 0 ? (double)z - (double)s : (double)z + (double)s);
        U32(v, 0x10) = y;
        U32(v, 0x18) = 0;
        U32(v, 0x1c) = 0;
        U32(v, 0x2c) = 0;
    }
    n = Render_ClipPolygon(scene, 4, U32(CAMERA, 0x3c), 0x49742400u);     /* far = 1e6 */
    if (n < 3)
        return;
    for (i = 0; i < n; i++) {
        uint32_t pv = SIM_PTR(u32a, 0x101c76a0)[i];
        uint32_t *v = out + i * 9;

        FL(v, 0) = (float)S32(pv, 0x18);
        FL(v, 4) = (float)S32(pv, 0x1c);
        v[2] = U32(pv, 8);
        v[3] = ONE;
        v[7] = ONE;
        v[8] = ONE;
        v[4] = 0;
        v[5] = 0xff;
    }
    DRIVER_POLY(n, out, flags | 0xa000);
}

// FUNCTION: HGSIM 0x10094d7f
int __cdecl Sim_10094d7f(uint8_t *obj)
{
    uint32_t bx, by, bz;
    uint32_t quad[4 * 9];
    uint32_t colour;
    float px, py, pz;
    int i;

    if (obj == 0)
        return 0;
    Sim_100a9cfb((void *)(uintptr_t)U32(obj, 0x552), &bx, &by, &bz);
    /* (the original also divides +0x56e, +0x572, +0x576 by the float at 0x1012b958 into locals it never reads) */
    colour = U32(obj, 0x566);
    for (i = 0; i < 0x10; i++) {
        uint8_t *p = obj + 0x12 + i * 0x2a;
        float c;

        px = (float)((double)FL(p, 0) + (double)Float_OfBits(bx));
        py = (float)((double)FL(p, 4) + (double)Float_OfBits(by));
        pz = (float)((double)FL(p, 8) + (double)Float_OfBits(bz));
        if (W7O_CALL_100d2669(&px, &py, &pz) == 0)
            continue;
        c = GFL(0x1012b970);
        QuadVertex(quad + 27, Bits_OfFloat(px), Bits_OfFloat(py));
        QuadVertex(quad + 18, Bits_OfFloat((float)((double)px + (double)c)), Bits_OfFloat(py));
        QuadVertex(quad + 9, Bits_OfFloat((float)((double)px + (double)c)),
                   Bits_OfFloat((float)((double)py + (double)c)));
        QuadVertex(quad, Bits_OfFloat(px), Bits_OfFloat((float)((double)py + (double)c)));
        DRIVER_POLY(4, quad, colour);
    }
    return 1;
}

// FUNCTION: HGSIM 0x10095453
int __cdecl Sim_10095453(uint8_t *obj)
{
    uint32_t quad[4 * 9];
    uint32_t colour;
    int32_t size;
    float px, py, pz;
    int i;

    if (obj == 0)
        return 0;
    colour = U32(obj, 0x566);
    size = S32(obj, 0x56a);
    for (i = 0; i < 0x10; i++) {
        uint8_t *p = obj + 0x12 + i * 0x2a;

        px = FL(p, 0);
        py = FL(p, 4);
        pz = FL(p, 8);
        if (W7O_CALL_100d2669(&px, &py, &pz) == 0)
            continue;
        QuadVertex(quad + 27, Bits_OfFloat(px), Bits_OfFloat(py));
        QuadVertex(quad + 18, Bits_OfFloat((float)((double)size + (double)px)), Bits_OfFloat(py));
        QuadVertex(quad + 9, Bits_OfFloat((float)((double)size + (double)px)),
                   Bits_OfFloat((float)((double)size + (double)py)));
        QuadVertex(quad, Bits_OfFloat(px), Bits_OfFloat((float)((double)size + (double)py)));
        DRIVER_POLY(4, quad, colour);
    }
    return 1;
}

// FUNCTION: HGSIM 0x100a4510
int __cdecl Sim_100a4510(void *obj, float range)
{
    uint32_t lod = 0, prev = 0, e, verts, poly, p, node;
    int32_t n;

    if (obj == 0)
        return 1;
    e = U32(obj, 0x2c);
    if (e == 0)
        return 1;
    if (!(FL(e, 0) != GFL(0x1012ba68))) {     /* equal, or a NaN */
        lod = e;
    } else {
        for (prev = e; e != 0; e = U32(e, 0xc)) {
            if ((double)FL(CAMERA, 0xb8) * (double)FL(e, 0) > (double)range) {
                lod = prev;
                break;
            }
            prev = e;
        }
    }
    if (lod == 0)
        lod = prev;
    if (lod == 0)
        return 1;
    U32(obj, 0x30) = lod;
    verts = lod + 0x18;
    if (U32(obj, 0x9c) != U32(lod, 0x10))
        Sim_100aa307(obj);
    for (n = S16(lod, 4), p = verts; n-- != 0; p += 0x58) {
        U32(p, 0x2c) = 0;
        U8(p, 0x30) = (uint8_t)(U8(p, 0x30) & 0xfb);
    }
    node = U32(obj, 0x28);
    if (node != 0)
        Sim_1005874a(BYTES(node) + 0x4c);
    else
        Sim_1005874a(SIM_PTR(void, 0x1014ccf0));
    poly = lod + U32(lod, 8);
    G32(0x101cc2b0) = U16(obj, 0) & 0x10f;
    n = S16(lod, 6);
    G32(0x1017d12c) = U32(obj, 0xb0);
    while (n-- != 0) {
        W7O_CALL_100a65e7((void *)(uintptr_t)poly, (void *)(uintptr_t)verts);
        poly += 0x28;
        if (G32(0x101c9ea4) == 0)
            break;
    }
    G32(0x1017d12c) = 0;
    U32(obj, 0xb0) = 0;
    return 0;
}

/* The fields the edge clippers interpolate. */
struct EdgeEnd {
    float x, y, z, a, b, f, c, d, e;    /* +0, +4, +8, +0x20, +0x24, +0x2c, +0x30, +0x34, +0x38 */
};

static void LoadEnd(struct EdgeEnd *v, const void *p)
{
    v->x = FL(p, 0);
    v->y = FL(p, 4);
    v->z = FL(p, 8);
    v->a = FL(p, 0x20);
    v->b = FL(p, 0x24);
    v->c = FL(p, 0x30);
    v->d = FL(p, 0x34);
    v->e = FL(p, 0x38);
    v->f = FL(p, 0x2c);
}

/* (to - from) * t / d + from (near plane) */
static float LerpDiv(float from, float to, float t, float d)
{
    return (float)(((double)to - (double)from) * (double)t / (double)d + (double)from);
}

/* (to - from) * t * k + from (far plane) */
static float LerpMul(float from, float to, float t, float k)
{
    return (float)(((double)to - (double)from) * (double)t * (double)k + (double)from);
}

// FUNCTION: HGSIM 0x100a5d38
void *__cdecl Render_ClipEdgeNear(void *a, const uint32_t *screenA, void *b, const uint32_t *screenB)
{
    struct EdgeEnd p, q;
    uint8_t *r;

    LoadEnd(&p, Sim_100a5c10(a, screenA));
    LoadEnd(&q, Sim_100a5c10(b, screenB));
    if (!(q.z > p.z)) {
        double dd = (double)p.z - (double)q.z;     /* the C library call gets the unrounded difference */
        float d = (float)dd;

        if (W7O_CALL_100fcb70_R(dd) >= (double)GFL(0x1012ba74)) {
            float t = (float)((double)GFL(0x10185c50) - (double)q.z);

            p.x = LerpDiv(q.x, p.x, t, d);
            p.y = LerpDiv(q.y, p.y, t, d);
            p.a = LerpDiv(q.a, p.a, t, d);
            p.b = LerpDiv(q.b, p.b, t, d);
            p.f = LerpDiv(q.f, p.f, t, d);
            p.c = LerpDiv(q.c, p.c, t, d);
            p.d = LerpDiv(q.d, p.d, t, d);
            p.e = LerpDiv(q.e, p.e, t, d);
        }
    } else {
        double dd = (double)q.z - (double)p.z;
        float d = (float)dd;

        if (W7O_CALL_100fcb70_R(dd) >= (double)GFL(0x1012ba74)) {
            float t = (float)((double)GFL(0x10185c50) - (double)p.z);

            p.x = LerpDiv(p.x, q.x, t, d);
            p.y = LerpDiv(p.y, q.y, t, d);
            p.f = LerpDiv(p.f, q.f, t, d);
            p.a = LerpDiv(p.a, q.a, t, d);
            p.b = LerpDiv(p.b, q.b, t, d);
            p.c = LerpDiv(p.c, q.c, t, d);
            p.d = LerpDiv(p.d, q.d, t, d);
            p.e = LerpDiv(p.e, q.e, t, d);
        }
    }
    r = BYTES(Sim_100a88ff());
    FL(r, 0) = p.x;
    FL(r, 4) = p.y;
    U32(r, 8) = G32(0x10185c50);
    FL(r, 0x20) = p.a;
    FL(r, 0x24) = p.b;
    FL(r, 0x30) = p.c;
    FL(r, 0x34) = p.d;
    FL(r, 0x38) = p.e;
    FL(r, 0x2c) = p.f;
    return r;
}

// FUNCTION: HGSIM 0x100a6022
void *__cdecl Render_ClipEdgeFar(void *a, void *b)
{
    struct EdgeEnd p, q;
    uint8_t *r;

    LoadEnd(&p, Sim_100a5d25(a));
    LoadEnd(&q, Sim_100a5d25(b));
    if (!(q.z > p.z)) {
        double dd = (double)p.z - (double)q.z;
        float d = (float)dd;

        if (W7O_CALL_100fcb70_R(dd) >= (double)GFL(0x1012ba74)) {
            float k = (float)((double)GFL(0x1012ba78) / (double)d);
            float t = (float)((double)GFL(0x10185bf8) - (double)q.z);

            p.x = LerpMul(q.x, p.x, t, k);
            p.y = LerpMul(q.y, p.y, t, k);
            p.a = LerpMul(q.a, p.a, t, k);
            p.b = LerpMul(q.b, p.b, t, k);
            p.c = LerpMul(q.c, p.c, t, k);
            p.d = LerpMul(q.d, p.d, t, k);
            p.e = LerpMul(q.e, p.e, t, k);
        }
    } else {
        double dd = (double)q.z - (double)p.z;
        float d = (float)dd;

        if (W7O_CALL_100fcb70_R(dd) >= (double)GFL(0x1012ba74)) {
            float k = (float)((double)GFL(0x1012ba78) / (double)d);
            float t = (float)((double)GFL(0x10185bf8) - (double)p.z);

            p.x = LerpMul(p.x, q.x, t, k);
            p.y = LerpMul(p.y, q.y, t, k);
            p.a = LerpMul(p.a, q.a, t, k);
            p.b = LerpMul(p.b, q.b, t, k);
            p.c = LerpMul(p.c, q.c, t, k);
            p.d = LerpMul(p.d, q.d, t, k);
            p.e = LerpMul(p.e, q.e, t, k);
        }
    }
    r = BYTES(Sim_100a88ff());
    FL(r, 0) = p.x;
    FL(r, 4) = p.y;
    U32(r, 8) = G32(0x10185bf8);
    FL(r, 0x20) = p.a;
    FL(r, 0x24) = p.b;
    FL(r, 0x30) = p.c;
    FL(r, 0x34) = p.d;
    FL(r, 0x38) = p.e;
    return r;
}
