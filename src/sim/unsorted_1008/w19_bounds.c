#include "w19.h"
#include "../util/access.h"
#include "../util/w3o.h"

#define W19_MARKER 0x7f7fc99eu
#define W19_DEPTHS 256

/* the minimum x each depth's last non-leaf call left in its shared frame slot ([ebp-4]) */
typedef struct W19Walk {
    uint32_t flag, out;
    uint32_t slot[W19_DEPTHS];
} W19Walk;

static int W19_Node(W19Walk *w, int depth, uint8_t *obj, uint32_t *ox, uint32_t *oy, uint32_t *oz)
{
    uint32_t node, child, minx, miny, minz, maxx, maxy, maxz, cx, cy, cz;

    if (obj == 0 || U32(obj, 0x28) == 0) {
        *oz = 0;
        *oy = 0;
        *ox = 0;
        return 0;
    }
    node = U32(obj, 0x28);
    *oz = *oy = *ox = 0;
    if (node == 0 || U32(node, 4) == 0) {
        uint32_t left = depth < W19_DEPTHS ? w->slot[depth] : 0;

        if ((U16(obj, 0) & w->flag) != 0)
            return 0;
        cx = U32(obj, 0x44);
        cy = U32(obj, 0x48);
        cz = U32(obj, 0x4c);
        U32(obj, w->out) = U32(obj, 0x38);
        U32(obj, w->out + 4) = U32(obj, 0x3c);
        U32(obj, w->out + 8) = U32(obj, 0x40);
        U32(obj, w->out + 0xc) = cx;
        U32(obj, w->out + 0x10) = cy;
        U32(obj, w->out + 0x14) = cz;
        *ox = cx;
        *oy = cy;
        *oz = cz;
        return left != W19_MARKER;
    }
    if ((U16(obj, 0) & w->flag) != 0) {
        minx = miny = minz = W19_MARKER;
        maxx = maxy = maxz = 0xff7fc99eu;
    } else {
        minx = maxx = U32(obj, 0x44);
        miny = maxy = U32(obj, 0x48);
        minz = maxz = U32(obj, 0x4c);
    }
    for (child = U32(node, 4); child != 0; child = U32(child, 8)) {
        if (U32(child, 0x8c) == 0)
            continue;
        if (W19_Node(w, depth + 1, BYTES(U32(child, 0x8c)), &cx, &cy, &cz) == 0)
            continue;
        if (!(Float_OfBits(cx) >= Float_OfBits(minx)))
            minx = cx;
        if (Float_OfBits(cx) > Float_OfBits(maxx))
            maxx = cx;
        if (!(Float_OfBits(cy) >= Float_OfBits(miny)))
            miny = cy;
        if (Float_OfBits(cy) > Float_OfBits(maxy))
            maxy = cy;
        if (!(Float_OfBits(cz) >= Float_OfBits(minz)))
            minz = cz;
        if (Float_OfBits(cz) > Float_OfBits(maxz))
            maxz = cz;
    }
    if (depth < W19_DEPTHS)
        w->slot[depth] = minx;
    FL(&cx, 0) = (float)(((double)Float_OfBits(minx) + Float_OfBits(maxx)) * GFL(0x1012b8e4));
    FL(&cy, 0) = (float)(((double)Float_OfBits(miny) + Float_OfBits(maxy)) * GFL(0x1012b8e4));
    FL(&cz, 0) = (float)(((double)Float_OfBits(minz) + Float_OfBits(maxz)) * GFL(0x1012b8e4));
    FL(obj, w->out) = (float)(((double)Float_OfBits(cx) - FL(obj, 0x44)) + FL(obj, 0x38));
    FL(obj, w->out + 4) = (float)(((double)Float_OfBits(cy) - FL(obj, 0x48)) + FL(obj, 0x3c));
    FL(obj, w->out + 8) = (float)(((double)Float_OfBits(cz) - FL(obj, 0x4c)) + FL(obj, 0x40));
    U32(obj, w->out + 0xc) = cx;
    U32(obj, w->out + 0x10) = cy;
    U32(obj, w->out + 0x14) = cz;
    *ox = cx;
    *oy = cy;
    *oz = cz;
    return minx != W19_MARKER;
}

static int W19_Walk(uint8_t *obj, uint32_t *x, uint32_t *y, uint32_t *z, uint32_t flag, uint32_t out)
{
    W19Walk w;
    int i;

    w.flag = flag;
    w.out = out;
    for (i = 0; i < W19_DEPTHS; i++)
        w.slot[i] = 0;      /* "not the marker" until a non-leaf call at that depth sets it */
    return W19_Node(&w, 0, obj, x, y, z);
}

// FUNCTION: HGSIM 0x10089066
int __cdecl Sim_10089066(uint8_t *obj, uint32_t *x, uint32_t *y, uint32_t *z)
{
    return W19_Walk(obj, x, y, z, 0x1000, 0x70);
}

// FUNCTION: HGSIM 0x1008950e
int __cdecl Sim_1008950e(uint8_t *obj, uint32_t *x, uint32_t *y, uint32_t *z)
{
    return W19_Walk(obj, x, y, z, 0x800, 0x54);
}
