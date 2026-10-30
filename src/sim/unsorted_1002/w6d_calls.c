#include "w6d.h"
#include "w2e.h"
#include "w2i.h"
#include "w4v.h"
#include "w5a.h"
#include "w5y.h"
#include "w5z.h"
#include "../unsorted_1000/w3o.h"
#include "../unsorted_1004/w4w.h"
#include "../unsorted_1005/w2g.h"
#include "../unsorted_1008/w3o.h"
#include "../unsorted_100a/w5x.h"
#include "../unsorted_100a/w5z.h"
#include "../unsorted_100b/w2j.h"
#include "../unsorted_100f/w2e.h"
#include "../util/util.h"
#include "../util/w3p.h"
#include "../win/w4w_api.h"
#include "../util/access.h"

static float BitsFloat(uint32_t u)
{
    union {
        uint32_t u;
        float f;
    } v;

    v.u = u;
    return v.f;
}

// FUNCTION: HGSIM 0x10024548
void __cdecl Sim_10024548(uint32_t a, int index, int flag, uint32_t b, uint32_t c, uint32_t d, uint32_t e, uint32_t f,
                          uint32_t g)
{
    uint32_t rec = 0x1096fc80u + (uint32_t)index * 0x156u;
    uint32_t value;

    if (G32(rec + 0x60) != 0)
        G32(0x10134890) = 0xffffffffu;
    Sim_100aa351((void *)(uintptr_t)G32(rec + 0x3c));
    Sim_100aa406((void *)(uintptr_t)G32(rec + 0x3c));
    value = G32(rec + 8);
    Sim_100585b3((int)G32(rec + 0x68));
    Sim_100ac58e((int)G32(rec + 0x6c));
    G32(rec + 0x68) = 0xffffffffu;
    G32(rec + 0x6c) = 0xffffffffu;
    if (flag != 0)
        W6D_CALL_100246b3(G32(rec + 0x50), a, value, b, c, d, e, f, g, 0, 0, 0);
    Sim_10021c3c(index);
}

// FUNCTION: HGSIM 0x100257ec
void __cdecl Sim_100257ec(uint32_t a, float x, float y, float z, float extra, float scale)
{
    uint32_t n = G32(0x1015dc68);

    while (n-- != 0) {
        uint32_t entry = 0x109e9ba0u + n * 0x50u;
        uint32_t obj;
        uint32_t px, py, pz;
        float radius, dx, dy, dz, limit;

        if ((S16(SIM_PTR(uint8_t, entry), 0) & 4) != 0)
            continue;
        obj = Sim_100b53bb((int)G32(entry + 8));
        if (obj == 0)
            continue;
        radius = Sim_1000946e((void *)(uintptr_t)obj, &px, &py, &pz);
        dx = (float)((double)BitsFloat(px) - (double)x);
        dy = (float)((double)BitsFloat(py) - (double)y);
        dz = (float)((double)BitsFloat(pz) - (double)z);
        limit = (float)((double)radius + (double)extra);
        if (X87_LessOrUnordered(Sim_10084a94(dx, dy, dz), (double)limit))
            W6D_CALL_10025aee(a, obj, (float)((double)GS32(0x1015dc80) * (double)scale), px, py, pz);
    }
}

// FUNCTION: HGSIM 0x10025f6b
uint32_t __cdecl Sim_10025f6b(void *obj)
{
    uint32_t twice = G32(0x101480a8) << 1;
    uint32_t i = 0x100;

    G32(0x1015dbd0) = 0x461c4000u; /* 10000.0 */
    while (i-- != 0) {
        uint32_t entry = 0x109a5a40u + i * 0x2cu;

        if (G32(entry + 0x24) == 0x16 && G32(entry) != 0 && U32(G32(entry), 0x8c) != 0) {
            G32(0x1015dbd0) = 0x461c4000u;
            break;
        }
    }
    W6D_CALL_100d3510(0x12, twice, 2);
    G32(0x1015dbf8) = U32(obj, 0x58);
    G32(0x1015dbfc) = U32(obj, 0x5c);
    G32(0x1015dc00) = U32(obj, 0x60);
    G32(0x1015dc0c) = twice;
    G32(0x1015dbf4) = 0;
    return Sim_10024616(0xfffffffeu, 0x16, G32(0x1015dbf8), G32(0x1015dbfc), G32(0x1015dc00), G32(0x1015dbf8),
                        G32(0x1015dbfc), G32(0x1015dc00));
}

// FUNCTION: HGSIM 0x100279f1
void *__cdecl Sim_100279f1(void *obj)
{
    uint32_t block = U32(obj, 0x1b);
    uint32_t length = U32(block, 0) == G32(0x101349e8) ? 0xc : U32(block, 4);
    uint32_t next = U32(obj, 0x1b) + length;

    if ((int32_t)(next - U32(obj, 0x17)) >= S32(obj, 0xf))
        return 0;
    if (S32(next, 4) <= S32(obj, 0x13) && U32(next, 4) >= 8) {
        U32(obj, 0x1b) = next;
        return (void *)(uintptr_t)next;
    }
    W6D_CALL_100bc280_5(0x2c, SIM_PTR(const char, 0x10134fd8), BYTES(obj) + 6, S16(obj, 4), 0);
    return 0;
}

// FUNCTION: HGSIM 0x10027e6a
void __cdecl Sim_10027e6a(void)
{
    int i;

    if (G32(0x1015dc30) == 0)
        return;
    for (i = 0; i < 0x3f1; i++) {
        uint32_t node = U32(G32(0x1015dc30), (uint32_t)i * 4);

        while (node != 0) {
            uint32_t next = U32(node, 8);

            W6D_CALL_10027fb8((void *)(uintptr_t)node);
            node = next;
        }
    }
    G32(0x109f13e0) = 0;
    G32(0x109f13f0) = 0;
    G32(0x109f13f4) = 0;
    SIM_API(W4wHeapFreeFn, W4W_IAT_HEAPFREE)(W4W_GAME_HEAP, 1, (void *)(uintptr_t)G32(0x1015dc30));
}

// FUNCTION: HGSIM 0x10028b90
int __cdecl Sim_10028b90(uint32_t key, uint32_t value)
{
    uint32_t entry;
    uint32_t bits;
    int i;

    if (key == 0)
        return -1;
    for (i = 0; i < 0x80; i++) {
        if (G32(0x109eeb04u + (uint32_t)i * 0x30u) == key)
            break;
    }
    if (i == 0x80) {
        for (i = 0; i < 0x80; i++) {
            if (G32(0x109eeb04u + (uint32_t)i * 0x30u) == 0)
                break;
        }
    }
    if (i == 0x80)
        return -1;
    Sim_1002990f(i);
    G32(0x1015dc40) = G32(0x1015dc40) + 1;
    W6D_CALL_100aa8cb(key);
    entry = 0x109eeb00u + (uint32_t)i * 0x30u;
    G32(entry) = value;
    G32(entry + 4) = key;
    /* fld / fchs / fstp: the sign flips, and a signalling NaN comes out quiet as it does through the x87 */
    bits = G32(0x1013abc0) ^ 0x80000000u;
    if ((bits & 0x7f800000u) == 0x7f800000u && (bits & 0x7fffffu) != 0)
        bits |= 0x400000u;
    G32(entry + 0x20) = bits;
    return i;
}

// FUNCTION: HGSIM 0x100299c8
void __cdecl Sim_100299c8(float x, float y, float z, float extra, float scale)
{
    uint32_t n = 0x80;

    while (n-- != 0) {
        uint32_t entry = 0x109f09e0u + n * 0x14u;
        uint32_t px, py, pz;
        float radius, dx, dy, dz, limit;
        double dist2;

        if (G32(entry) == 0 || G32(entry + 4) == 0)
            continue;
        radius = Sim_1000946e((void *)(uintptr_t)Sim_100a9b71((void *)(uintptr_t)G32(entry + 4)), &px, &py, &pz);
        dx = (float)((double)BitsFloat(px) - (double)x);
        dy = (float)((double)BitsFloat(py) - (double)y);
        dz = (float)((double)BitsFloat(pz) - (double)z);
        limit = (float)((double)radius + (double)extra);
        dist2 = (double)dx * dx + (double)dy * dy;
        dist2 = dist2 + (double)dz * dz;
        if (!X87_LessOrUnordered((double)limit * limit, dist2))
            Sim_100291a2((int)n, (float)((double)GS32(0x1015dc80) * (double)scale));
    }
}

// FUNCTION: HGSIM 0x1002a1f2
void __cdecl Sim_1002a1f2(int index, void(__cdecl *callback)(int, void *))
{
    uint32_t rec;

    PTRTAB(index) = 0;
    if (Sim_1002a29f(index) == 0)
        return;
    rec = PTRTAB(index);
    W6D_CALL_1002a2f6((void *)(uintptr_t)rec);
    U32(rec, 4) = (uint32_t)index;
    if ((uint32_t)index == G32(0x1017b8c8))
        U32(rec, 0x54) = (uint32_t)(uintptr_t)SIM_PTR(uint8_t, 0x1015fd38);
    else
        U32(rec, 0x54) = rec + 0x2a0;
    if (U32(rec, 0x54) != 0)
        Mem_Fill((void *)(uintptr_t)U32(rec, 0x54), 0, 0x5c);
    if (callback != 0)
        callback(index, (void *)(uintptr_t)rec);
}

// FUNCTION: HGSIM 0x1002c63a
int __cdecl Sim_1002c63a(void)
{
    int i;

    if (U8(G32(0x1017b8c4), 9) != 0 || G32(0x1015ec58) == 0)
        W6D_CALL_1009bd47();
    for (i = 0; i < 0x1e; i++) {
        uint32_t obj = SIM_PTR(u32a, 0x109e88e0)[i];

        if (U32(obj, 0x108) != 0)
            ((void(__cdecl *)(void *))(uintptr_t)U32(SIM_PTR(u32a, 0x109e88e0)[i], 0x108))(
                (void *)(uintptr_t)SIM_PTR(u32a, 0x109e88e0)[i]);
        SIM_API(W4wHeapFreeFn, W4W_IAT_HEAPFREE)(W4W_GAME_HEAP, 1, (void *)(uintptr_t)SIM_PTR(u32a, 0x109e88e0)[i]);
    }
    Sim_100481dd(-1);
    return SIM_API(W4wHeapFreeFn, W4W_IAT_HEAPFREE)(W4W_GAME_HEAP, 1, (void *)(uintptr_t)G32(0x1015dc2c));
}

// FUNCTION: HGSIM 0x1002ce45
void __cdecl Sim_1002ce45(void *obj, float x, float y, float z, int *above, int *in, uint32_t *value)
{
    uint32_t box;

    Sim_1002fa3f(obj);
    box = U32(obj, 0x98);
    if (box == 0) {
        *in = 0;
        *above = 0;
        return;
    }
    if (!X87_LessOrUnordered(x, F32(box, 0)) && X87_LessEqualOrUnordered(x, F32(box, 4)) &&
        !X87_LessOrUnordered(z, F32(box, 0x10)) && X87_LessEqualOrUnordered(z, F32(box, 0x14))) {
        *in = 1;
        *value = U32(box, 0xc);
        *above = !X87_LessOrUnordered(y, F32(box, 8)) && X87_LessEqualOrUnordered(y, F32(box, 0xc));
        return;
    }
    *in = 0;
    *above = 0;
}

/* The common body of Sim_1002fba5 and Sim_1002fd4c: off is the offset of the point in the 0x58-byte entries. */
static void BoundingBox(void *obj, uint32_t off, float *min_x, float *max_x, float *min_y, float *max_y,
                        float *min_z, float *max_z)
{
    uint32_t block = U32(obj, 0x2c);
    uint32_t entry;
    uint32_t n;

    if (block == 0) {
        *(u32a *)max_z = 0;
        *(u32a *)min_z = 0;
        *(u32a *)max_y = 0;
        *(u32a *)min_y = 0;
        *(u32a *)max_x = 0;
        *(u32a *)min_x = 0;
        return;
    }
    if (U32(obj, 0x9c) != U32(block, 0x10)) {
        uint32_t saved = U32(obj, 0x30);

        U32(obj, 0x30) = block;
        Sim_100aa307(obj);
        U32(obj, 0x30) = saved;
    }
    entry = block + 0x18;
    *(u32a *)min_z = 0x7f7fc99eu;
    *(u32a *)min_y = 0x7f7fc99eu;
    *(u32a *)min_x = 0x7f7fc99eu;
    *(u32a *)max_z = 0xff7fc99eu;
    *(u32a *)max_y = 0xff7fc99eu;
    *(u32a *)max_x = 0xff7fc99eu;
    for (n = (uint32_t)(int32_t)S16(block, 4); n-- != 0; entry += 0x58) {
        if (F32(entry, off) > *(f32a *)max_x)
            *(u32a *)max_x = U32(entry, off);
        if (F32(entry, off + 4) > *(f32a *)max_y)
            *(u32a *)max_y = U32(entry, off + 4);
        if (F32(entry, off + 8) > *(f32a *)max_z)
            *(u32a *)max_z = U32(entry, off + 8);
        if (X87_LessOrUnordered(F32(entry, off), *(f32a *)min_x))
            *(u32a *)min_x = U32(entry, off);
        if (X87_LessOrUnordered(F32(entry, off + 4), *(f32a *)min_y))
            *(u32a *)min_y = U32(entry, off + 4);
        if (X87_LessOrUnordered(F32(entry, off + 8), *(f32a *)min_z))
            *(u32a *)min_z = U32(entry, off + 8);
    }
}

// FUNCTION: HGSIM 0x1002fba5
void __cdecl Sim_1002fba5(void *obj, float *min_x, float *max_x, float *min_y, float *max_y, float *min_z,
                          float *max_z)
{
    BoundingBox(obj, 0xc, min_x, max_x, min_y, max_y, min_z, max_z);
}

// FUNCTION: HGSIM 0x1002fd4c
void __cdecl Sim_1002fd4c(void *obj, float *min_x, float *max_x, float *min_y, float *max_y, float *min_z,
                          float *max_z)
{
    BoundingBox(obj, 0, min_x, max_x, min_y, max_y, min_z, max_z);
}
