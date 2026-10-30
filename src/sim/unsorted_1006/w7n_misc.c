#include "w7n.h"
#include "w3o.h"
#include "w3p.h"
#include "w3t.h"
#include "w6e.h"
#include "../unsorted_1008/w3o.h"
#include "../unsorted_100a/w5x.h"
#include "../util/util.h"
#include "../util/w3o.h"

// FUNCTION: HGSIM 0x10063415
int __cdecl Sim_10063415(int noDraw, uint32_t id, uint32_t x, uint32_t y, int kind)
{
    int32_t step;

    if (G32(0x10144f4c) != id) {
        G32(0x10144f28) = x;
        G32(0x10144f2c) = y;
        GS32(0x10144f30) = GS32(0x109e3e1c) >> 1;
        GS32(0x10144f34) = GS32(0x109e3e1c) >> 1;
        G32(0x10144f48) = 1;
        if (kind == 0)
            G32(0x10144f44) = 0xc;
        else if (kind == 2)
            G32(0x10144f44) = 0x14;
        else
            G32(0x10144f44) = 4;
        G32(0x10144f4c) = id;
        if (noDraw == 0)
            Sim_1006326b(SIM_PTR(uint32_t, 0x10144f28));
        return 0;
    }
    if (noDraw != 0)
        return 0;
    if (GS32(0x10144f34) > 0) {
        step = Render_Ftol((double)GS32(0x1015dc80) * (double)GFL(0x10144f38));
        Sim_1006326b(SIM_PTR(uint32_t, 0x10144f28));
        G32(0x10144f30) = G32(0x10144f30) - (uint32_t)step;
        G32(0x10144f34) = G32(0x10144f34) - (uint32_t)step;
        G32(0x10144f28) = x;
        G32(0x10144f2c) = y;
    }
    return GS32(0x10144f84) > 0 ? 0 : 1;
}

// FUNCTION: HGSIM 0x10067356
void *__cdecl Sim_10067356(uint8_t *obj, float r, float *dist, float *limit, float x, float y, float z, float *dx,
                           float *dy, float *dz)
{
    void *best = 0;
    void *hit;
    uint8_t *list;
    uint32_t node;
    float childDist, childLimit = Float_OfBits(0x7f7fc99eu);
    float childDx, childDy, childDz;

    *dx = x - FL(obj, 0x60);
    *dy = y - FL(obj, 0x64);
    *dz = z - FL(obj, 0x68);
    *limit = FL(obj, 0x6c) + r;
    *dist = (float)Sim_10084a94(*dx, *dy, *dz);
    if (*dist >= *limit)
        return 0;
    list = BYTES(U32(obj, 0x28));
    if (list == 0 || U32(list, 4) == 0) {
        if (!(*dist >= *limit) && (U16(obj, 0) & 0x800) == 0)
            best = obj;
        return best;
    }
    *dx = x - FL(obj, 0x44);
    *dy = y - FL(obj, 0x48);
    *dz = z - FL(obj, 0x4c);
    *limit = FL(obj, 0x50) + r;
    *dist = (float)Sim_10084a94(*dx, *dy, *dz);
    if (*dist > *limit) {
        childDist = Float_OfBits(0x7f7fc99eu);
        childLimit = Float_OfBits(0x7f7fc99eu);
        best = 0;
    } else {
        best = obj;
    }
    for (node = U32(list, 4); node != 0; node = U32(node, 8)) {
        hit = Sim_10067356(BYTES(U32(node, 0x8c)), r, &childDist, &childLimit, x, y, z, &childDx, &childDy,
                           &childDz);
        if (hit != 0 && !(childDist >= *dist)) {
            *dist = childDist;
            *limit = childLimit;
            *dx = childDx;
            *dy = childDy;
            *dz = childDz;
            best = hit;
        }
    }
    return best;
}

// FUNCTION: HGSIM 0x10068e77
int __cdecl Sim_10068e77(uint8_t **a, uint8_t **b, float *outX, float *outY, float *outZ)
{
    union {
        float f;
        uint32_t u;
    } aPos[3], aAng[3], bPos[3], bAng[3], p[3];
    const float *box;
    int i, j, k, inside = 0;

    Sim_100a9cfb(BYTES(U32(*a, 0x48)), &aPos[0].u, &aPos[1].u, &aPos[2].u);
    Sim_100a9ca3(BYTES(U32(*a, 0x48)), (uint32_t)(uintptr_t)&aAng[0].u, (uint32_t)(uintptr_t)&aAng[1].u,
                 (uint32_t)(uintptr_t)&aAng[2].u);
    Sim_100a9cfb(BYTES(U32(*b, 0x48)), &bPos[0].u, &bPos[1].u, &bPos[2].u);
    Sim_100a9ca3(BYTES(U32(*b, 0x48)), (uint32_t)(uintptr_t)&bAng[0].u, (uint32_t)(uintptr_t)&bAng[1].u,
                 (uint32_t)(uintptr_t)&bAng[2].u);
    box = (const float *)(*b + 0x284);
    for (i = 0; i < 2; i++) {
        for (j = 0; j < 2; j++) {
            for (k = 0; k < 2; k++) {
                p[0].f = Sim_10068e55(box, 0, i);
                p[1].f = Sim_10068e55(box, 1, j);
                p[2].f = Sim_10068e55(box, 2, k);
                Sim_10068d76(p[0].f, p[1].f, p[2].f, bAng[0].f, bAng[1].f, bAng[2].f, &p[0].u, &p[1].u, &p[2].u);
                Sim_10068e1d(p[0].f, p[1].f, p[2].f, bPos[0].f, bPos[1].f, bPos[2].f, &p[0].f, &p[1].f, &p[2].f);
                Sim_10068e1d(p[0].f, p[1].f, p[2].f, -aPos[0].f, -aPos[1].f, -aPos[2].f, &p[0].f, &p[1].f,
                             &p[2].f);
                Sim_10068d76(p[0].f, p[1].f, p[2].f, -aAng[0].f, -aAng[1].f, -aAng[2].f, &p[0].u, &p[1].u,
                             &p[2].u);
                inside = Sim_10068b10((const float *)(*a + 0x284), p[0].f, p[1].f, p[2].f);
                if (inside != 0) {
                    if (outX != 0 && outY != 0 && outZ != 0) {
                        *(u32a *)outX = p[0].u;
                        *(u32a *)outY = p[1].u;
                        *(u32a *)outZ = p[2].u;
                    }
                    return inside;
                }
            }
        }
    }
    return inside;
}
