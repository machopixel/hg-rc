#include "w18.h"
#include "w3q.h"
#include "w7o.h"
#include "../util/access.h"
#include "../util/w3o.h"
#include "../unsorted_100a/w2g.h"
#include "../unsorted_100a/w5x.h"
#include "../unsorted_100f/w2e.h"

#define W18P_C(addr) ((double)GFL(addr))
#define W18P_VERT(verts, idx) ((verts) + (uint32_t)*(idx) * 0x58u)

typedef void(__cdecl *W18pVertexFn)(void *vertex);
typedef uint32_t(__cdecl *W18pPolyFn)(void *poly, void *verts, uint32_t kind, uint32_t depth);

/* the renderer's vertex callback [0x1014cb44] */
static void W18P_Out(void *vertex)
{
    ((W18pVertexFn)(uintptr_t)G32(0x1014cb44))(vertex);
}

/* the camera-space position +0x20..+0x28 of a vertex and its near / far bits, once per frame (+0x30 bit 4) */
static void W18P_Transform(uint8_t *v)
{
    float x, y, z;
    uint8_t bits = 0;

    G32(0x101cc29c) += 1;
    FL(v, 0x20) = (float)((double)FL(v, 0xc) - GFL(0x10185c48));
    FL(v, 0x24) = (float)((double)FL(v, 0x10) - GFL(0x10185c44));
    FL(v, 0x28) = (float)((double)FL(v, 0x14) - GFL(0x10185c4c));
    x = FL(v, 0x20);
    y = FL(v, 0x24);
    z = FL(v, 0x28);
    FL(v, 0x20) = (float)(((double)x * GFL(0x10185ba0) + (double)y * GFL(0x10185ba4)) + (double)z * GFL(0x10185ba8));
    FL(v, 0x24) = (float)(((double)x * GFL(0x10185bac) + (double)y * GFL(0x10185bb0)) + (double)z * GFL(0x10185bb4));
    FL(v, 0x28) = (float)(((double)x * GFL(0x10185bb8) + (double)y * GFL(0x10185bbc)) + (double)z * GFL(0x10185bc0));
    z = FL(v, 0x28);
    U8(v, 0x30) |= 4;
    if (!((double)z >= W18P_C(0x10185c50)))
        bits |= 1;
    else if ((double)z > W18P_C(0x10185bf8))
        bits |= 2;
    U8(v, 0x30) = (uint8_t)((U8(v, 0x30) & 0xfc) | bits);
}

/* the depth of the vertex list 0x101c76a0 by [0x101cc2b0]: bit 1 the mean, bit 2 the nearest, else the farthest */
static float W18P_Depth(void)
{
    uint32_t list = 0x101c76a0u, n = G32(0x101c9ea8);
    float depth;

    if ((G32(0x101cc2b0) & 2) != 0) {
        float sum = 0.0f;

        while (n-- != 0) {
            sum = (float)((double)sum + FL(G32(list), 8));
            list += 4;
        }
        depth = (float)((double)sum / (double)GS32(0x101c9ea8));
    } else if ((G32(0x101cc2b0) & 4) != 0) {
        U32(&depth, 0) = 0x7f7fc99e;
        while (n-- != 0) {
            if (!(FL(G32(list), 8) >= depth))
                U32(&depth, 0) = U32(G32(list), 8);
            list += 4;
        }
    } else {
        depth = 0.0f;
        while (n-- != 0) {
            if (FL(G32(list), 8) > depth)
                U32(&depth, 0) = U32(G32(list), 8);
            list += 4;
        }
    }
    if ((G32(0x101cc2b0) & 1) != 0)
        depth = (float)((double)depth + W18P_C(0x1012ba84));
    return depth;
}

// FUNCTION: HGSIM 0x100a65e7
void __cdecl Sim_100a65e7(uint8_t *poly, uint8_t *verts)
{
    const uint8_t *idx;
    uint8_t *v, *first, *prev, *cur;
    uint32_t n, k, table, type, edge, firstEdge, rec, depthBits;
    uint8_t andBits = 3, firstIn, prevIn, curIn;
    float depth;

    G32(0x101cc2a4) += 1;
    if (U16(poly, 2) >= 3) {
        if (G32(0x1017d14c) != 0) {
            if (!((double)FL(poly, 0x1c) >= W18P_C(0x1012ba80)))
                return;
        } else {
            float dx, dy, dz, dot;

            v = W18P_VERT(verts, poly + U32(poly, 4));
            dx = (float)((double)FL(v, 0xc) - GFL(0x10185c48));
            dy = (float)((double)FL(v, 0x10) - GFL(0x10185c44));
            dz = (float)((double)FL(v, 0x14) - GFL(0x10185c4c));
            dot = (float)(((double)FL(poly, 0x18) * dx + (double)FL(poly, 0x1c) * dy) + (double)FL(poly, 0x20) * dz);
            if ((U32(&dot, 0) & 0x80000000u) == 0)
                return;
        }
    }
    G32(0x101c767c) += 1;
    idx = poly + U32(poly, 4);
    for (n = U16(poly, 2); n-- != 0; idx++) {
        v = W18P_VERT(verts, idx);
        if ((U8(v, 0x30) & 4) == 0)
            W18P_Transform(v);
        andBits &= U8(v, 0x30);
    }
    if ((int8_t)andBits == 1 || (int8_t)andBits == 2)
        return;
    U8(GBYTES(0x101c7680), 0) = 0;
    U8(GBYTES(0x101cc2a0), 0) = 0xf;
    G32(0x101c9ea8) = 0;
    G32(0x1014ce70) = 1;
    type = U16(poly, 0) & 0xf000u;
    table = 0;
    if (type == 0xb000 || type == 0xc000 || type == 0xd000 || type == 0xe000) {
        uint32_t t = (type >> 12) - 0xb;

        G32(0x1014ce70) = 0;
        if (U16(poly, 2) == 3)
            table = (U32(poly, 8) & 1) != 0 ? 0x1014cd30u + t * 0x18u : 0x1014cd90u + t * 0x18u;
        else if (U16(poly, 2) == 4)
            table = 0x1014cdf0u + t * 0x20u;
    }
    idx = poly + U32(poly, 4);
    n = U16(poly, 2);
    first = prev = W18P_VERT(verts, idx);
    firstIn = prevIn = U8(first, 0x30) & 1;
    if (table != 0) {
        edge = firstEdge = table;
        if (prevIn == 0)
            W18P_Out(Sim_100a5c10(first, (const uint32_t *)(uintptr_t)firstEdge));
        for (k = 0; --n != 0;) {
            uint32_t e;

            idx++;
            k++;
            cur = W18P_VERT(verts, idx);
            e = table + k * 8u;
            curIn = U8(cur, 0x30) & 1;
            if (curIn != prevIn)
                W18P_Out(Render_ClipEdgeNear(prev, (const uint32_t *)(uintptr_t)edge, cur, (const uint32_t *)(uintptr_t)e));
            prev = cur;
            edge = e;
            prevIn = curIn;
            if (curIn == 0)
                W18P_Out(Sim_100a5c10(prev, (const uint32_t *)(uintptr_t)edge));
        }
        if (firstIn != prevIn)
            W18P_Out(Render_ClipEdgeNear(prev, (const uint32_t *)(uintptr_t)edge, first,
                                         (const uint32_t *)(uintptr_t)firstEdge));
    } else {
        if (prevIn == 0)
            W18P_Out(Sim_100a5d25(first));
        while (--n != 0) {
            idx++;
            cur = W18P_VERT(verts, idx);
            curIn = U8(cur, 0x30) & 1;
            if (curIn != prevIn)
                W18P_Out(Sim_100a6009((uint32_t)(uintptr_t)prev, (uint32_t)(uintptr_t)cur));
            prev = cur;
            prevIn = curIn;
            if (curIn == 0)
                W18P_Out(Sim_100a5d25(prev));
        }
        if (firstIn != prevIn)
            W18P_Out(Sim_100a6009((uint32_t)(uintptr_t)prev, (uint32_t)(uintptr_t)first));
    }
    if ((GS32(0x101c9ea8) < 3 && U16(poly, 2) > 2) || U8(GBYTES(0x101cc2a0), 0) != 0)
        return;
    rec = Sim_100a8943();
    U32(rec, 4) = (uint32_t)(uintptr_t)poly;
    U32(rec, 0xc) = G32(0x1017d12c);
    G32(0x101c6740) = G32(0x1017d178);
    U16(rec, 0) = (uint16_t)G32(0x101c9ea8);
    depth = W18P_Depth();
    depthBits = U32(&depth, 0);
    U32(rec, 8) = depthBits;
    Mem_Move((void *)(uintptr_t)G32(0x101c6740), GBYTES(0x101c76a0), G32(0x101c9ea8) * 4u);
    G32(0x101c6740) = G32(0x101c6740) + G32(0x101c9ea8) * 4u;
    G32(0x1017d178) = G32(0x101c6740);
    U16(rec, 2) = (uint16_t)((W18pPolyFn)(uintptr_t)G32(0x1014cb48))(poly, verts, U16(poly, 0), depthBits);
    if (GS32(0x1017d138) < GS32(0x1017d17c)) {
        uint32_t count = U16(poly, 2);

        G32(0x101c9e9c) += 1;
        if (GS32(0x101c9ea8) > 1)
            G32(0x1017d13c) += 1;
        U32(G32(0x101cc2ac), G32(0x1017d138) * 8u) = rec;
        U32(G32(0x101cc2ac), G32(0x1017d138) * 8u + 4u) = depthBits;
        G32(0x1017d138) += 1;
        G32(0x101c672c) = G32(0x101c672c) + count - 2u;
    } else {
        G32(0x101c9ea4) = 0;
    }
}
