#include "w7m.h"
#include "fns.h"
#include "w3r.h"
#include "w3s.h"
#include "../unsorted_1002/w3q.h"
#include "../unsorted_1002/w6d.h"
#include "../unsorted_100f/w2e.h"
#include "../view/view.h"
#include "../unsorted_1002/w3o.h"
#include "../unsorted_1009/w3o.h"
#include "../unsorted_1009/w3p.h"
#include "../unsorted_1009/w6g.h"

// FUNCTION: HGSIM 0x10031acd
int __cdecl Sim_10031acd(const void *node, const void *mesh, float x, float y, float z)
{
    const uint8_t *verts;
    int32_t i;
    int r;
    float hit;

    if (node == 0)
        return 0;
    if (!(x >= W7M_F(node, 0)) || x > W7M_F(node, 4) || !(z >= W7M_F(node, 0x10)) || z > W7M_F(node, 0x14))
        return 0;
    if (y > W7M_F(node, 0xc))
        return 2;
    if (!(y >= W7M_F(node, 8)))
        return 3;
    if (U32(node, 0x18) == 0) {
        for (i = 0; i < 4; i++) {
            r = Sim_10031acd(BYTES(U32(node, 0x1c + 4 * i)), mesh, x, y, z);
            if (r != 0)
                return r;
        }
        return 2;
    }
    verts = (const uint8_t *)mesh + 0x18;
    for (i = 0; i < S32(node, 0x18); i++) {
        const void *poly = BYTES(U32(node, 0x2c + 4 * i));

        if (W7M_F(poly, 0x1c) > W7M_GF(0x101112e8) && Sim_1002e9f5(poly, verts, x, z) != 0)
            return Sim_1002d8be(poly, verts, x, y, z, &hit) != 0 ? 3 : 2;
    }
    return 2;
}

// FUNCTION: HGSIM 0x10031f95
int __cdecl Sim_10031f95(const void *node, const void *mesh, float x, float y, float z, float *out)
{
    const uint8_t *verts;
    int32_t i;

    if (node == 0)
        return 0;
    if (!(x >= W7M_F(node, 0)) || x > W7M_F(node, 4) || !(z >= W7M_F(node, 0x10)) || z > W7M_F(node, 0x14))
        return 0;
    if (U32(node, 0x18) == 0) {
        for (i = 0; i < 4; i++) {
            if (Sim_10031f95(BYTES(U32(node, 0x1c + 4 * i)), mesh, x, y, z, out) != 0)
                return 1;
        }
        *(u32a *)out = 0;
        return 1;
    }
    verts = (const uint8_t *)mesh + 0x18;
    for (i = 0; i < S32(node, 0x18); i++) {
        const void *poly = BYTES(U32(node, 0x2c + 4 * i));

        if (W7M_F(poly, 0x1c) > W7M_GF(0x101112e8) && Sim_1002e9f5(poly, verts, x, z) != 0) {
            Sim_1002d8be(poly, verts, x, y, z, out);
            return 1;
        }
    }
    *(u32a *)out = 0;
    return 1;
}

// FUNCTION: HGSIM 0x100339e8
int __cdecl Sim_100339e8(uint32_t *rec, const void *obj)
{
    uint32_t type;
    int r;

    if (rec == 0 || obj == 0)
        return 0;
    type = U16(obj, 2) & 0xf0;
    if (type == 0xd0)
        rec[0] = 1;
    else if (type == 0xe0)
        rec[0] = 2;
    else
        rec[0] = 0;
    rec[1] = U32(obj, 0x44);
    rec[2] = U32(obj, 0x48);
    rec[3] = U32(obj, 0x4c);
    rec[4] = 0;
    rec[5] = 0xffffffffu;
    rec[6] = (uint32_t)(uintptr_t)obj;
    rec[7] = U32(obj, 0x50);
    r = Sim_10033744((uint8_t *)rec);
    Sim_1002fba5((void *)obj, (float *)(rec + 11), (float *)(rec + 12), (float *)(rec + 13), (float *)(rec + 14),
                 (float *)(rec + 15), (float *)(rec + 16));
    return r;
}

// FUNCTION: HGSIM 0x10033aeb
int __cdecl Sim_10033aeb(void)
{
    int ok = 1;
    int r;
    int32_t list_index = 0;
    int32_t count = 0;
    uint32_t list, node;
    uint32_t *rec = SIM_PTR(uint32_t, 0x109e4cc4);

    Mem_Fill(SIM_PTR(void, 0x109e4cc0), 0, 0x3524);
    Sim_10087c86();
    for (list = SIM_PTR(u32a, 0x1036f2a0)[list_index]; list != 0; list = SIM_PTR(u32a, 0x1036f2a0)[list_index]) {
        list_index++;
        for (node = U32(list, 0x10); ok != 0 && node != 0; node = U32(node, 0x10)) {
            if (count >= 200)
                continue;
            if ((U16(node, 2) & 0xf0) != 0xd0 && (U16(node, 2) & 0xf0) != 0xe0)
                continue;
            r = Sim_100339e8(rec, BYTES(node));
            G32(0x109e4cc0) = G32(0x109e4cc0) + 1;
            count++;
            rec += 0x11;
            if (r == 0 || count >= 200)
                ok = 0;
        }
    }
    return ok;
}

// FUNCTION: HGSIM 0x10033eb3
int __cdecl Sim_10033eb3(float x, float y, float z, float radius, uint32_t *list)
{
    int32_t total = GS32(0x109e4cc0);
    int32_t count = 0;
    int32_t i;
    float limit, dist;
    double d;
    uint32_t *pair = SIM_PTR(uint32_t, 0x1015dd78);
    uint8_t *rec = SIM_PTR(uint8_t, 0x109e4cc4);

    if (total == 0)
        return 0;
    if (list == 0)
        return 0;
    Mem_Fill(SIM_PTR(void, 0x109e3e40), 0, 0x320);
    Mem_Fill(SIM_PTR(void, 0x1015dd78), 0, 0x320);
    limit = (float)((double)radius * (double)radius);
    for (i = 0; i < total; i++, rec += 0x44) {
        if (Sim_1003356a(rec) != 0)
            continue;
        d = Entry_DistSq(rec, x, y, z);
        dist = (float)d;
        if (!(d >= (double)limit)) {
            pair[0] = (uint32_t)i;
            *(w7m_f32 *)&pair[1] = dist;
            pair += 2;
            count++;
        }
    }
    /* sorted by the comparison routine behind the thunk at 0x10001488 */
    W7M_QSORT(SIM_PTR(void, 0x1015dd78), (uint32_t)count, 8, SIM_PTR(void, 0x10001488));
    for (i = 0; i < count; i++)
        SIM_PTR(u32a, 0x109e3e40)[i] = SIM_PTR(u32a, 0x1015dd78)[2 * i];
    *(u32a *)list = (uint32_t)(uintptr_t)SIM_PTR(void, 0x109e3e40);
    return count;
}

// FUNCTION: HGSIM 0x1003387d
int __cdecl Sim_1003387d(const void *obj, const uint32_t *vec, float back, uint32_t *out)
{
    uint32_t seg[0xe];
    uint32_t cx, cy, cz;
    float dx, dz;
    uint32_t px, pz;

    if (obj == 0 || U32(obj, 0x18) == 0 || U32(U32(obj, 0x18), 0x30) == 0 || vec == 0 || out == 0)
        return 0;
    cx = U32(obj, 0x20);
    cy = U32(obj, 0x24);
    cz = U32(obj, 0x28);
    dx = (float)((double)W7m_Float(vec[0]) - (double)W7m_Float(cx));
    dz = (float)((double)W7m_Float(vec[2]) - (double)W7m_Float(cz));
    /* the segment from the centre mirrored at vec to vec; the original computes the mirrored y as well, but then
     * uses the centre's y for both ends (the y results are unused, so they are left out here) */
    px = W7m_Bits((float)((double)W7m_Float(cx) - (double)dx));
    pz = W7m_Bits((float)((double)W7m_Float(cz) - (double)dz));
    Sim_1009cab0((float *)seg, W7m_Float(px), W7m_Float(cy), W7m_Float(pz), W7m_Float(vec[0]), W7m_Float(cy),
                 W7m_Float(vec[2]));
    Sim_1009cc82(seg);
    if (W7M_CALL_10034111(U32(obj, 0x18), seg, (const uint8_t *)obj + 0x2c) == 0)
        return 0;
    Sim_1009cd36(seg, (float)((double)W7m_Float(seg[0x30 / 4]) - (double)back));
    out[0] = seg[0xc / 4];
    out[2] = seg[0x14 / 4];
    W7M_F(out, 4) = (float)Sim_1002d961(W7m_Float(seg[0xc / 4]), W7m_Float(seg[0x10 / 4]), W7m_Float(seg[0x14 / 4]));
    return 1;
}
