#include "w7l.h"
#include "w3o.h"
#include "w5a.h"
#include "../unsorted_1008/w3o.h"
#include "../unsorted_1009/w2e.h"
#include "../unsorted_1009/w3o.h"
#include "../unsorted_1009/w6g.h"
#include "../util/w3o.h"

// FUNCTION: HGSIM 0x1002cf23
int __cdecl Sim_1002cf23(void *obj, void *seg)
{
    uint8_t *box;
    uint32_t near_, far_, n2, f2;
    int axis;
    float length;

    Sim_1002fa3f(obj);
    box = BYTES(U32(obj, 0x98));
    if (box == 0)
        return 0;
    if (W7L_CALL_1009cfd4(U32(seg, 0), U32(seg, 0x24), U32(box, 0), U32(box, 4), &near_, &far_) != 0)
        return 0;
    axis = 0;
    if (W7L_CALL_1009cfd4(U32(seg, 4), U32(seg, 0x28), U32(box, 8), U32(box, 0xc), &n2, &f2) != 0)
        return 0;
    if (Float_OfBits(n2) > Float_OfBits(near_)) {
        axis = 1;
        near_ = n2;
    }
    if (!(Float_OfBits(f2) >= Float_OfBits(far_)))
        far_ = f2;
    if (W7L_CALL_1009cfd4(U32(seg, 8), U32(seg, 0x2c), U32(box, 0x10), U32(box, 0x14), &n2, &f2) != 0)
        return 0;
    if (Float_OfBits(n2) > Float_OfBits(near_)) {
        near_ = n2;
        axis = 2;
    }
    if (!(Float_OfBits(f2) >= Float_OfBits(far_)))
        far_ = f2;
    if (Float_OfBits(near_) > Float_OfBits(far_))
        return 0;
    if (near_ & 0x80000000u) {
        G32(0x1015dd3c) = near_;
        near_ = 0;
    }
    length = Sim_1009cbde(seg);
    if (Float_OfBits(far_) > length)
        far_ = Bits_OfFloat(length);
    if (Float_OfBits(near_) > Float_OfBits(far_))
        return 0;
    Sim_1009cd36(seg, Float_OfBits(near_));
    switch (axis) {
    case 0:
        G32(0x1015dd48) = 0;
        G32(0x1015dd44) = G32(0x1015dd48);
        G32(0x1015dd40) = FL(seg, 0x18) > GFL(0x101112c0) ? 0xbf800000u : 0x3f800000u;     /* -1.0 / 1.0 */
        break;
    case 1:
        G32(0x1015dd48) = 0;
        G32(0x1015dd40) = G32(0x1015dd48);
        G32(0x1015dd44) = FL(seg, 0x1c) > GFL(0x101112c0) ? 0xbf800000u : 0x3f800000u;
        break;
    case 2:
        G32(0x1015dd44) = 0;
        G32(0x1015dd40) = G32(0x1015dd44);
        G32(0x1015dd48) = FL(seg, 0x20) > GFL(0x101112c0) ? 0xbf800000u : 0x3f800000u;
        break;
    }
    return 1;
}

// FUNCTION: HGSIM 0x1002e790
int __cdecl Sim_1002e790(void *face, void *points, void *seg)
{
    uint32_t nx = U32(face, 0x18), ny = U32(face, 0x1c), nz = U32(face, 0x20);
    float den, d, num, t;
    double numd, td;
    uint8_t *p;
    uint32_t px, py, pz;

    den = (float)((double)Float_OfBits(nx) * FL(seg, 0x24) + (double)Float_OfBits(ny) * FL(seg, 0x28)
                  + (double)Float_OfBits(nz) * FL(seg, 0x2c));
    if ((Bits_OfFloat(den) & 0x80000000u) == 0)
        return 0;
    p = BYTES(points) + (uint32_t)U8(face, U32(face, 4)) * 0x58;
    d = (float)-((double)Float_OfBits(nx) * FL(p, 0xc) + (double)Float_OfBits(ny) * FL(p, 0x10)
                 + (double)Float_OfBits(nz) * FL(p, 0x14));
    numd = (double)Float_OfBits(nx) * FL(seg, 0) + (double)Float_OfBits(ny) * FL(seg, 4)
           + (double)Float_OfBits(nz) * FL(seg, 8) + d;
    num = (float)numd;
    if (!(numd >= GFL(0x101112d4)))
        return 0;
    td = -(double)num / den;
    t = (float)td;
    if (!(td >= GFL(0x101112d4)) || t > FL(seg, 0x30))
        return 0;
    px = Bits_OfFloat((float)((double)FL(seg, 0x24) * t + FL(seg, 0)));
    py = Bits_OfFloat((float)((double)FL(seg, 0x28) * t + FL(seg, 4)));
    pz = Bits_OfFloat((float)((double)FL(seg, 0x2c) * t + FL(seg, 8)));
    if (W7L_CALL_1002e91e(face, points, px, py, pz) == 0)
        return 0;
    Sim_1009ceab(seg, Float_OfBits(px), Float_OfBits(py), Float_OfBits(pz));
    G32(0x1015dd40) = nx;
    G32(0x1015dd44) = ny;
    G32(0x1015dd48) = nz;
    return 1;
}

// FUNCTION: HGSIM 0x1002d5e0
int __cdecl Sim_1002d5e0(void *obj, void *seg)
{
    uint8_t *model = BYTES(U32(obj, 0x30));
    int i;

    if (model == 0)
        return 0;
    for (i = 0; i < S16(model, 6); i++) {
        if (Sim_1002e790(model + U32(model, 8) + i * 0x28, model + 0x18, seg) != 0)
            return 1;
    }
    return 0;
}

/* The search of Sim_1002dda9 (skip_kind6 0) and Sim_1002e362 (skip_kind6 1). */
static int W7l_NearestHit(void *seg, uint32_t *out, int id, int skip_kind6)
{
    uint32_t hit_node = 0, best = 0x7f7fc99eu;      /* 3.4e38 */
    uint8_t copy[0x38], cut[0x38], hit[0x38];
    uint32_t dist, a, b, c;
    uint32_t list;
    int k = 0;

    if (!(Sim_1009cbde(seg) > GFL(0x101112c4)))
        return Sim_1002dc60(FL(seg, 0), FL(seg, 4), FL(seg, 8), (void **)out);
    if (G32(0x1017c8cc) == 0)
        return 0;
    Sim_100881dd(FL(seg, 0), FL(seg, 8));
    while ((list = G32(0x1036f2a0 + 4 * k)) != 0) {
        uint32_t node;

        k++;
        for (node = U32(list, 0x18); node != 0; node = U32(node, 0x18)) {
            uint32_t found;

            if (skip_kind6) {
                if ((U16(node, 2) & 0x100) != 0 || U32(node, 0x34) == 6)
                    continue;
            } else if ((U16(node, 2) & 0x100) != 0 && ((uint32_t)U16(node, 0x24) == (uint32_t)id || id < 0)) {
                continue;
            }
            dist = best;
            Sim_1009cfbd(copy, seg);
            found = W7L_CALL_1002dfbb(node, copy, cut, &dist, &a, &b, &c);
            if (found != 0 && !(Float_OfBits(dist) >= Float_OfBits(best))) {
                best = dist;
                Sim_1009cfbd(hit, cut);
                hit_node = found;
                G32(0x1015dd58) = a;
                G32(0x1015dd5c) = b;
                G32(0x1015dd60) = c;
            }
        }
    }
    *(u32a *)out = hit_node;
    if (hit_node == 0)
        return 0;
    Sim_1009cfbd(seg, hit);
    return 1;
}

// FUNCTION: HGSIM 0x1002dda9
int __cdecl Sim_1002dda9(void *seg, uint32_t *out, int id)
{
    return W7l_NearestHit(seg, out, id, 0);
}

// FUNCTION: HGSIM 0x1002e362
int __cdecl Sim_1002e362(void *seg, uint32_t *out)
{
    return W7l_NearestHit(seg, out, 0, 1);
}

// FUNCTION: HGSIM 0x1002e569
int __cdecl Sim_1002e569(void *obj, void *seg, float t)
{
    uint32_t kind, fn;

    if (!(Sim_1009cbde(seg) > GFL(0x101112c4)))
        return 1;
    kind = U32(obj, 0x34);
    fn = G32(0x10138bc4 + kind * 0xc);
    if (fn != 0)
        return SIM_FN(int(__cdecl *)(void *, void *), fn)(obj, seg);
    fn = G32(0x10138bc0 + kind * 0xc);
    if (fn != 0) {
        float x = FL(seg, 0), y = FL(seg, 4), z = FL(seg, 8);
        float sx = (float)(((double)FL(seg, 0xc) - x) / GFL(0x101112d0));
        float sy = (float)(((double)FL(seg, 0x10) - y) / GFL(0x101112d0));
        float sz = (float)(((double)FL(seg, 0x14) - z) / GFL(0x101112d0));
        int i;

        for (i = 0; i < 4; i++) {
            x = (float)((double)x + sx);
            y = (float)((double)y + sy);
            z = (float)((double)z + sz);
            if (SIM_FN(int(__cdecl *)(void *, uint32_t, uint32_t, uint32_t), fn)(obj, Bits_OfFloat(x), Bits_OfFloat(y),
                                                                                Bits_OfFloat(z)) != 0) {
                Sim_1009ceab(seg, x, y, z);
                G32(0x1015dd48) = 0;
                G32(0x1015dd44) = G32(0x1015dd48);
                G32(0x1015dd40) = G32(0x1015dd44);
                return 1;
            }
        }
        return 0;
    }
    Sim_1002e6de(obj, (float *)seg, t);
    return 1;
}
