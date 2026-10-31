#include "../fx/w32.h"
#include <string.h>
#include "w8u.h"
#include "w2i.h"
#include "w3o.h"
#include "w7l.h"
#include "../unsorted_1000/w3o.h"
#include "../unsorted_1000/w3s.h"
#include "../unsorted_1003/w2b.h"
#include "../unsorted_1008/w7o.h"
#include "../unsorted_100a/w5b.h"
#include "../unsorted_100a/w5c.h"
#include "../unsorted_100f/w2e.h"
#include "../unsorted_100f/w6h.h"
#include "../view/view.h"
#include "../view/w3r.h"
#include "../util/util.h"
#include "../util/w3o.h"

// FUNCTION: HGSIM 0x10029067
void __cdecl Sim_10029067(void)
{
    int i;

    for (i = 0; i < 0x80; i++)
        Sim_10029421(i);
    for (i = 0; i < 0x80; i++) {
        uint8_t *e = GBYTES(0x109f09e0 + 0x14 * i);

        if (U32(e, 0) == 1 && (int32_t)(G32(0x1015dc78) - U32(e, 8)) > 0x712 && G32(0x10179cd4) == 0) {
            Sim_10028fb0((void *)(uintptr_t)U32(e, 4), (void(__cdecl *)(void *))(uintptr_t)U32(e, 0x10));
            memcpy(e, GBYTES(0x1015dc48), 0x14);
        }
    }
}

/* fld / fchs / fstp of a float: only the sign flips (the game never makes signalling NaNs). */
static uint32_t W8u_NegBits(uint32_t u)
{
    return u ^ 0x80000000u;
}

// FUNCTION: HGSIM 0x100291e3
void __cdecl Sim_100291e3(void *obj)
{
    /* five 3x4 matrices of the original frame: to the pivot (translation = -pivot position), the rotation, the
     * object's position, a work matrix and back from the pivot (translation = +pivot position) */
    uint32_t pivotM[12], rot[12], posM[12], work[12], back[12];
    uint8_t *pivot;
    uint8_t *child;

    if ((U32(obj, 0x94) & 1) == 0)
        return;
    U32(obj, 0x94) = U32(obj, 0x94) & ~1u;
    Sim_100a9c25(obj);
    Mat3x4_SetIdentity(pivotM);
    Mat3x4_SetIdentity(rot);
    Mat3x4_SetIdentity(posM);
    Mat3x4_SetIdentity(work);
    Mat3x4_SetIdentity(back);
    pivot = BYTES(U32(obj, 0x8c));
    if (pivot != 0) {
        pivotM[9] = W8u_NegBits(U32(pivot, 0x38));
        pivotM[10] = W8u_NegBits(U32(pivot, 0x3c));
        pivotM[11] = W8u_NegBits(U32(pivot, 0x40));
        Mem_Move(rot, BYTES(obj) + 0xc, 0x30);
        rot[9] = 0;
        rot[10] = 0;
        rot[11] = 0;
        back[9] = U32(pivot, 0x38);
        back[10] = U32(pivot, 0x3c);
        back[11] = U32(pivot, 0x40);
        posM[9] = U32(obj, 0x30);
        posM[10] = U32(obj, 0x34);
        posM[11] = U32(obj, 0x38);
    }
    Mat3x4_Multiply((const float *)rot, (const float *)pivotM, (float *)work);
    Mat3x4_Multiply((const float *)back, (const float *)work, (float *)work);
    Mat3x4_Multiply((const float *)posM, (const float *)work, (float *)(BYTES(obj) + 0xc));
    Mem_Move(BYTES(obj) + 0x4c, BYTES(obj) + 0xc, 0x30);
    U32(obj, 0x94) = U32(obj, 0x94) | 0x8000;
    if (pivot != 0) {
        Sim_10009a53(pivot, (const float *)(BYTES(obj) + 0x4c));
        U32(pivot, 0x28) = (uint32_t)(uintptr_t)obj;
    }
    for (child = BYTES(U32(obj, 4)); child != 0; child = BYTES(U32(child, 8)))
        Sim_100291e3(child);
    if (G32(0x1017c8c4) != 0 && U32(obj, 0) == 0)
        Sim_10088a2d((uint16_t *)pivot);
}

/* Float at a byte offset / its bits. */
#define W8U_F(p, off) FL(p, off)

// FUNCTION: HGSIM 0x10029421
void __cdecl Sim_10029421(int index)
{
    uint8_t *e = GBYTES(0x109eeb00 + 0x30 * index);
    uint8_t *model;
    uint32_t x, y, z;           /* [ebp-0x28], [ebp-0x30], [ebp-0x3c] */
    float radius, ground, vy, dv, newY;
    double ticks, sum;
    int bounced = 0, fast;
    float mx, my, mz, rx, ry, rz;
    uint32_t ox, oy, oz, nx, ny, nz;  /* 1.2: [ebp-0x38], [ebp-0x40], [ebp-0x44]; the node after the move */

    if (U32(e, 4) == 0)
        return;
    model = BYTES(Sim_100a9b71((void *)(uintptr_t)U32(e, 4)));
    radius = Sim_1000946e(model, &x, &y, &z);
    x = U32(e, 0x24);
    ox = x;
    y = U32(e, 0x28);
    oy = y;
    z = U32(e, 0x2c);
    oz = z;
    y = Bits_OfFloat((float)((double)Float_OfBits(y) - (double)radius / (double)GFL(0x10111290)));
    vy = W8U_F(e, 0xc);
    fast = (double)vy > (double)GFL(0x10111294);
    ticks = (double)GS32(0x1015dc80);
    {
        double g = ticks * (double)W8U_F(e, 0x20);

        dv = (float)g;
        newY = (float)(((g * (double)GFL(0x1011128c) + (double)vy) * ticks) + (double)Float_OfBits(y));
    }
    sum = (double)vy + (double)dv;
    vy = (float)sum;
    if (!(sum >= (double)GFL(0x10111294))) {
        ground = (float)Sim_1002d961(Float_OfBits(x), Float_OfBits(y), Float_OfBits(z));
        if (!((double)newY > (double)ground)) {
            if (fast) {
                uint8_t *o = BYTES(U32(e, 4));
                const char *name = (const char *)(uintptr_t)U32(o, 0x90);

                if (name != 0 && (Crt_StrStr(name, SIM_PTR(const char, 0x101352f4)) != 0 ||          /* "WRF" */
                                  Crt_StrStr(name, SIM_PTR(const char, 0x101352f8)) != 0))           /* "WBZ" */
                    Sim_10038d65((int)(uintptr_t)o);
                else
                    W8U_CALL_10029b1f(o);
                bounced = 1;
            }
            if ((double)vy > (double)GFL(0x10111298))
                bounced = 1;
            newY = ground;
            vy = (float)-((double)vy / (double)GFL(0x10111288));
            if (Sim_1009c929(100) < 0x32) {
                vy = (float)((double)vy / (double)GFL(0x10111290));
                W8U_F(e, 8) = (float)-((double)W8U_F(e, 8) / (double)GFL(0x10111290));
                W8U_F(e, 0x10) = (float)-((double)W8U_F(e, 0x10) / (double)GFL(0x10111290));
            }
            W8U_F(e, 0x14) = (float)-((double)W8U_F(e, 0x14) / (double)GFL(0x10111290));
            W8U_F(e, 0x18) = (float)-((double)W8U_F(e, 0x18) / (double)GFL(0x10111290));
            W8U_F(e, 0x1c) = (float)-((double)W8U_F(e, 0x1c) / (double)GFL(0x10111290));
        }
    }
    W8U_F(e, 0xc) = vy;
    mx = (float)(ticks * (double)W8U_F(e, 8));
    my = (float)((double)newY - (double)Float_OfBits(y));
    mz = (float)(ticks * (double)W8U_F(e, 0x10));
    rx = (float)(ticks * (double)W8U_F(e, 0x14));
    ry = (float)(ticks * (double)W8U_F(e, 0x18));
    rz = (float)(ticks * (double)W8U_F(e, 0x1c));
    W8U_F(e, 0x24) = (float)((double)W8U_F(e, 0x24) + (double)mx);
    W8U_F(e, 0x28) = (float)((double)W8U_F(e, 0x28) + (double)my);
    W8U_F(e, 0x2c) = (float)((double)W8U_F(e, 0x2c) + (double)mz);
    Sim_100a9d47((void *)(uintptr_t)U32(e, 4), U32(e, 0x24),
                 Bits_OfFloat((float)((double)W8U_F(e, 0x28) - (double)W8U_F(model, 0x3c))), U32(e, 0x2c));
    Sim_100aa127((void *)(uintptr_t)U32(e, 4), Bits_OfFloat(rx), Bits_OfFloat(ry), Bits_OfFloat(rz), 0);
    Sim_100291e3((void *)(uintptr_t)U32(e, 4));
    if (bounced) {
        G32(0x1015dc40) = G32(0x1015dc40) - 1;
        Sim_1002990f(index);
    } else if (W32_PARTICLES()) {
        /* 1.2 (0x1001ef21, round 32): smoke along the piece's move this frame */
        Sim_100a9cfb((void *)(uintptr_t)U32(e, 4), &nx, &ny, &nz);
        Sim12_1007f8a9(Float_OfBits(ox), Float_OfBits(oy), Float_OfBits(oz),
#ifdef W32D2_MUTATE /* the checker's negative check only */
                       Float_OfBits(nx),
#else
                       (float)((double)Float_OfBits(nx) - (double)Float_OfBits(ox)),
#endif
                       (float)((double)Float_OfBits(ny) - (double)Float_OfBits(oy)),
                       (float)((double)Float_OfBits(nz) - (double)Float_OfBits(oz)));
    }
}
