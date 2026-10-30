#include "w10.h"
#include "../simmem.h"
#include "../util/access.h"
#include "../util/util.h"
#include "../unsorted_1008/w8w.h"
#include "../unsorted_1009/w2f.h"
#include "../unsorted_100a/w5a.h"
#include "../unsorted_100a/w5z.h"
#include "w3s.h"
#include "w5a.h"
#include "w5b.h"
#include "w6e.h"
#include "w7m.h"
#include "../unsorted_100c/w8z.h"
#include "../unsorted_100f/w8z.h"

typedef void(__cdecl *W10_4Fn10087d83)(void *camera);

static uint32_t W10_4Bits(float f)
{
    union {
        float f;
        uint32_t u;
    } v;

    v.f = f;
    return v.u;
}

// FUNCTION: HGSIM 0x100456fc
void __cdecl Sim_100456fc(void)
{
    uint32_t i = 0, cell, entry, node, kind;
    float x, y, z;
    uint32_t a, angle;

    SIM_FN(W10_4Fn10087d83, 0x10087d83)((void *)(uintptr_t)G32(0x1014cae0));
    for (cell = SIM_PTR(u32a, 0x1036f2a0)[i]; cell != 0; cell = SIM_PTR(u32a, 0x1036f2a0)[i]) {
        i++;
        for (entry = U32(cell, 0x20); entry != 0; entry = U32(entry, 0x20)) {
            kind = U16(entry, 2) & 0xf0u;
            if (kind != 0x10 && kind != 0x60 && kind != 0x20)
                continue;
            if (U16(entry, 0) & 0x1000u)
                continue;
            node = U32(entry, 0x28);
            if (node == 0)
                continue;
            Sim_100a9cfb((void *)(uintptr_t)node, (uint32_t *)&x, (uint32_t *)&y, (uint32_t *)&z);
            if (Sim_1009b2eb() != 0) {
                angle = 0x43340000u;       /* 180.0f */
                a = 0xc2340000u;           /* -45.0f */
            } else {
                const float *cam = (const float *)(uintptr_t)G32(0x1014cae0);

                angle = W10_4Bits(Sim_1008486e(cam[0] - x, cam[2] - z));
                a = 0;
            }
            Sim_100aa0da((void *)(uintptr_t)node, a, angle, 0, 0);
            Sim_100aa742((void *)(uintptr_t)node);
        }
    }
}


typedef double(__cdecl *W10_4FmodFn)(double x, double y);

#define W10_4F(addr) (*SIM_PTR(float, addr))
#define W10_4CAM(i) (((float *)(uintptr_t)G32(0x1014cae0))[i])
/* the original's fcomp tests: "below" holds for an unordered compare, "equal" too */
#define W10_4BELOW(a, b) (!((a) >= (b)))
#define W10_4EQUAL(a, b) (!((a) < (b) || (a) > (b)))

// FUNCTION: HGSIM 0x10044d0b
void __cdecl Sim_10044d0b(float speed, float lift, float yaw, float turn)
{
    uint8_t *rec = (uint8_t *)(uintptr_t)U32(G32(0x109e8b40), 0x28);
    float tx, ty, tz, ta, tb, tc;       /* Sim_10044b2c's results: tx, ty, tz at ebp-0x54, -4, -0xc; ta, tb, tc at -0x2c, -0x38, -0x40 */
    float a1, a2, a3, a4;               /* Sim_100cbc6b's four angles (ebp-0x1c, -0x58, -0x44, -0x20) */
    float h, head, c, sn, fx, fz, rise, ground;
    double t, sum;

    Sim_10044b2c((uint32_t *)&ta, (uint32_t *)&tb, (uint32_t *)&tc, (uint32_t *)&tx, (uint32_t *)&ty,
                 (uint32_t *)&tz);
    if (G32(0x1013aed4) != 1) {
        if (W10_4EQUAL(W10_4F(0x1015eeec), W10_4F(0x10111584))) {
            /* float * float is exact in the 53-bit register; the division uses that unrounded product */
            t = (double)*(float *)(rec + 0x53c) * (double)W10_4F(0x10111588);
            W10_4F(0x1015eeec) = (float)t;
            W10_4F(0x1015eef0) = (float)(t / (double)W10_4F(0x1011158c));
            W10_4F(0x1015eef4) = (float)((double)W10_4F(0x1015eeec) * (double)W10_4F(0x10111590));
            W10_4F(0x1015eef8) = (float)((double)W10_4F(0x1015eeec) / (double)W10_4F(0x10111594));
            G32(0x109b9a84) = G32(0x1015eef4);
            W10_4F(0x109b9ad4) = -*(float *)(rec + 0x520);
            W10_4F(0x109b9b8c) = (float)(-(double)*(float *)(rec + 0x520) + (double)W10_4F(0x10111598));
        }
        G32(0x109b9bb0) = G32(0x1015dc78);
        G32(0x109b9af0) = G32(0x1015dc78);
        G32(0x109b9a90) = G32(0x1015dc78);
        G32(0x109b9ac0) = G32(0x1015dc78);
        G32(0x109b9aa0) = G32(0x1015dc78);
        W10_4F(0x109b9bb8) = (float)((double)W10_4CAM(0) - (double)tx);
        W10_4F(0x109b9af8) = (float)((double)W10_4CAM(1) - (double)ty);
        W10_4F(0x109b9a98) = (float)((double)W10_4CAM(2) - (double)tz);
        G32(0x109b9ac8) = U32(G32(0x1014cae0), 0xc);
        G32(0x109b9aa8) = U32(G32(0x1014cae0), 0x10);
        U32(G32(0x1014cae0), 0x14) = 0;
        if (W10_4BELOW(Crt_Fabs((double)W10_4CAM(0) - (double)tx), (double)W10_4F(0x10111570)))
            W10_4CAM(0) = (float)((double)W10_4CAM(0) + (double)W10_4F(0x10111580));
        G32(0x1013aed0) = 0x3f800000;
        Sim_10044909(0);
    }
    sum = (double)W10_4F(0x1015eeec) + (double)speed;
    W10_4F(0x1015eeec) = (float)sum;
    if (sum > (double)W10_4F(0x1015eef4))
        G32(0x1015eeec) = G32(0x1015eef4);
    else if (W10_4BELOW(W10_4F(0x1015eeec), W10_4F(0x1015eef0)))
        G32(0x1015eeec) = G32(0x1015eef0);
    h = (float)((double)W10_4F(0x1015eef8) + (double)lift);
    W10_4F(0x1013aec8) = (float)((double)W10_4F(0x1013aec8) + (double)turn);
    W10_4F(0x1013aec8) = (float)SIM_FN(W10_4FmodFn, 0x100fcb30)((double)W10_4F(0x1013aec8), 360.0);
    Sim_100cbc6b((float)((double)tx - (double)W10_4CAM(0)), (float)((double)ty - (double)W10_4CAM(1)),
                 (float)((double)tz - (double)W10_4CAM(2)), &a1, &a2, &a3, &a4);
    Sim_10047e1a(SIM_PTR(void, 0x109b9aa0), (float)(-(double)a4 - (double)yaw / (double)W10_4F(0x1011156c)));
    W10_4CAM(4) = Sim_10047cf2(SIM_PTR(uint32_t, 0x109b9aa0));
    Sim_10047e1a(SIM_PTR(void, 0x109b9ac0), a1);
    W10_4CAM(3) = Sim_10047cf2(SIM_PTR(uint32_t, 0x109b9ac0));
    t = SIM_FN(W10_4FmodFn, 0x100fcb30)((double)(float)((double)tb - (double)W10_4F(0x1013aec8)), 360.0);
    head = (float)t;
    if (W10_4BELOW(t, (double)W10_4F(0x1011159c)))
        head = (float)((double)head + (double)W10_4F(0x101115a0));
    else if ((double)head > (double)W10_4F(0x101115a4))
        head = (float)((double)head - (double)W10_4F(0x101115a0));
    c = Sim_10084766((float)((double)head + (double)W10_4F(0x101115a8)));
    sn = Sim_10084766(head);
    fx = (float)((double)W10_4F(0x1015eeec) * (double)sn);
    fz = (float)((double)W10_4F(0x1015eeec) * (double)c);
    rise = h;
    W10_4F(0x109b9bb4) = fx;
    W10_4CAM(0) = (float)((double)Sim_10047bcd(SIM_PTR(uint32_t, 0x109b9bb0)) + (double)tx);
    W10_4F(0x109b9a94) = fz;
    W10_4CAM(2) = (float)((double)Sim_10047bcd(SIM_PTR(uint32_t, 0x109b9a90)) + (double)tz);
    ground = Sim_1004487c((const float *)(uintptr_t)G32(0x1014cae0));
    if (W10_4BELOW((double)ty + (double)rise, (double)ground))
        rise = (float)((double)ground - (double)ty);
    W10_4F(0x1015eef8) = h;
    if (W10_4F(0x1015eef8) > W10_4F(0x109b9a84))
        G32(0x1015eef8) = G32(0x109b9a84);
    else if ((U16(rec, 0x560) & 0x20) != 0 && W10_4BELOW(W10_4F(0x1015eef8), W10_4F(0x109b9b8c)))
        G32(0x1015eef8) = G32(0x109b9b8c);
    else if (W10_4BELOW(W10_4F(0x1015eef8), W10_4F(0x109b9ad4)))
        G32(0x1015eef8) = G32(0x109b9ad4);
    W10_4F(0x109b9af4) = rise;
    W10_4CAM(1) = (float)((double)Sim_10047bcd(SIM_PTR(uint32_t, 0x109b9af0)) + (double)ty);
}