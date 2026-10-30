#include "w10.h"
#include "../simmem.h"
#include "../util/access.h"
#include "../util/util.h"
#include "w3o.h"
#include "w5z.h"
#include "w7l.h"
#include "../render/w2n.h"
#include "../unsorted_1000/w5x.h"
#include "../unsorted_1000/w2l.h"
#include "../unsorted_1004/fns.h"
#include "../unsorted_1004/w2k.h"
#include "../unsorted_1005/w2g.h"
#include "../unsorted_1005/w2h.h"
#include "../unsorted_1005/w2j.h"
#include "../unsorted_1008/w3o.h"
#include "../unsorted_100a/w5b.h"
#include "../unsorted_100a/w5y.h"
#include "../unsorted_100a/w5z.h"
#include "../unsorted_100a/w6g.h"
#include "../unsorted_100c/w8z.h"
#include "../unsorted_100d/w5z.h"
#include "../unsorted_100d/w6h.h"

#define W10_2E_F(addr) (*SIM_PTR(float, addr))
#define W10_2E_FX(code) (0x10147b80u + (uint32_t)(code) * 0x3cu)
#define W10_2E_SLOT(i) (0x109a5a40u + (uint32_t)(i) * 0x2cu)

/* the air-burst rule of the switch: ground hits (flag 0x100) drop by [0x10111260], unless it is the network flag 0x4000 */
#define W10_2E_LOW(hi) (((hi) & 0x100) != 0 && !(((hi) & 0x4000) != 0 && G32(0x1015ef0c) != 0))

static inline float W10_2E_Bits(uint32_t u)
{
    union {
        uint32_t u;
        float f;
    } v;

    v.u = u;
    return v.f;
}

// FUNCTION: HGSIM 0x100246b3
void __cdecl Sim_100246b3(uint32_t owner, uint8_t *node, int32_t code, float x, float y, float z, uint32_t tx, uint32_t ty,
                          uint32_t tz, uint32_t ra, uint32_t rb, uint32_t rc)
{
    uint32_t light[14];       /* the original's 0x38-byte light record at ebp-0xc4 (Sim_100585e1 reads +0..+0xc, +0x2c, +0x34) */
    uint32_t shade[5];        /* the original's 0x14-byte record at ebp-0x80 (Sim_100ac620 reads all of it) */
    float a, b, c, d, dxc, dyc, dzc, d1, d2, cx, cz;
    uint32_t hi, quiet = 0, fx, e = 0, cam, part;
    int lit = 0, burst = 0, count, i, near, far;
    int32_t freeslot, burst_kind = -1, li, s, size;
    uint8_t *me;

    if (code < 0)
        return;
    hi = (uint32_t)code & 0xff00;
    code &= 0xff;
    if (code >= 0x4d)
        return;
    if ((hi & 0x4000) != 0)
        quiet = 1;
    if (G32(0x1015ef3c) != 0) {
        /* the own unit feels a near blast (all three offsets from its +0x58, as the original) */
        me = BYTES(U32(PTRTAB(G32(0x1017b8c8)), 0x28));
        part = U32(me, 0);
        if (!(Sim_10084a94((float)((double)x - W10_2E_Bits(U32(part, 0x58))), (float)((double)y - W10_2E_Bits(U32(part, 0x58))),
                           (float)((double)z - W10_2E_Bits(U32(part, 0x58)))) >= (double)W10_2E_F(0x10111234))) {
            Sim_100cbc6b((float)((double)x - W10_2E_Bits(U32(part, 0x58))), (float)((double)y - W10_2E_Bits(U32(part, 0x5c))),
                         (float)((double)z - W10_2E_Bits(U32(part, 0x60))), &a, &b, &c, &d);
            Sim_10045f3b(0, (uint32_t)Render_Ftol((double)a), 0x32);
        }
    }
    switch (code) {
    case 1:
    case 2:
    case 6:
        if ((hi & 0x100) != 0)
            code = 0x4b;
        lit = 1;
        break;
    case 4:
    case 57:
    case 60:
        if ((hi & 0x100) != 0)
            code = 0x4a;
        break;
    case 62:
        if (W10_2E_LOW(hi)) {
            y = (float)((double)y - W10_2E_F(0x10111260));
            code = 0x2f;
        } else if ((hi & 0x400) != 0) {
            code = 0x34;
            lit = 1;
        }
        break;
    case 68:
        if (W10_2E_LOW(hi)) {
            y = (float)((double)y - W10_2E_F(0x10111260));
            code = 0x2a;
        } else if ((hi & 0x400) != 0) {
            code = 0x2a;
        }
        lit = 1;
        break;
    case 69:
    case 61:
    case 64:
    case 65:
    case 66:
        if (W10_2E_LOW(hi)) {
            y = (float)((double)y - W10_2E_F(0x10111260));
            code = 0x2a;
        } else if ((hi & 0x400) != 0) {
            code = 0x2c;
        }
        lit = 1;
        break;
    case 59:
    case 63:
        if (W10_2E_LOW(hi)) {
            y = (float)((double)y - W10_2E_F(0x10111260));
            code = 0x32;
        } else if ((hi & 0x400) != 0) {
            code = 0x32;
            lit = 1;
        }
        break;
    case 67:
    case 70:
    case 71:
    case 72:
        if (W10_2E_LOW(hi)) {
            y = (float)((double)y - W10_2E_F(0x10111260));
            code = 0x2a;
        } else if ((hi & 0x400) != 0) {
            code = 0x2a;
            lit = 1;
        }
        break;
    case 5:
        if (W10_2E_LOW(hi)) {
            y = (float)((double)y - W10_2E_F(0x10111260));
            code = 0x2a;
        } else if ((hi & 0x400) != 0) {
            code = 9;
        } else if ((hi & 0x2000) != 0) {
            code = 0x12;
        }
        break;
    case 36:
        burst = 1;
        burst_kind = 0x10b;
        lit = 1;
        break;
    case 11:
    case 37:
    case 38:
        size = (hi & 0x1000) != 0 ? 0x10 : (hi & 0x100) != 0 ? 4 : (hi & 0x200) != 0 ? 4 : 8;
        Sim_10025c8c(*(uint32_t *)&x, *(uint32_t *)&y, *(uint32_t *)&z, size);
        return;
    default:
        break;
    }
    fx = W10_2E_FX(code);
    cam = G32(0x1014cae0);
    dxc = (float)((double)W10_2E_Bits(U32(cam, 0)) - x);
    dyc = (float)((double)W10_2E_Bits(U32(cam, 4)) - y);
    dzc = (float)((double)W10_2E_Bits(U32(cam, 8)) - z);
    if (U32(SIM_PTR(uint8_t, fx), 0x14) != 0 || U32(SIM_PTR(uint8_t, fx), 0x10) != 0) {
        /* a free effect slot of the code; at most two live ones nearby while [0x10152418] is 1 */
        count = 0;
        freeslot = -1;
        for (i = 0; i < 0x100; i++) {
            e = W10_2E_SLOT(i);
            if (S32(SIM_PTR(uint8_t, e), 0x24) != code)
                continue;
            if (U32(SIM_PTR(uint8_t, e), 0x18) != 0) {
                if ((S32(SIM_PTR(uint8_t, e), 0x24) >= 0x17 && S32(SIM_PTR(uint8_t, e), 0x24) <= 0x21 && code != 0x4c) ||
                    (U32(SIM_PTR(uint8_t, fx), 0x28) & 2) != 0)
                    continue;
                if (!(Sim_10084a94((float)((double)W10_2E_Bits(U32(SIM_PTR(uint8_t, e), 8)) - x),
                                   (float)((double)W10_2E_Bits(U32(SIM_PTR(uint8_t, e), 0xc)) - y),
                                   (float)((double)W10_2E_Bits(U32(SIM_PTR(uint8_t, e), 0x10)) - z)) >=
                      (double)W10_2E_F(0x10111248))) {
                    count++;
                    if (!(G32(0x10152418) == 1 && count <= 2))
                        return;
                }
            } else if (freeslot < 0 && (U32(SIM_PTR(uint8_t, fx), 0x14) == 0 || U32(SIM_PTR(uint8_t, e), 0) != 0)) {
                freeslot = i;
            }
        }
        if (freeslot < 0)
            return;
        e = (uint32_t)(uintptr_t)SIM_PTR(uint8_t, W10_2E_SLOT(freeslot));
        U32(e, 0x18) = 1;
        *(float *)BYTES(e + 8) = x;
        *(float *)BYTES(e + 0xc) = y;
        *(float *)BYTES(e + 0x10) = z;
        U32(e, 0x14) = U32(SIM_PTR(uint8_t, fx), 0);
        U32(e, 4) = 0;
        U32(e, 0x28) = owner;
        if (lit) {
            li = Sim_10058575();
            if (li >= 0) {
                light[0] = 1;
                light[1] = *(uint32_t *)&x;
                light[2] = *(uint32_t *)&y;
                light[3] = *(uint32_t *)&z;
                light[0x2c / 4] = 0x469c4000;     /* 20000.0f */
                light[0x34 / 4] = U32(e, 0x14);
                Sim_100585e1(li, 0xf, light);
                Sim_1005892e(li, 1);
            }
            s = Sim_100ac21a(6, 0);
            if (s >= 0) {
                shade[0] = *(uint32_t *)&x;
                *(float *)&shade[1] = (float)Sim_1002d961(x, y, z);
                shade[2] = *(uint32_t *)&z;
                shade[4] = 0x459c4000;     /* 5000.0f */
                shade[3] = U32(e, 0x14);
                Sim_100ac620(s, 7, shade);
            }
        }
        if (U32(e, 0) != 0) {
            if ((U32(SIM_PTR(uint8_t, fx), 0x28) & 1) != 0) {
                uint32_t p;

                if (node != 0 && (U16(node, 2) & 0x400) == 0 && (U16(node, 2) & 0x100) != 0 &&
                    (p = Sim_100096ee(node)) != 0) {
                    U32(e, 4) = p;
                    Sim_100a9cfb((void *)(uintptr_t)p, (uint32_t *)&x, (uint32_t *)&y, (uint32_t *)&z);
                } else {
                    U32(e, 0x14) = G32(0x1015dc80) + 0xb5u;
                }
            }
            if (((code >= 0x17 && code <= 0x21) || code == 0x4c) && G32(0x1015dc08) != 0) {
                Sim_10009408((void *)(uintptr_t)U32(U32(e, 0), 0x8c), (unsigned short)0xff9f);
                Sim_100d7111((void *)(uintptr_t)G32(0x1015dc08), (void *)(uintptr_t)U32(e, 0));
                G32(0x1015dc08) = 0;
            } else {
                Sim_100aa0da((void *)(uintptr_t)U32(e, 0), ra, rb, rc, 0);
                Sim_100a9d47((void *)(uintptr_t)U32(e, 0), *(uint32_t *)&x, *(uint32_t *)&y, *(uint32_t *)&z);
            }
            Sim_100aa39f((void *)(uintptr_t)U32(e, 0));
            Sim_100aa406((void *)(uintptr_t)U32(e, 0));
            Sim_100aa799((void *)(uintptr_t)U32(e, 0));
            if (S32(e, 0x1c) != -1) {
                Sim_1000716a((int)(int16_t)U16(e, 0x1c), 0);
                Sim_1000711c((short)U16(e, 0x1c), U16(SIM_PTR(uint8_t, fx), 0x1c));
            }
        }
        if (U32(SIM_PTR(uint8_t, fx), 0x10) != 0 && G32(0x10134894) != 0) {
            /* a big one moves the camera to the given point (once; the old view saved at 0x109f14e0) */
            near = 0;
            far = 1;
            if (G32(0x1015dc04) != 0) {
                d1 = (float)((double)dxc * dxc + (double)dzc * dzc);
                cx = (float)((double)W10_2E_Bits(U32(cam, 0x1c)) - W10_2E_Bits(U32(cam, 0)));
                cz = (float)((double)W10_2E_Bits(U32(cam, 0x24)) - W10_2E_Bits(U32(cam, 8)));
                d2 = (float)((double)cx * cx + (double)cz * cz);
                if (!(d1 >= d2))
                    near = 1;
                else
                    far = 0;
            }
            if (far) {
                if (!near) {
                    G32(0x1015dc04) = 1;
                    G32(0x1013a018) = 0;
                    G32(0x109f14e0) = U32(G32(0x1014cae0), 0x1c);
                    G32(0x109f14e4) = U32(G32(0x1014cae0), 0x20);
                    G32(0x109f14e8) = U32(G32(0x1014cae0), 0x24);
                    W10_2E_F(0x109f14ec) = (float)S16(G32(0x1014cae0), 0x28);
                    G32(0x109f14f0) = (uint32_t)(int32_t)S16(G32(0x1014cae0), 0x2a);
                    U16(G32(0x1014cae0), 0x2a) = (uint16_t)(U16(G32(0x1014cae0), 0x2a) - 10);
                    if (S16(G32(0x1014cae0), 0x2a) > 0xff || S16(G32(0x1014cae0), 0x2a) < 0)
                        U16(G32(0x1014cae0), 0x2a) = 0x40;
                    G32(0x109f14f4) = G32(0x1014cb24);
                    U16(G32(0x1014cae0), 0x28) = 0;
                    G32(0x1014cb24) = 0;
                }
                G32(0x10134888) = (uint32_t)freeslot;
                U32(e, 0x20) = 1;
                U32(G32(0x1014cae0), 0x1c) = tx;
                U32(G32(0x1014cae0), 0x20) = ty;
                U32(G32(0x1014cae0), 0x24) = tz;
                if (S32(SIM_PTR(uint8_t, fx), 4) > -1)
                    Sim_100d36d6(U32(SIM_PTR(uint8_t, fx), 4), U32(SIM_PTR(uint8_t, fx), 0), 1);
            }
        }
    }
    if (burst)
        Sim_10024616(owner, (uint32_t)burst_kind, *(uint32_t *)&x, *(uint32_t *)&y, *(uint32_t *)&z, tx, ty, tz);
    if (S32(SIM_PTR(uint8_t, fx), 8) > 0 &&
        (S32(SIM_PTR(uint8_t, fx), 0xc) == -1 || Sim_1009c929(100) < S32(SIM_PTR(uint8_t, fx), 0xc)))
        Sim_1000ed52(*(uint32_t *)&dxc, *(uint32_t *)&dyc, *(uint32_t *)&dzc, (uint32_t)fx + 0x2cu,
                     (Sim_100448ff() == 0 && quiet == 0) ? 1u : 0u, 0);
}
