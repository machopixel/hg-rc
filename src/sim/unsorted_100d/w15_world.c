#include <string.h>

#include "w15.h"
#include "fns.h"
#include "../simmem.h"
#include "../util/access.h"
#include "../util/util.h"
#include "../util/w3o.h"
#include "../unsorted_1000/w2l.h"
#include "../unsorted_1000/w5x.h"
#include "../unsorted_1001/w10.h"
#include "../unsorted_1001/w12.h"
#include "../unsorted_1001/w3s.h"
#include "../unsorted_1001/w6d.h"
#include "../unsorted_1002/w5x.h"
#include "../unsorted_1002/w6d.h"
#include "../unsorted_1002/w7l.h"
#include "../unsorted_1002/w8t.h"
#include "../unsorted_1003/fns.h"
#include "../unsorted_1003/w8u.h"
#include "../unsorted_1003/w9b.h"
#include "../unsorted_1003/w9e.h"
#include "../unsorted_1004/w10.h"
#include "../unsorted_1004/w2f.h"
#include "../unsorted_1004/w4v.h"
#include "../unsorted_1004/w7m.h"
#include "../unsorted_1005/w14.h"
#include "../unsorted_1005/w2e.h"
#include "../unsorted_1005/w2g.h"
#include "../unsorted_1005/w2h.h"
#include "../unsorted_1005/w2k.h"
#include "../unsorted_1005/w4u.h"
#include "../unsorted_1005/w6e.h"
#include "../unsorted_1005/w8v.h"
#include "../unsorted_1006/w10.h"
#include "../unsorted_1007/w3q.h"
#include "../unsorted_1007/w7n.h"
#include "../unsorted_1008/w8w.h"
#include "../unsorted_1009/w8x.h"
#include "../unsorted_100a/w7o.h"
#include "../unsorted_100b/w11.h"
#include "../unsorted_100b/w14.h"
#include "../unsorted_100b/w2f.h"
#include "../unsorted_100b/w2h.h"
#include "../unsorted_100b/w3p.h"
#include "../unsorted_100b/w4v.h"
#include "../unsorted_100b/w5a.h"
#include "../unsorted_1005/w5a.h"
#include "../unsorted_100f/w7r.h"
#include "../unsorted_100b/w5z.h"
#include "../unsorted_100c/w4v.h"
#include "../unsorted_100d/w14.h"
#include "../unsorted_100d/w3r.h"
#include "../unsorted_100d/w3s.h"
#include "../unsorted_100d/w3t.h"
#include "../unsorted_100d/w5x.h"
#include "../unsorted_100d/w5y.h"
#include "../unsorted_100d/w6h.h"
#include "../unsorted_100d/w7q.h"
#include "../unsorted_100d/w8z.h"
#include "../unsorted_100d/w9e.h"
#include "../unsorted_100e/w3p.h"
#include "../unsorted_100f/w2e.h"
#include "../unsorted_100f/w2k.h"
#include "../unsorted_100f/w6h.h"
#include "../unsorted_1010/w6k.h"
#include "../unsorted_1002/w8u.h"
#include "../unsorted_1005/w3s.h"
#include "../unsorted_1005/w5x.h"
#include "../unsorted_1008/w7n.h"
#include "../unsorted_100a/w5x.h"
#include "../unsorted_100a/w5z.h"
#include "../unsorted_100a/w5y.h"
#include "../unsorted_100b/w2j.h"
#include "../unsorted_100b/fns.h"
#include "../unsorted_100b/w6g.h"
#include "../render/w7o.h"
#include "../view/w3r.h"

/* The resource loader and the error report of the DLL (left original). */
#define W15_LOAD SIM_FN(uint32_t(__cdecl *)(const void *, const char *, int32_t *), 0x100ba7d5)
#define W15_REPORT(code) SIM_FN(void(__cdecl *)(uint32_t, uint32_t), 0x100bc280)((code), 0)
/* The thunk of this function, the visitor INCL hands Sim_100dafa8 (as the original does). */
#define W15_SELF_THUNK 0x10003305u
#define W15_TAG(addr) G32(addr)
#define W15_GEAR() BYTES(G32(0x1017e6b8))
#define W15_GF64(addr) (*SIM_PTR(const double, addr))

typedef void(__cdecl *W15GearInitFn)(int, void *);

/* The locals of one call that outlive a chunk. */
typedef struct W15State {
    uint8_t *file;
    uint32_t ok;          /* [ebp-4] */
    uint32_t track;       /* [ebp-0x10], 0x1007950a of the current name */
    uint32_t track0;      /* [ebp-8] */
    const char *name;     /* [ebp-0x1c] */
    const char *name0;    /* [ebp-0x18] */
    uint32_t order;       /* [ebp-0x24], set by AFFL */
    uint8_t *gp;          /* [ebp-0xcc], the GP case's gear record (unset in the original, see w15.h) */
} W15State;

static float W15_F(uint32_t bits)
{
    float f;

    memcpy(&f, &bits, 4);
    return f;
}

static uint32_t W15_B(float f)
{
    uint32_t bits;

    memcpy(&bits, &f, 4);
    return bits;
}

/* a - b of two floats at the original's 53-bit x87 precision, stored as a float. */
static float W15_Sub(uint32_t a, uint32_t b)
{
    return (float)((double)W15_F(a) - (double)W15_F(b));
}

/* The gear type d (an entry of the table 0x10135300) as gear number [0x1015dc64]: shared by GP and GPS. */
static uint8_t *W15_NewGear(uint32_t d)
{
    uint8_t *gear;

    Sim_1002a1f2(GS32(0x1015dc64), (W15GearInitFn)(uintptr_t)U32(d, 4));
    gear = BYTES(PTRTAB(G32(0x1015dc64)));
    if (gear == 0)
        W15_REPORT(0xd);
    else if (U32(gear, 0x28) == 0)
        W15_REPORT(0xe);
    U32(gear, 0) = U32(d, 0);
    U32(gear, 0x30) = U32(d, 8);
    U32(gear, 0x34) = U32(d, 0xc);
    U32(gear, 0x38) = U32(d, 0x10);
    U32(gear, 0x44) = U32(d, 0x1c);
    U32(gear, 0x20) = G32(0x1017d200);
    U32(gear, 0x24) = 0xffffffffu;
    if (G32(0x1015dc64) == G32(0x1017b8c8)) {
        U32(gear, 0x14) = 0;
        U32(gear, 0x3c) = U32(d, 0x14);
        U32(gear, 0x40) = U32(d, 0x18);
    } else {
        U32(gear, 0x3c) = 0;
        U32(gear, 0x40) = 0;
    }
    U32(gear, 4) = G32(0x1015dc64);
    G32(0x1015dc64) += 1;
    G32(0x1017e6b8) = (uint32_t)(uintptr_t)gear;
    return gear;
}

/* The entry of the gear type table 0x10135300 for kind (11 entries; report 0x30 and entry 0 when none matches). */
static uint32_t W15_GearType(int32_t kind)
{
    int32_t i, found = -1;

    for (i = 0; i <= 10; i++) {
        if (kind == GS32(0x10135300 + (uint32_t)i * 0x20)) {
            found = i;
            break;
        }
    }
    if (found == -1) {
        W15_REPORT(0x30);
        found = 0;
    }
    return 0x10135300u + (uint32_t)found * 0x20;
}

static void W15_Plnt(const uint8_t *c)
{
    uint8_t *cfg = BYTES(G32(0x1017b8c4));
    int32_t v = (S32(cfg, 0xf) != 0 && G32(0x1015ec58) != 0) ? S32(cfg, 0xf) : S32(c, 8);

    GFL(0x1013abc0) = (float)((double)v * (double)GFL(0x1012bda8) * (double)GFL(0x1012bdac));
    G32(0x1013abcc) = U32(c, 0x10);
    G32(0x1013abc8) = U32(c, 0x14);
    if (G32(0x1013abc8) == 0)
        G32(0x1013abc8) = 1;
    cfg = BYTES(G32(0x1017b8c4));
    if (U32(cfg, 0x13) != 0 && G32(0x1015ec58) != 0)
        G32(0x1013abd8) = U32(BYTES(G32(0x1017b8c4)), 0x13);
    else
        G32(0x1013abd8) = U32(c, 0x24);
    G32(0x1015ebbc) = U32(c, 0x38) == 0;
    G32(0x1015dd70) = U32(c, 0x3c);
    G32(0x10134894) = U32(c, 0x40) == 0;
    G32(0x1014cb04) = U32(c, 0x44) == 0;
    G32(0x1014cb08) = U32(c, 0x48) == 0;
    G32(0x1014cb0c) = U32(c, 0x4c) == 0;
    if (S32(c, 0x28) > 0)
        GFL(0x101453bc) = (float)S32(c, 0x28);
    if (U32(c, 0x2c) != 0 || U32(c, 0x30) != 0) {
        G32(0x1014befc) = U32(c, 0x2c);
        G32(0x1017cc58) = U32(c, 0x30);
    }
    if (S32(c, 0x34) > 0)
        GFL(0x101453c0) = (float)((double)S32(c, 0x34) * (double)GFL(0x1012bda8));
}

static void W15_Palg(uint8_t *c)
{
    int32_t n = 0, i;
    uint8_t *p = c + 8;

    if (G32(0x1014cb38) != 0) {
        /* the original names every slot of the first three rows with the last row (+0x138) */
        for (i = 0; i < 3; i++) {
            Sim_100d37ea((const char *)c + 0x138, n);
            Sim_100d37ea((const char *)c + 0x138, n + 1);
            Sim_100d37ea((const char *)c + 0x138, n + 2);
            Sim_100d37ea((const char *)c + 0x138, n + 3);
            n += 4;
            p += 0x40;
        }
        Sim_100d37ea((const char *)p, n);
        Sim_100d37ea((const char *)p + 0x10, n + 1);
        Sim_100d37ea((const char *)p + 0x20, n + 2);
        Sim_100d37ea((const char *)p + 0x30, n + 3);
    } else {
        for (i = 0; i < 4; i++) {
            Sim_100d37ea((const char *)p, n);
            Sim_100d37ea((const char *)p + 0x10, n + 1);
            Sim_100d37ea((const char *)p + 0x20, n + 2);
            Sim_100d37ea((const char *)p + 0x30, n + 3);
            n += 4;
            p += 0x40;
        }
    }
    Sim_100d37ea((const char *)c + 0x108, 0x10);
    Sim_100d37ea((const char *)c + 0x118, 0x11);
    Sim_100d37ea((const char *)c + 0x128, 0x12);
    Sim_100d37ea((const char *)c + 0x138, 0x13);
}

static void W15_Lite(const uint8_t *c)
{
    GFL(0x1014ca1c) = (float)S32(c, 0x10);
    GFL(0x1014ca20) = (float)S32(c, 0x14);
    GFL(0x1014ca24) = (float)S32(c, 0x18);
    if (U32(c, 0x10) == 999999 && U32(c, 0x14) == 999999 && U32(c, 0x18) == 999999) {
        G32(0x10143fd4) = 0;
    } else if (U32(c, 0x10) == 0xd9038 && U32(c, 0x14) == 0xd9038 && U32(c, 0x18) == 0xd9038) {
        G32(0x10143fd4) = 0;
        G32(0x101668d8) = 1;
    }
    U16(GBYTES(0x1014ca28), 0) = U16(c, 0x1e);
    U16(G32(0x1014cae0), 0x2a) = U16(c, 0x1c);
    G32(0x1014cb2c) = U32(c, 0x20);
    if (GS32(0x1014cb2c) > 0)
        G32(0x1014cb24) = 1;
    if (S16(GBYTES(0x1014ca28), 0) != 0)
        Sim_10084cee(SIM_PTR(float, 0x1014ca1c), SIM_PTR(float, 0x1014ca20), SIM_PTR(float, 0x1014ca24));
}

static void W15_Vwsp(const uint8_t *c)
{
    uint8_t *v = BYTES(U32(c, 8));
    uint32_t n = (U32(c, 4) - 8) >> 5, i;

    if (n > 8) {
        W15_REPORT(0x27);
        return;
    }
    for (i = 0; i < n; i++) {
        int32_t k = S32(v, 0);

        if (k <= 8 && k >= 0) {
            G32(0x10185404 + (uint32_t)k * 0x14) = U32(v, 4);
            G32(0x10185408 + (uint32_t)k * 0x14) = U32(v, 8);
            G32(0x1018540c + (uint32_t)k * 0x14) = U32(v, 4) + U32(v, 0xc) - 1;
            G32(0x10185410 + (uint32_t)k * 0x14) = U32(v, 8) + U32(v, 0x10) - 1;
        } else {
            W15_REPORT(0x27);
        }
        v += 0x20;
    }
}

static void W15_Vwst(const uint8_t *c)
{
    GFL(0x1014ca00) = (float)S32(c, 8);
    GFL(0x1014ca04) = (float)S32(c, 0xc);
    GFL(0x1014ca08) = (float)S32(c, 0x10);
    GFL(0x1014ca0c) = (float)((double)S32(c, 0x14) * (double)GFL(0x1012bda8));
    GFL(0x1014ca10) = (float)((double)S32(c, 0x18) * (double)GFL(0x1012bda8));
    GFL(0x1014ca14) = (float)((double)S32(c, 0x1c) * (double)GFL(0x1012bda8));
    GFL(0x1014ca18) = (float)((double)S32(c, 0x20) * (double)GFL(0x1012bda8));
}

static void W15_Scrg(const uint8_t *c)
{
    int key = Sim_100db770(S16(c, 8));
    uint32_t obj = Sim_100b36d8((uint32_t)key), node;

    if (obj == 0) {
        int idx = Sim_100b38c5(key);

        if (idx != -1 && Sim_100b48de(idx, (int32_t)G32(0x101a2110 + (uint32_t)idx * 0x88)) != 0)
            obj = Sim_100b53bb(idx);
    }
    if (obj != 0 && (node = Sim_100096ee((void *)(uintptr_t)obj)) != 0)
        Sim_100a9300((void *)(uintptr_t)node);
    else
        W15_REPORT(0x2e);
}

static void W15_Gp(W15State *s, const uint8_t *c)
{
    int32_t key = S16(c, 8), kind = S16(c, 0xa);
    uint32_t obj, node, d;

    key = Sim_100db770(key);
    if (kind < 0 || kind > 10) {
        W15_REPORT(0x30);
        return;
    }
    obj = Sim_100b36d8((uint32_t)key);
    if (obj == 0) {
        G32(0x1017e6b8) = 0;
        /* the original stores through its unset gear pointer here; C starts it as 0 and then stores nothing */
        if (s->gp != 0)
            U32(s->gp, 0x48) = 0;
        return;
    }
    node = Sim_100096ee((void *)(uintptr_t)obj);
    if (node == 0)
        return;
    d = W15_GearType(kind);
    s->gp = W15_NewGear(d);
    U32(s->gp, 0x48) = node;
    U32(s->gp, 0x4c) = node;
    Sim_1003acf3(s->gp);
    Sim_1003a990(s->gp);
}

/* PITF / VPTF: a nested file named by the chunk, read with this function (report `missing` when it does not open). */
static void W15_Nested(W15State *s, const uint8_t *c, uint32_t missing)
{
    uint8_t key[0x40];
    uint8_t rec[0x24];
    uint8_t *f;

    if (G32(0x1017e6b8) == 0 || U32(W15_GEAR(), 0x14) != 0)
        return;
    U16(key, 0) = U16(c, 8);
    Str_Copy((char *)key + 2, (const char *)c + 0xa);
    f = Sim_100278c0(key, rec);
    if (f == 0) {
        W15_REPORT(missing);
        return;
    }
    s->ok &= Sim_100dcc12(f);
    Sim_10027a8f(f);
}

static void W15_Mgdf(const uint8_t *c)
{
    uint8_t *u;

    if (G32(0x1017e6b8) == 0)
        return;
    u = BYTES(U32(W15_GEAR(), 0x28));
    Sim_100462c0((const int16_t *)(c + 8), (float *)(u + 0x520), (float *)(u + 0x524), (float *)(u + 0x528),
                 (float *)(u + 0x52c), (float *)(u + 0x530), (float *)(u + 0x534), (float *)(u + 0x53c));
    U32(u, 0x588) = U32(u, 0x520);
}

static void W15_Gt(W15State *s, const uint8_t *c)
{
    int32_t neg = S16(c, 0x18), idx, k0, k1, a, b;
    uint32_t flags = Sim_100dcbe0(U16(c, 0x1a));
    uint8_t *r;

    idx = GameThing_NextIndex();
    if (idx == -1)
        return;
    r = GBYTES(0x109e9ba0 + (uint32_t)idx * 0x50);
    U16(r, 0) = U16(c, 0x1c);
    U32(r, 2) = 0;
    if (neg < 0) {
        neg = -neg;
        if ((neg & 1) != 0)
            U32(r, 2) |= 1;
        else
            U32(r, 2) |= 2;
    }
    U32(r, 0xc) = neg == 32000 ? 0 : (uint32_t)neg;
    U32(r, 0x10) = s->order;
    Str_CopyN((char *)r + 0x18, (const char *)c + 0x20, 0x16);
    U8(r, 0x2d) = 0;
    Str_CopyN((char *)r + 0x2e, (const char *)c + 0x36, 0x16);
    U8(r, 0x43) = 0;
    k0 = S16(c, 8);
    k1 = S16(c, 0xa);
    a = -1;
    if (k0 != -1) {
        k0 = Sim_100db770(k0);
        a = Sim_100b38c5(k0);
        if (a == -1)
            a = Sim_100b391c((uint32_t)k0);
        b = -1;
        if (k1 != -1) {
            k1 = Sim_100db770(k1);
            b = Sim_100b38c5(k1);
            if (b == -1)
                b = Sim_100b391c((uint32_t)k1);
        }
        if (a != -1) {
            int32_t size = 0;
            uint32_t res;

            Sim_100b4342(a, b, idx);
            res = W15_LOAD(GBYTES(0x101a2100 + (uint32_t)a * 0x88), SIM_PTR(const char, 0x10152540), &size);
            if (res != 0) {
                uint32_t p = res + 0x1d4;

                U32(r, 0x44) = U32(p, 8) != 0 ? U32(p, 8) : 0;
                if (U32(p, 0xc) != 0 && U32(r, 0xc) == 0)
                    U32(r, 0x48) = U32(p, 0xc);
                else
                    U32(r, 0x48) = U32(r, 0xc);
                if (U32(r, 0xc) == 0 && neg != 32000)
                    U32(r, 0xc) = U32(r, 0x44) + U32(r, 0x48);
                Sim_100bb2b0((const uint32_t *)GBYTES(0x101a2100 + (uint32_t)a * 0x88), SIM_PTR(const char, 0x10152544));
            } else {
                U32(r, 0x44) = 0;
                U32(r, 0x48) = (uint32_t)neg;
            }
        }
        if (neg == 32000) {
            U32(r, 0x44) = 0;
            U32(r, 0x48) = 0;
            U32(r, 0xc) = 0;
        }
    }
    U32(r, 8) = (uint32_t)a;
    if (s->track != 0)
        Sim_10079664(s->name, flags, ((uint32_t)idx & 0xff) | 0x400);
}

/* A shot slot of the table 0x1096fc80 (0x156 bytes each, [0x1017e818] the next one) for the object key. */
static void W15_ShotSlot(int32_t key, uint32_t value)
{
    uint32_t obj, node;

    if (GS32(0x1017e818) >= 0x200 || GS32(0x1017e818) <= -1)
        return;
    key = Sim_100db770(key);
    if (U32(GBYTES(0x1096fcbc), G32(0x1017e818) * 0x156) == 0 && (obj = Sim_100b36d8((uint32_t)key)) != 0 &&
        (node = Sim_100096ee((void *)(uintptr_t)obj)) != 0) {
        U32(GBYTES(0x1096fc80), G32(0x1017e818) * 0x156) = value;
        U32(GBYTES(0x1096fcbc), G32(0x1017e818) * 0x156) = node;
        U32(GBYTES(0x1096fccc), G32(0x1017e818) * 0x156) = 0;
        Sim_100aa351((void *)(uintptr_t)node);
        Sim_100aa406((void *)(uintptr_t)node);
        Sim_100aa4f0((void *)(uintptr_t)node, 0x400);
    }
    G32(0x1017e818) += 1;
}

/* An explosion slot of the table 0x109a5a40 (0x2c bytes each, [0x1017e81c] the next one) for the object key. */
static void W15_BlastSlot(int32_t key, int32_t count, int32_t kind, int32_t fallback)
{
    uint8_t *e;
    uint32_t obj, node;

    if (GS32(0x1017e81c) < 0 || GS32(0x1017e81c) >= 0x100)
        return;
    if (kind < 0 || kind >= 0x4d)
        kind = fallback;
    e = GBYTES(0x109a5a40 + G32(0x1017e81c) * 0x2c);
    if (U32(e, 0) == 0) {
        if (key != -1) {
            key = Sim_100db770(key);
            obj = Sim_100b36d8((uint32_t)key);
            if (obj != 0 && (node = Sim_100096ee((void *)(uintptr_t)obj)) != 0) {
                U32(e, 0) = node;
                Sim_100aa351((void *)(uintptr_t)node);
                Sim_100aa406((void *)(uintptr_t)node);
                Sim_100aa4f0((void *)(uintptr_t)node, 0x20);
                U32(e, 0x1c) = (uint32_t)count;
                if (count > 0)
                    Sim_1000711c((short)U16(e, 0x1c), 2);
            }
        }
        U32(e, 0x24) = (uint32_t)kind;
        U32(e, 0x18) = 0;
    }
    G32(0x1017e81c) += 1;
}

/* An OBJ chunk image for Sim_100db78b, as HGBT / HGXP build it on the stack. The original never writes +0..+7, the
 * dword +0x34 (the colour Sim_100db78b reads) or the bytes after the name; C has them 0 (see w15.h). */
static void W15_PoolRecord(uint8_t *rec, const uint8_t *c, uint32_t base)
{
    int i;

    memset(rec, 0, 0x80);
    U16(rec, 0xa) = 0xffff;
    U16(rec, 0xc) = U16(c, base);
    for (i = 0; i < 3; i++) {
        U32(rec, 0xe + 4 * i) = (uint32_t)S32(c, base + 2 + 4 * i);
        U32(rec, 0x1a + 4 * i) = (uint32_t)(int32_t)((double)S32(c, base + 0xe + 4 * i) / W15_GF64(0x1012bdb0));
        U32(rec, 0x26 + 4 * i) = (uint32_t)S32(c, base + 0x1a + 4 * i);
    }
    U16(rec, 0x32) = U16(c, base + 0x26);
}

static void W15_Hgbt(uint8_t *c)
{
    uint8_t rec[0x80];
    int32_t n, i, key;

    W15_PoolRecord(rec, c, 0x20e);
    U16(rec, 8) = (uint16_t)(uintptr_t)(c + 0xe);
    n = S16(c, 0x23a);
    U16(rec, 0x38) = (uint16_t)n;
    Str_Copy((char *)rec + 0x3a, (const char *)c + 0x23c);
    for (i = 0; i < n; i++) {
        U16(rec, 8) = U16(c, 0xe + i * 2);
        key = S16(rec, 8);
        Sim_100db78b(rec, 0, 0, GS32(0x1017d224), GS32(0x1017d1fc), G32(0x1017d200));
        W15_ShotSlot(key, (uint32_t)(int32_t)S16(c, 0xa));
    }
}

static void W15_Hgxp(uint8_t *c)
{
    uint8_t rec[0x80];
    int32_t n, i, key;

    W15_PoolRecord(rec, c, 0x210);
    n = S16(c, 0x23c);
    U16(rec, 0x38) = (uint16_t)n;
    Str_Copy((char *)rec + 0x3a, (const char *)c + 0x23e);
    for (i = 0; i < n; i++) {
        U16(rec, 8) = U16(c, 0x10 + i * 2);
        key = S16(rec, 8);
        Sim_100db78b(rec, 0, 0, GS32(0x1017d224), GS32(0x1017d1fc), G32(0x1017d200));
        W15_BlastSlot(key, S16(c, 0xa), S16(c, 0xc), 0x3e);
    }
}

static void W15_Navp(W15State *s, const uint8_t *c)
{
    uint8_t *e;
    float v[3];
    uint32_t flags, word;

    if (GS32(0x1017e5a8) >= 0x80 || GS32(0x1017e5a8) == -1)
        return;
    e = GBYTES(0x109a8640 + G32(0x1017e5a8) * 0x54);
    v[0] = (float)S32(c, 8);
    v[1] = (float)S32(c, 0xc);
    v[2] = (float)S32(c, 0x10);
    Sim_100b403d(v);
    U32(e, 0x18) = W15_B(v[0]);
    U32(e, 0x1c) = W15_B(v[1]);
    U32(e, 0x20) = W15_B(v[2]);
    U32(e, 0) = (uint32_t)(int32_t)S16(c, 0x18);
    U16(e, 0x24) = U16(c, 0x1a);
    U32(e, 0xc) = (uint32_t)(int32_t)S16(c, 0x1e);
    U32(e, 8) = (uint32_t)(int32_t)S16(c, 0x1c);
    FL(e, 0x14) = (float)((double)S32(c, 0x14) * (double)GFL(0x1012bda8));
    FL(e, 0x10) = (float)((double)U16(c, 0x20) * (double)GFL(0x1012bdb8));
    U32(e, 4) = 0;
    Str_CopyN((char *)e + 0x28, (const char *)c + 0x24, 0x15);
    U8(e, 0x3d) = 0;
    flags = Sim_100dcbe0(U16(c, 0x22));
    word = (uint32_t)U8(GBYTES(0x1017e5a8), 0) | 0x100;
    G32(0x1017e5a8) += 1;
    if (s->track != 0)
        Sim_10079664(s->name, flags, word);
}

static void W15_Navo(W15State *s, const uint8_t *c)
{
    uint32_t a, b, cc, obj, node, flags, word;
    int32_t key, idx, found = 0;

    if (GS32(0x1017e5a8) >= 0x80 || GS32(0x1017e5a8) == -1)
        return;
    key = Sim_100db770(S16(c, 0xa));
    idx = Sim_100b38c5(key);
    if (idx != -1) {
        Sim_100b53f0(idx, &a, &b, &cc);
        U32(GBYTES(0x109a8658), G32(0x1017e5a8) * 0x54) = a;
        U32(GBYTES(0x109a865c), G32(0x1017e5a8) * 0x54) = b;
        U32(GBYTES(0x109a8660), G32(0x1017e5a8) * 0x54) = cc;
        found = 1;
    } else if ((obj = Sim_100b36d8((uint32_t)key)) != 0 && (node = Sim_100096ee((void *)(uintptr_t)obj)) != 0) {
        Sim_100a9ca3((void *)(uintptr_t)node, (uint32_t)(uintptr_t)&a, (uint32_t)(uintptr_t)&b,
                     (uint32_t)(uintptr_t)&cc);
        U32(GBYTES(0x109a8644), G32(0x1017e5a8) * 0x54) = node;
        U32(GBYTES(0x109a8654), G32(0x1017e5a8) * 0x54) = b;
        found = 1;
    }
    if (!found)
        return;
    FL(GBYTES(0x109a8650), G32(0x1017e5a8) * 0x54) = (float)S16(c, 0xc);
    U32(GBYTES(0x109a8640), G32(0x1017e5a8) * 0x54) = 1;
    U16(GBYTES(0x109a8664), G32(0x1017e5a8) * 0x54) = 0;
    U32(GBYTES(0x109a8648), G32(0x1017e5a8) * 0x54) = 0;
    flags = Sim_100dcbe0(U16(c, 0xe));
    word = (uint32_t)U8(GBYTES(0x1017e5a8), 0) | 0x100;
    G32(0x1017e5a8) += 1;
    if (s->track != 0)
        Sim_10079664(s->name, flags, word);
}

static void W15_Tsk(uint8_t *c)
{
    int32_t kind = S16(c, 8), key = -1, idx;
    uint32_t interval = U32(c, 0xa);
    char *text = (char *)c + 0xe;
    char *semi = Crt_StrChr(text, 0x3b);
    TimerCallback cb;

    if (semi != 0) {
        *semi = 0;
        key = Crt_AToI(text);
        key = Sim_100db770(key);
        *semi = 0x3b;
    }
    if (kind < 0 || kind >= 6) {
        W15_REPORT(0x33);
        return;
    }
    idx = Sim_100b38c5(key);
    cb = (TimerCallback)(uintptr_t)G32(0x10152468 + (uint32_t)kind * 4);
    if (idx != -1)
        Sim_100b41dc(idx, cb, interval, (uint32_t)(uintptr_t)text);
    else
        Sim_100cc6ea(SIM_PTR(uint32_t, 0x1017b8c0), cb, interval, (uint32_t)(uintptr_t)text);
}

/* POS (rotate 0) and ROT (rotate 1): the position or angles of an object's node. */
static void W15_Place(const uint8_t *c, int rotate)
{
    int32_t key = S16(c, 8);
    uint32_t x = W15_B((float)S32(c, 0xa)), y = W15_B((float)S32(c, 0xe)), z = W15_B((float)S32(c, 0x12));
    uint32_t obj, node;

    key = Sim_100db770(key);
    obj = Sim_100b36d8((uint32_t)key);
    if (obj == 0 || (node = Sim_100096ee((void *)(uintptr_t)obj)) == 0)
        return;
    if (rotate)
        Sim_100aa0da((void *)(uintptr_t)node, x, y, z, 0);
    else
        Sim_100a9d47((void *)(uintptr_t)node, x, y, z);
    Sim_100aa799((void *)(uintptr_t)node);
}

/* The positions the GPS cases read: X1 / Y1 / Z1 of a part, X2 / Y2 / Z2 of the gear's model. */
typedef struct W15Pos {
    uint32_t x1, y1, z1, x2, y2, z2;
} W15Pos;

/* The gear's view node (gear +0x4c) from the part `part` of the model, or `fallback` when the part is missing. */
static void W15_ViewNode(uint8_t *gp, uint32_t model, uint32_t part, uint32_t fallback)
{
    uint8_t copy[0xa4];

    U32(gp, 0x4c) = (uint32_t)(uintptr_t)Sim_1005ae58((void *)(uintptr_t)model, SIM_PTR(char, part));
    if (U32(gp, 0x4c) == 0) {
        U32(gp, 0x4c) = fallback;
    } else {
        Mem_Move(copy, (const void *)(uintptr_t)U32(gp, 0x4c), 0xa4);
        Sim_100aa4f0((void *)(uintptr_t)U32(gp, 0x4c), 0x70);
    }
}

/* The gun part (both gun slots +0x4c0 / +0x4c4) and the eye height +0x520 from it; returns the part. */
static uint32_t W15_Gun(uint8_t *u, uint32_t model, uint32_t name)
{
    uint32_t gun = (uint32_t)(uintptr_t)Sim_1005ae58((void *)(uintptr_t)model, SIM_PTR(char, name));

    if (gun == 0) {
        W15_REPORT(0x56);
    } else {
        U32(u, 0x4c0) = gun;
        U32(u, 0x4c4) = gun;
        Sim_100aa4f0((void *)(uintptr_t)gun, 0x70);
    }
    return gun;
}

static void W15_Offsets(uint32_t part, uint32_t model, W15Pos *p, uint8_t *u)
{
    Sim_100a9cfb((void *)(uintptr_t)part, &p->x1, &p->y1, &p->z1);
    Sim_100a9cfb((void *)(uintptr_t)model, &p->x2, &p->y2, &p->z2);
    FL(u, 0x520) = W15_Sub(p->y2, p->y1);
    U32(u, 0x588) = U32(u, 0x520);
}

static void W15_Part(uint32_t part, W15Pos *p)
{
    Sim_100a9d21((void *)(uintptr_t)part, &p->x1, &p->y1, &p->z1);
}

/* GPS types 3, 4 and 10. */
static void W15_GpsTank(uint8_t *gp, uint8_t *u, uint32_t model, int32_t type)
{
    W15Pos p;
    uint32_t gun = W15_Gun(u, model, 0x10152548);

    W15_Offsets(gun, model, &p, u);
    FL(u, 0x53c) = Sim_10089325(BYTES(U32(model, 0x8c)));
    U32(u, 0x490) = (uint32_t)(uintptr_t)Sim_1005ae58((void *)(uintptr_t)model, SIM_PTR(char, 0x10152550));
    if (U32(u, 0x490) == 0) {
        U32(u, 0x490) = 0;
    } else {
        W15_Part(U32(u, 0x490), &p);
        FL(u, 0x52c) = W15_Sub(p.x1, p.x2);
        FL(u, 0x528) = W15_Sub(p.y1, p.y2);
        FL(u, 0x530) = W15_Sub(p.x1, p.x2);
    }
    U32(u, 0x534) = 0x43b40000;
    U32(u, 0x48c) = (uint32_t)(uintptr_t)Sim_1005ae58((void *)(uintptr_t)model, SIM_PTR(char, 0x10152558));
    if (U32(u, 0x48c) == 0 && type == 3)
        U32(u, 0x48c) = U32(model, 4);
    U32(gp, 0x4c) = (uint32_t)(uintptr_t)Sim_1005ae58((void *)(uintptr_t)model, SIM_PTR(char, 0x10152560));
    if (U32(gp, 0x4c) == 0) {
        if (U32(u, 0x490) != 0)
            U32(gp, 0x4c) = U32(u, 0x490);
        else if (U32(u, 0x48c) != 0)
            U32(gp, 0x4c) = U32(u, 0x48c);
        else
            U32(gp, 0x4c) = model;
    } else {
        uint8_t copy[0xa4];

        Mem_Move(copy, (const void *)(uintptr_t)U32(gp, 0x4c), 0xa4);
        Sim_100aa4f0((void *)(uintptr_t)U32(gp, 0x4c), 0x70);
    }
    W15_Part(U32(gp, 0x4c), &p);
    FL(u, 0x524) = W15_Sub(p.y1, p.y2);
}

/* GPS types 1 and 9 (the walking gears). */
static void W15_GpsGear(uint8_t *gp, uint8_t *u, uint32_t model)
{
    W15Pos p;
    uint32_t left, right;
    const uint8_t *code = BYTES(U32(model, 0x90));

    switch ((int8_t)code[0]) {
    case 0x44:
    case 0x64:
        G32(0x1017e7f4) = 1;
        break;
    case 0x47:
    case 0x67:
        G32(0x1017e800) = 1;
        break;
    case 0x53:
    case 0x73:
        switch ((int8_t)code[1]) {
        case 0x41:
        case 0x61:
            G32(0x1017e7f8) = 1;
            break;
        case 0x4e:
        case 0x6e:
            G32(0x1017e7fc) = 1;
            break;
        }
        break;
    }
    U32(u, 0x494) = (uint32_t)(uintptr_t)Sim_1005ae58((void *)(uintptr_t)model, SIM_PTR(char, 0x10152568));
    if (U32(u, 0x494) == 0)
        U32(u, 0x494) = (uint32_t)(uintptr_t)Sim_1005ae58((void *)(uintptr_t)model, SIM_PTR(char, 0x10152574));
    if (U32(u, 0x494) == 0)
        U32(u, 0x494) = (uint32_t)(uintptr_t)Sim_1005ae58((void *)(uintptr_t)model, SIM_PTR(char, 0x1015257c));
    left = (uint32_t)(uintptr_t)Sim_1005ae58((void *)(uintptr_t)model, SIM_PTR(char, 0x10152584));
    right = (uint32_t)(uintptr_t)Sim_1005ae58((void *)(uintptr_t)model, SIM_PTR(char, 0x10152590));
    if (left == 0 || right == 0) {
        W15_REPORT(0x56);
    } else {
        U32(u, 0x4c0) = left;
        U32(u, 0x4c4) = right;
        Sim_100aa4f0((void *)(uintptr_t)left, 0x70);
        Sim_100aa4f0((void *)(uintptr_t)right, 0x70);
    }
    W15_Offsets(left, model, &p, u);
    FL(u, 0x53c) = -W15_F(p.y1);
    W15_ViewNode(gp, model, 0x1015259c, model);
    W15_Part(U32(gp, 0x4c), &p);
    FL(u, 0x524) = W15_Sub(p.y1, p.y2);
    U32(u, 0x490) = (uint32_t)(uintptr_t)Sim_1005ae58((void *)(uintptr_t)model, SIM_PTR(char, 0x101525a8));
    if (U32(u, 0x490) == 0)
        W15_REPORT(0x56);
    W15_Part(U32(u, 0x490), &p);
    FL(u, 0x52c) = W15_Sub(p.x1, p.x2);
    U32(u, 0x490) = (uint32_t)(uintptr_t)Sim_1005ae58((void *)(uintptr_t)model, SIM_PTR(char, 0x101525b0));
    if (U32(u, 0x490) == 0) {
        U32(u, 0x490) = (uint32_t)(uintptr_t)Sim_1005ae58((void *)(uintptr_t)model, SIM_PTR(char, 0x101525bc));
        if (U32(u, 0x490) == 0)
            W15_REPORT(0x56);
    }
    W15_Part(U32(u, 0x490), &p);
    FL(u, 0x528) = W15_Sub(p.y1, p.y2);
    FL(u, 0x530) = W15_Sub(p.x1, p.x2);
    U32(u, 0x534) = 0x43340000;
    U32(u, 0x48c) = (uint32_t)(uintptr_t)Sim_1005ae58((void *)(uintptr_t)model, SIM_PTR(char, 0x101525c4));
    if (U32(u, 0x48c) == 0) {
        if (Sim_1010d240(BYTES(U32(model, 0x90)), SIM_PTR(const uint8_t, 0x101525d0), 2) == 0) {
            U32(u, 0x48c) = 0;
        } else {
            U32(u, 0x48c) = (uint32_t)(uintptr_t)Sim_1005ae58((void *)(uintptr_t)model, SIM_PTR(char, 0x101525d4));
            if (U32(u, 0x48c) == 0)
                W15_REPORT(0x56);
        }
    }
}

/* GPS types 5 and 8. */
static void W15_GpsCraft(uint8_t *gp, uint8_t *u, uint32_t model)
{
    W15Pos p;
    uint32_t gun;

    if (Sim_1010d240(BYTES(U32(model, 0x90)), SIM_PTR(const uint8_t, 0x101525dc), 3) == 0)
        G32(0x1017e7f0) = 1;
    else if (Sim_1010d240(BYTES(U32(model, 0x90)), SIM_PTR(const uint8_t, 0x101525e0), 3) == 0)
        G32(0x1017e7ec) = 1;
    gun = W15_Gun(u, model, 0x101525e4);
    W15_Offsets(gun, model, &p, u);
    U32(u, 0x520) = 0x457a0000;
    FL(u, 0x53c) = Sim_10089325(BYTES(U32(model, 0x8c)));
    U32(u, 0x58c) = 0;
    W15_ViewNode(gp, model, 0x101525ec, model);
    W15_Part(U32(gp, 0x4c), &p);
    FL(u, 0x524) = W15_Sub(p.y1, p.y2);
    U32(u, 0x48c) = (uint32_t)(uintptr_t)Sim_1005ae58((void *)(uintptr_t)model, SIM_PTR(char, 0x101525f4));
    U32(u, 0x490) = (uint32_t)(uintptr_t)Sim_1005ae58((void *)(uintptr_t)model, SIM_PTR(char, 0x101525fc));
    if (U32(u, 0x490) != 0) {
        W15_Part(U32(u, 0x490), &p);
        FL(u, 0x52c) = W15_Sub(p.x1, p.x2);
        FL(u, 0x528) = W15_Sub(p.y1, p.y2);
        FL(u, 0x530) = W15_Sub(p.x1, p.x2);
    }
    U32(u, 0x534) = U32(u, 0x48c) != 0 ? 0x43b40000 : 0;
}

/* GPS, every other type. */
static void W15_GpsOther(uint8_t *gp, uint8_t *u, uint32_t model)
{
    W15Pos p;
    uint32_t gun = W15_Gun(u, model, 0x10152604);

    W15_Offsets(gun, model, &p, u);
    FL(u, 0x53c) = Sim_10089325(BYTES(U32(model, 0x8c)));
    W15_ViewNode(gp, model, 0x1015260c, model);
    W15_Part(U32(gp, 0x4c), &p);
    FL(u, 0x524) = W15_Sub(p.y1, p.y2);
    U32(u, 0x490) = 0;
    U32(u, 0x534) = 0;
    U32(u, 0x48c) = 0;
}

/* GPS: a gear from its GDF design. Returns 0 when the gear table is full (the reading stops, result 0). */
static int W15_Gps(W15State *s, uint8_t *c)
{
    char design_name[0x100];
    char n1[9], n2[9];
    uint32_t state = U8(c, 0xe), group = U8(c, 0xc), lead = U8(c, 0xd), flags, design, d, model;
    int32_t type, gear_index = 0;
    uint8_t *gp, *u;

    if (G32(0x1015ec58) != 0) {
        /* 1.2 (0x100c1152), network games only: a gear whose type name holds "tank" comes as infantry */
        Str_CopyN(design_name, (const char *)c + 0x36, 0x16);
        design_name[0x15] = 0;
        Crt12_StrLwr(design_name);
#ifdef W15T_MUTATE /* the checker's negative check only: no swap */
        if (0) {
#else
        if (W5A_STRSTR(design_name, "tank") != 0) {
#endif
            U16(c, 8) = 0;
            U16(c, 0xa) = 0;
            Str_CopyN((char *)c + 0x24, "d01infnt", 8);
            Str_CopyN((char *)c + 0x2d, "mmb00std", 8);
            Str_CopyN((char *)c + 0x36, "Infantry", 0x16);
            Str_CopyN((char *)c + 0x4c, "GREL", 0x16);
        }
    }
    Mem_Fill(design_name, 0, 0x100);
    Str_CopyN(n1, (const char *)c + 0x24, 8);
    n1[8] = 0;
    Str_CopyN(n2, (const char *)c + 0x2d, 8);
    n2[8] = 0;
    if (group >= 0x10)
        group = 0;
    if (state == 0) {
        G32(0x1013bb40) = group;
        G32(0x1017e7e8) = 1;
        if (G32(0x1015ec58) == 0)
            G32(0x1017b8c8) = G32(0x1015dc64);
    }
    flags = Sim_100dcbe0(U16(c, 0x20));
    Str_Copy(design_name, n1);
    design = Sim_100599a0(design_name);
    G32(0x1017d200) = 0;
    if (design == 0)
        W15_REPORT(0x55);
    G32(0x1017d200) = (uint32_t)(Sim_1005ada5((void *)(uintptr_t)design) - 1);
    if (GS32(0x1015dc64) >= 0x19) {
        s->ok = 0;
        return 0;
    }
    type = S32(design, 0xb4);
    if (type == 2)
        type = 4;
    d = W15_GearType(type);
    gp = W15_NewGear(d);
    Sim_1003acf3(gp);
    Sim_1003a990(gp);
    model = Sim_1003b417((int)G32(0x1017e820));
    G32(0x1017e820) = G32(0x101c42a0);
    Sim_100aa799((void *)(uintptr_t)model);
    U32(gp, 0x48) = model;
    U32(gp, 0x4c) = model;
    if (G32(0x1017e6b8) == 0) {
        W15_REPORT(0x43);
        return 1;
    }
    gear_index = S32(W15_GEAR(), 4);
    {
        int i;

        for (i = 0; i < 8; i++)
            U16(W15_GEAR(), 0x188 + i * 2) = U16(c, 0x10 + i * 2);
    }
    if (U8(G32(0x1017b8c4), 5) == 0)
        U16(W15_GEAR(), 0x194) = 0;
    U32(W15_GEAR(), 0x14) = state;
    U16(W15_GEAR(), 0x1c) = U16(c, 0x22);
    if (G32(0x1015ec58) != 0 && U32(W15_GEAR(), 4) != G32(0x1017b8c8))
        U16(W15_GEAR(), 0x1c) |= 0x4000;
    Str_CopyN((char *)W15_GEAR() + 0x124, (const char *)c + 0x36, 0x16);
    U8(W15_GEAR(), 0x139) = 0;
    Str_CopyN((char *)W15_GEAR() + 0x13a, (const char *)c + 0x4c, 0x16);
    U8(W15_GEAR(), 0x14f) = 0;
    if (G32(0x1015ec58) != 0 && U32(W15_GEAR(), 0x14) == 2) {
        U32(W15_GEAR(), 0x14) = 3;
        U16(W15_GEAR(), 0x1c) &= 0xb7ff;
        U32(W15_GEAR(), 0xc) = 0xffffffffu;
        U32(W15_GEAR(), 0x18) = 0;
    }
    if (s->track != 0)
        Sim_10079664(s->name, flags, ((uint32_t)gear_index & 0xff) | 0x200);
    u = BYTES(U32(W15_GEAR(), 0x28));
    switch (type) {
    case 3:
    case 4:
    case 10:
        W15_GpsTank(gp, u, model, type);
        break;
    case 1:
    case 9:
        W15_GpsGear(gp, u, model);
        break;
    case 5:
    case 8:
        W15_GpsCraft(gp, u, model);
        break;
    default:
        W15_GpsOther(gp, u, model);
        break;
    }
    if (u != 0 && Sim_1005cd4a(u, BYTES(design)) == 0) {
        Sim_1005a138((void *)(uintptr_t)design);
        design = 0;
        W15_REPORT(0x43);
        s->ok = 0;
    }
    Str_Copy((char *)u + 0xbeb, design_name);
    if (design != 0)
        Sim_1005a138((void *)(uintptr_t)design);
    U32(W15_GEAR(), 8) = group;
    if ((U32(W15_GEAR(), 0) == 2 || U32(W15_GEAR(), 0) == 8) && G32(0x1013bb40) == group &&
        G32(0x109b7d48 + group * 0x6c) == 0) {
        Sim_100526a9((unsigned)gear_index, 4);
        G32(0x109b7d6c + group * 0x6c) = (uint32_t)gear_index;
        G32(0x109b7d48 + group * 0x6c) = 1;
    } else {
        Sim_100526a9((unsigned)gear_index, G32(0x109b7d44 + group * 0x6c));
        U32(W15_GEAR(), 0x280) = U32(W15_GEAR(), 0x10);
        G32(0x109b7d5c + group * 0x6c + G32(0x109b7d44 + group * 0x6c) * 4) = (uint32_t)gear_index;
        G32(0x109b7d44 + group * 0x6c) += 1;
    }
    if (lead == 1)
        Sim_100526e3((int)group, gear_index);
    return 1;
}

static void W15_Mlit(const uint8_t *c)
{
    uint32_t src[12];
    int32_t idx = Sim_10058575();

    memset(src, 0, sizeof(src));
    src[0] = 1;
    src[1] = W15_B((float)S32(c, 8));
    src[2] = W15_B((float)S32(c, 0xc));
    src[3] = W15_B((float)S32(c, 0x10));
    src[11] = W15_B((float)S32(c, 0x14));
    Sim_100585e1(idx, 7, src);
}

static void W15_Camo(const uint8_t *c)
{
    int32_t a = S32(c, 8), b = S32(c, 0xc);

    if (a < 1)
        a = 0;
    else if (a >= 8)
        a = 7;
    if (b < 1)
        b = 0;
    else if (b >= 8)
        b = 7;
    G32(0x10152658 + (uint32_t)a * 4) = (uint32_t)b;
}

/* One chunk. Returns 0 when the reading stops. */
static int W15_Chunk(W15State *s, uint8_t *c)
{
    uint32_t tag = U32(c, 0);

    if (tag == W15_TAG(0x101349f4)) {
        W15_Plnt(c);
    } else if (tag == W15_TAG(0x10134a00)) {
        G32(0x1015ebb4) = (uint32_t)(int32_t)S16(c, 0xc);
    } else if (tag == W15_TAG(0x10134af8)) {
        Str_Copy(SIM_PTR(char, 0x1015da30), (const char *)c + 8);
    } else if (tag == W15_TAG(0x10134ad4)) {
        GFL(0x1014ca3c) = (float)S32(c, 8);
        /* 1.2 (0x100c1152): the second value two and a half times */
#ifdef W15V_MUTATE /* the checker's negative check only: 1.0's */
        GFL(0x1014ca40) = (float)S32(c, 0xc);
#else
        GFL(0x1014ca40) = (float)((double)S32(c, 0xc) * 2.5);
#endif
    } else if (tag == W15_TAG(0x10134aec)) {
        Str_Copy(SIM_PTR(char, 0x10a3ae80), (const char *)c + 0xa);
    } else if (tag == W15_TAG(0x10134ae4)) {
        Str_CopyN(SIM_PTR(char, 0x1015dcb0), (const char *)c + 0xa, 8);
        U8(GBYTES(0x1015dcb8), 0) = 0;
        Str_Copy(SIM_PTR(char, 0x1015daf0), (const char *)c + 0xa);
    } else if (tag == W15_TAG(0x10134ae8)) {
        if (G32(0x1017e6b8) != 0 && U32(W15_GEAR(), 0x14) == 0)
            Str_Copy((char *)W15_GEAR() + 0x98, (const char *)c + 0xa);
    } else if (tag == W15_TAG(0x10134adc)) {
        s->order = U32(c, 8);
    } else if (tag == W15_TAG(0x101349fc)) {
        /* TERR: nothing */
    } else if (tag == W15_TAG(0x10134a10)) {
        if (G32(0x1015ec58) != 0 && G32(0x1017b8c4) != 0)
            G32(0x1013abb8) = U32(BYTES(G32(0x1017b8c4)), 0xb);
        else
            G32(0x1013abb8) = U32(c, 0x10);
        G32(0x1015ebb8) = (uint32_t)(int32_t)S16(c, 0x14);
    } else if (tag == W15_TAG(0x101349f8)) {
        W15_Palg(c);
    } else if (tag == W15_TAG(0x10134a04)) {
        W15_Lite(c);
    } else if (tag == W15_TAG(0x10134a38)) {
        Sim_100da020(c, SIM_PTR(uint32_t, 0x10138c78), 0, 0);
    } else if (tag == W15_TAG(0x10134a34)) {
        Sim_100da020(c, SIM_PTR(uint32_t, 0x10138c7c), SIM_PTR(uint32_t, 0x1014cb50), 0);
    } else if (tag == W15_TAG(0x10134a3c)) {
        Sim_100da020(c, SIM_PTR(uint32_t, 0x10138c74), 0, 0);
    } else if (tag == W15_TAG(0x10134a08)) {
        W15_Vwsp(c);
    } else if (tag == W15_TAG(0x10134a0c)) {
        W15_Vwst(c);
    } else if (tag == W15_TAG(0x10134b5c)) {
        Str_Copy(SIM_PTR(char, 0x1017e808), (const char *)c + 8);
        s->track = Sim_1007950a(SIM_PTR(const char, 0x1017e808));
        s->name = SIM_PTR(const char, 0x1017e808);
    } else if (tag == W15_TAG(0x10134b60)) {
        U8(GBYTES(0x1017e808), 0) = 0;
        s->track = s->track0;
        s->name = s->name0;
    } else if (tag == W15_TAG(0x10134a14)) {
        Sim_100da098(c);
    } else if (tag == W15_TAG(0x10134ad0)) {
        Sim_100db0ee(c);
    } else if (tag == W15_TAG(0x10134af0)) {
        Sim_100dacf9(c);
    } else if (tag == W15_TAG(0x10134af4)) {
        Sim_100daea4(c);
    } else if (tag == W15_TAG(0x10134a18)) {
        Sim_100dac0a(c);
        s->track = Sim_1007950a(SIM_PTR(const char, 0x10152538));
        s->name = SIM_PTR(const char, 0x1015253c);
    } else if (tag == W15_TAG(0x10134a30)) {
        int16_t a = S16(c, 8), b = S16(c, 0xa), d = S16(c, 0xc);

        if (a == -1)
            a = 1;
        if (b == -1)
            b = 0;
        if (d != -1)
            Sim_10048030((uint32_t)(int32_t)d, a, b);
    } else if (tag == W15_TAG(0x10134a40)) {
        Sim_100e2756(S32(c, 8), S32(c, 0xc), S32(c, 0x10));
    } else if (tag == W15_TAG(0x10134a44)) {
        Sim_100b3fd5(S32(c, 8), S32(c, 0xc), S32(c, 0x10), S32(c, 0x14), S32(c, 0x18), S32(c, 0x1c), S32(c, 0x20),
                     S32(c, 0x24), S32(c, 0x28));
    } else if (tag == W15_TAG(0x10134a48)) {
        if (G32(0x1017d1fc) == 0) {
            int32_t i;

            G32(0x1017d1fc) = 1;
            G32(0x1017d200) = 0;
            G32(0x101c42a0) = 0;
            for (i = 0; i < GS32(0x101a1160); i++)
                U32(G32(0x1017d1f4), (uint32_t)i * 4) = 0xffffffffu;
        } else {
            G32(0x1017d200) += 1;
        }
    } else if (tag == W15_TAG(0x10134a4c)) {
        G32(0x1017d1fc) = 0;
    } else if (tag == W15_TAG(0x10134a50)) {
        Sim_100b3b16((const int32_t *)c);
    } else if (tag == W15_TAG(0x10134a54)) {
        Sim_100b3f8f();
    } else if (tag == W15_TAG(0x10134a58)) {
        Sim_100b3f94((uint32_t)(uintptr_t)s->file);
    } else if (tag == W15_TAG(0x10134a5c)) {
        Sim_100db78b(c, 0, 0, GS32(0x1017d224), GS32(0x1017d1fc), G32(0x1017d200));
    } else if (tag == W15_TAG(0x10134a60)) {
        if (Sim_10019cc0((int16_t *)(c + 8)) == 0)
            W15_REPORT(0x4f);
    } else if (tag == W15_TAG(0x10134a64)) {
        W15_Scrg(c);
    } else if (tag == W15_TAG(0x10134a68)) {
        int key = Sim_100db770(S16(c, 8));

        if (GS32(0x1017e6a0) < 0x96) {
            int v = Sim_100b3839((uint32_t)key);

            if (v != -1) {
                G32(0x1017eec0 + G32(0x1017e6a0) * 4) = (uint32_t)v;
                G32(0x1017e6a0) += 1;
            } else {
                W15_REPORT(0x2f);
            }
        }
    } else if (tag == W15_TAG(0x10134a6c)) {
        if (GS32(0x1015dc64) >= 0x19)
            return 0;
        W15_Gp(s, c);
    } else if (tag == W15_TAG(0x10134a70)) {
        /* CPTF: nothing */
    } else if (tag == W15_TAG(0x10134a74)) {
        W15_Nested(s, c, 0x31);
    } else if (tag == W15_TAG(0x10134a78)) {
        W15_Nested(s, c, 0x32);
    } else if (tag == W15_TAG(0x10134a7c)) {
        if (G32(0x1017e6b8) != 0 && G32(0x1017e7e8) != 0) {
            Sim_100466b6((const int16_t *)(c + 8));
            G32(0x1017e7e8) = 0;
        }
    } else if (tag == W15_TAG(0x10134a80)) {
        W15_Mgdf(c);
    } else if (tag == W15_TAG(0x10134a84)) {
        if (G32(0x1017e6b8) != 0) {
            int key = Sim_100db770(S16(c, 8));
            int idx = Sim_100b3839((uint32_t)key);
            uint32_t v;

            if (idx >= 0 && (v = Sim_1003b417(idx)) != 0)
                U32(W15_GEAR(), 0x4c) = v;
        }
    } else if (tag == W15_TAG(0x10134ad8)) {
        if (G32(0x1017e6b8) != 0) {
            int key = Sim_100db770(S16(c, 8));
            uint32_t u = U32(W15_GEAR(), 0x28), v;
            int idx = Sim_100b3839((uint32_t)key);

            if (idx >= 0 && (v = Sim_1003b417(idx)) != 0)
                U32(u, (int32_t)S16(c, 0xa) * 4 + 0x498) = v;
        }
    } else if (tag == W15_TAG(0x10134a88)) {
        W15_Gt(s, c);
    } else if (tag == W15_TAG(0x10134a8c)) {
        int key = Sim_100db770(S16(c, 8)), v = S16(c, 0xa), idx;

        if (v != -1 && (idx = Sim_100b3839((uint32_t)key)) != -1)
            Sim_1003b50f(idx, (uint16_t)v);
    } else if (tag == W15_TAG(0x10134a90)) {
        W15_ShotSlot(S16(c, 8), (uint32_t)(int32_t)S16(c, 0xa));
    } else if (tag == W15_TAG(0x10134b68)) {
        W15_Hgbt(c);
    } else if (tag == W15_TAG(0x10134a94)) {
        W15_BlastSlot(S16(c, 8), S16(c, 0xa), S16(c, 0xc), 3);
    } else if (tag == W15_TAG(0x10134b64)) {
        W15_Hgxp(c);
    } else if (tag == W15_TAG(0x10134a98)) {
        W15_Navp(s, c);
    } else if (tag == W15_TAG(0x10134a9c)) {
        W15_Navo(s, c);
    } else if (tag == W15_TAG(0x10134aa0)) {
        int32_t key = S16(c, 8), idx;

        if (key != -1) {
            key = Sim_100db770(key);
            idx = Sim_100b38c5(key);
            if (idx != -1) {
                G32(0x1015e540) = 1;
                G32(0x1013a018) = 1;
                G32(0x1013a01c) = (uint32_t)idx;
            }
        }
    } else if (tag == W15_TAG(0x10134aa4)) {
        W15_Tsk(c);
    } else if (tag == W15_TAG(0x10134aa8)) {
        W15_Place(c, 0);
    } else if (tag == W15_TAG(0x10134aac)) {
        W15_Place(c, 1);
    } else if (tag == W15_TAG(0x10134ab0)) {
        s->ok &= (uint32_t)Sim_100dafa8(c, SIM_FN(W7qVisitFn, W15_SELF_THUNK));
    } else if (tag == W15_TAG(0x10134ab4)) {
        Sim_100db205(c);
    } else if (tag == W15_TAG(0x10134ab8)) {
        return W15_Gps(s, c);
    } else if (tag == W15_TAG(0x10134abc)) {
        Sim_100db77e(0);
    } else if (tag == W15_TAG(0x10134ac0)) {
        G32(0x1017e6ac) += 0x10000;
        Sim_100db77e((int)G32(0x1017e6ac));
    } else if (tag == W15_TAG(0x10134acc)) {
        Sim_10048112((uint32_t)(int32_t)S16(c, 8), (uint32_t)(int32_t)S16(c, 0xa), S16(c, 0xc),
                     (const int16_t *)(c + 0xe));
    } else if (tag == W15_TAG(0x10134b40)) {
        Sim_100d8936(c + 8);
    } else if (tag == W15_TAG(0x10134b48)) {
        W15_Mlit(c);
    } else if (tag == W15_TAG(0x10134b4c)) {
        W15_Camo(c);
    } else if (tag == W15_TAG(0x10134b54)) {
        Sim_100da805((const char *)c + 0xa);
    } else if (tag == W15_TAG(0x10134b58)) {
        Sim_100daa08((const char *)c + 0xa);
    } else if (tag != W15_TAG(0x101349ec) && tag != W15_TAG(0x101349f0)) {
        W15_REPORT(8);
    }
    return 1;
}

// FUNCTION: HGSIM 0x100dcc12
uint32_t __cdecl Sim_100dcc12(uint8_t *file)
{
    W15State s;
    uint8_t *c;

    s.file = file;
    s.ok = 1;
    s.order = 0xffffffffu;
    s.gp = 0;
    s.track = Sim_1007950a((const char *)file + 6);
    s.track0 = s.track;
    s.name = (const char *)file + 6;
    s.name0 = s.name;
    while ((c = Sim_100279f1(file)) != 0) {
        Net_ProcessMessages();
        if (!W15_Chunk(&s, c))
            break;
    }
    if (s.track != 0)
        Sim_100797f2();
    return s.ok;
}
