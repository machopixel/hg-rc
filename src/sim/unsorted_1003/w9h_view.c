#include "w9h.h"
#include "../render/w2n.h"
#include "../util/access.h"
#include "../util/util.h"
#include "../unsorted_1000/w2i.h"
#include "../unsorted_100a/w7o.h"
#include "../unsorted_100d/fns.h"
#include "../unsorted_100d/w5c.h"

/* Unconverted lighting helpers: the vertex shader writes +0x3c of every vertex in the list, the other answers a level. */
typedef void(__cdecl *W9hShadeFn)(uint8_t *obj, uint32_t b, uint32_t *list, uint32_t level);
typedef int32_t(__cdecl *W9hLightFn)(uint8_t *obj, uint32_t b, uint32_t c, uint32_t d);
/* Camera functions patched by hooks_view.c (and one not converted): called at their original addresses. */
typedef void(__cdecl *W9hCameraFn)(void *camera);
/* The display driver's polygon routine stored in dword 0x1014cb4c: (vertex count, 0x24-byte vertices, flags). */
typedef void(__cdecl *W9hDrawFn)(uint32_t count, void *vertices, uint32_t flags);

#define W9H_FLAGS(obj) ((uint32_t)U16(U32((obj), 0x24), 2))
#define W9H_LEVEL(obj) ((int32_t)(U16(U32((obj), 0x24), 0) & 0xf0) >> 4)

// FUNCTION: HGSIM 0x10032a10
uint32_t __cdecl Sim_10032a10(uint8_t *obj, uint32_t b, uint32_t colour, uint32_t d)
{
    uint32_t hi = 0, lo = 0, mid = 0;
    int32_t shade = 0;
    uint32_t kind = colour & 0xf000;
    uint32_t flags = W9H_FLAGS(obj);
    uint32_t *list = SIM_PTR(uint32_t, 0x101c76a0);
    int32_t level, i, t;

    if (G32(0x1014cb1c) != 0) {
        if (G32(0x1014cb20) == 1) {
            switch (U32(U32(obj, 0x24), 0x34)) {
            case 0:
                return 0xa;
            case 1:
                return 0x1f;
            case 2:
                return 0x14;
            case 3:
                return 0x18;
            case 5:
                return 1;
            case 6:
                return 0x17;
            case 7:
                return 0xf;
            default:
                return 0xfe;
            }
        }
        if (G32(0x1014cb20) == 2) {
            uint32_t v = U16(U32(obj, 0x24), 0) & 0x10f;

            if ((int32_t)v > 0x100) {
                if (v == 0x101)
                    return 0x17;
                if (v == 0x102)
                    return 7;
                if (v == 0x104)
                    return 0x1f;
                return 0xfe;
            }
            if (v == 0x100)
                return 0xf;
            switch (v) {
            case 0:
                return 0xa;
            case 1:
                return 0x14;
            case 2:
                return 1;
            case 4:
                return 0x1c;
            default:
                return 0xfe;
            }
        }
        if (G32(0x1014cb1c) == 1 && kind != 0x3000) {
            if (kind == 0x8000 || kind == 0x9000)
                return colour;
            if ((flags & 0x200) != 0)
                return 0x17;
            if ((flags & 0x400) != 0)
                return (kind == 0x1000 || kind == 0 || kind == 0x4000) ? 7 : colour;
            if ((flags & 0x100) != 0 || (flags & 0x50) != 0) {
                level = W9H_LEVEL(obj);
                if (level < 1)
                    return 0x17;
                return level < 8 ? 0x1f : 4;
            }
            if ((flags & 0x20) == 0 && (flags & 0x60) == 0 && (flags & 0x10) == 0)
                return 1;
        } else if (G32(0x1014cb1c) == 3) {
            if ((flags & 0x200) != 0)
                return 0x17;
            if ((flags & 0x400) != 0)
                return 7;
            if ((flags & 0x100) != 0) {
                level = W9H_LEVEL(obj);
                colour = level < 1 ? 0x10 : level < 8 ? 0x1f : 4;
                shade = SIM_FN(W9hLightFn, 0x100591ff)(obj, b, 0xff, d);
                colour += (uint32_t)(((int32_t)((uint32_t)shade * 48u) / 15) >> 4);
                return colour | 0x1000;
            }
            return 1;
        } else if (G32(0x1014cb1c) == 4) {
            if ((flags & 0x200) != 0)
                return 0x17;
            if ((flags & 0x400) != 0)
                return 7;
            if ((flags & 0x100) != 0) {
                level = W9H_LEVEL(obj);
                if (level == 0)
                    colour = 0x10;
                else if (G32(0x10179ca4) != 0 && Sim_1009c929(100) > 0x14)
                    colour = 0x10;
                else
                    colour = level < 8 ? 0x1f : 4;
                return 0x2000 | colour;
            }
            return 1;
        }
    }
    switch (kind) {
    case 0:
        lo = (colour & 0xff) >> 4;
        if (G32(0x1014cb28) != 0) {
            lo >>= 2;
            mid = 0xf0;
        } else {
            mid = (colour & 0xf00) >> 4;
        }
        return mid | lo;
    case 0x8000:
    case 0x9000:
        return colour;
    case 0x3000:
        if (G32(0x1014cb38) == 0)
            return colour;
        if ((G32(0x1014cb38) & flags) == 0)
            return colour;
        kind = 0x1000;
        lo = colour & 0xf0;
        break;
    case 0x2000:
    case 0x5000:
    case 0x6000:
    case 0x7000:
    case 0xb000:
    case 0xc000:
    case 0xd000:
    case 0xe000:
        lo = 0xff;
        if (G32(0x1014cb38) != 0)
            kind = 0x1000;
        else
            hi = colour & 0xff;
        break;
    default:
        lo = colour & 0xff;
        if ((flags & 0x400) != 0)
            return colour;
        break;
    }
    if ((flags & 0x400) != 0) {
        shade = 0xf;
    } else {
        SIM_FN(W9hShadeFn, 0x10058c5e)(obj, b, list, lo);
        shade = Render_Ftol((double)S32(list[0], 0x3c) / 31.0 * 7.0 + 0.5);
    }
    if (G32(0x1014cb1c) != 3 && ((flags & 0x100) != 0 || (flags & 0xf0) == 0x50)) {
        level = W9H_LEVEL(obj);
        if (level > 0) {
            if (G32(0x1015dd70) != 0) {
                shade += Render_Ftol((double)level * ((double)(int32_t)(15u - (uint32_t)shade) / 15.0));
                for (i = 0; i < GS32(0x101c9ea8); i++) {
                    uint32_t v = list[i];

                    S32(v, 0x3c) += Render_Ftol((double)level * ((double)(int32_t)(15u - U32(v, 0x3c)) / 15.0));
                }
            } else {
                t = Render_Ftol((double)(16 - level) / 16.0);
                shade = Render_Ftol((double)t * (double)shade) & 0xf;
                for (i = 0; i < GS32(0x101c9ea8); i++) {
                    uint32_t v = list[i];

                    S32(v, 0x3c) = Render_Ftol((double)t * (double)S32(v, 0x3c)) & 0xf;
                }
            }
        }
    }
    switch (kind) {
    case 0x2000:
    case 0x5000:
    case 0x6000:
    case 0x7000:
    case 0xb000:
    case 0xc000:
    case 0xd000:
    case 0xe000:
        shade = (int32_t)((uint32_t)shade << 8);
        mid = 0;
        break;
    default:
        mid = G32(0x1014cb28) != 0 ? 0xf0 : (colour & 0xf00) >> 4;
        break;
    }
    return kind | hi | mid | (uint32_t)shade;
}

// FUNCTION: HGSIM 0x100352d5
void __cdecl Sim_100352d5(void)
{
    uint8_t quad[4 * 0x24];

    if (G32(0x1017b940) == 0)
        return;
    if (G32(0x1013a014) != 0) {
        SIM_FN(W9hCameraFn, 0x100d2306)(BYTES(G32(0x1014cae0)));
        G32(0x1013a014) = 0;
    }
    Sim_100d255e(BYTES(G32(0x1014cae0)));
    SIM_FN(W9hCameraFn, 0x100d1f90)(BYTES(G32(0x1014cae0)));
    if (G32(0x1014cb18) != 0 || G32(0x1014cb1c) != 0 || G32(0x1015d1ac) == 4) {
        /* (0,0) (639,0) (639,479) (0,479) as floats in x, y and again in u, v (+0x1c, +0x20); +0x10 = 7,
           +0x18 = 0xff000000; the other fields are left unset like the original does */
        U32(quad, 0x00) = 0;
        U32(quad, 0x1c) = 0;
        U32(quad, 0x04) = 0;
        U32(quad, 0x20) = 0;
        U32(quad, 0x24) = 0x441fc000;
        U32(quad, 0x40) = 0x441fc000;
        U32(quad, 0x28) = 0;
        U32(quad, 0x44) = 0;
        U32(quad, 0x48) = 0x441fc000;
        U32(quad, 0x64) = 0x441fc000;
        U32(quad, 0x4c) = 0x43ef8000;
        U32(quad, 0x68) = 0x43ef8000;
        U32(quad, 0x6c) = 0;
        U32(quad, 0x88) = 0;
        U32(quad, 0x70) = 0x43ef8000;
        U32(quad, 0x8c) = 0x43ef8000;
        U32(quad, 0x10) = 7;
        U32(quad, 0x18) = 0xff000000;
        U32(quad, 0x34) = 7;
        U32(quad, 0x3c) = 0xff000000;
        U32(quad, 0x58) = 7;
        U32(quad, 0x60) = 0xff000000;
        U32(quad, 0x7c) = 7;
        U32(quad, 0x84) = 0xff000000;
        (*SIM_PTR(W9hDrawFn, 0x1014cb4c))(4, quad, G32(0x10138c70));
    } else if (G32(0x1014cb04) != 0 || G32(0x1014cb08) != 0) {
        SIM_FN(W9hCameraFn, 0x100a0def)(BYTES(G32(0x1014cae0)));
    }
    Sim_100a4c45();
    Sim_100071f6();
    Sim_100d32e5(0);
}
