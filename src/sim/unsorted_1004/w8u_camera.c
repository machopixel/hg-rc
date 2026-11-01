#include "w8u.h"
#include "fns.h"
#include "w2e.h"
#include "w3s.h"
#include "w5a.h"
#include "w5y.h"
#include "w6e.h"
#include "w7m.h"
#include "../unsorted_1008/w3o.h"
#include "../unsorted_1000/w5a.h"
#include "../unsorted_100a/w7o.h"
#include "../util/w3o.h"
#include "w3p.h"

#define W8U_CAM() BYTES(G32(0x1014cae0))
#define W8U_TICKS() ((double)GS32(0x1015dc80))

// FUNCTION: HGSIM 0x1004170e
void __cdecl Sim_1004170e(const uint8_t *pkt, int index)
{
    uint8_t *unit;
    double len;
    float vol;

#ifndef NETG_MUTATE /* the checker's negative check only: no guard */
    if (g_w34NetOver != 0)          /* 1.2 (0x100354eb): ignored once the game is over */
        return;
#endif

    if (GS32(0x1017b8c8) == index)
        return;
    if (PTRTAB(index) == 0)
        return;
    unit = BYTES(U32(PTRTAB(index), 0x28));
    GFL(0x1015dd58) = (float)((double)S32(pkt, 2) * (double)GFL(0x10111528));
    GFL(0x1015dd5c) = (float)((double)S32(pkt, 6) * (double)GFL(0x10111528));
    GFL(0x1015dd60) = (float)((double)S32(pkt, 0xa) * (double)GFL(0x10111528));
    FL(unit, 0x554) = (float)((double)S32(pkt, 0xe) * (double)GFL(0x10111528));
    FL(unit, 0x558) = (float)((double)S32(pkt, 0x12) * (double)GFL(0x10111528));
    FL(unit, 0x55c) = (float)((double)S32(pkt, 0x16) * (double)GFL(0x10111528));
    len = Sim_10084a94(FL(unit, 0x554), FL(unit, 0x558), FL(unit, 0x55c));
    vol = (float)len;
    if (len > (double)GFL(0x10111538))
        vol = 25.0f;
    vol = (float)((double)GFL(0x1011153c) * (double)vol / (double)GFL(0x10111540) / (double)GFL(0x10111544));
    Sim_1000ebfb(SIM_PTR(const char, 0x1013adf8), (uint32_t)Render_Ftol(vol), 0, 5, 0x32);   /* "AFXHGCOO" */
    W8U_CALL_1006788c(U32(PTRTAB(G32(0x1017b8c8)), 0x28), U32(PTRTAB(index), 0x28));
}

// FUNCTION: HGSIM 0x10043f1a
int __cdecl Sim_10043f1a(void)
{
    uint32_t a, b, c, d, e, f;      /* [ebp-0xc], [ebp-0x10], [ebp-0x14], [ebp-0x18], [ebp-4], [ebp-8] */
    float v;

    if (GS32(0x1015ed20) < 0 || GS32(0x1015ed20) >= GS32(0x1013aec4))
        G32(0x1015ee98) = 0;
    if (G32(0x1015ee98) == 0)
        return 0;
    G32(0x1015ed38) = G32(0x1015ed38) + G32(0x1015dc80);
    if (GS32(0x1015ed38) >= GS32(0x1015ed88 + 0x1c * G32(0x1015ed20))) {
        G32(0x1015ed38) = 0;
        G32(0x1015ed20) = G32(0x1015ed20) + 1;
        if (GS32(0x1015ed20) < GS32(0x1013aec4))
            Sim_1004409f((int)G32(0x1015ed20));
    }
    Sim_10045308((float *)&a, (float *)&b, &c, &d, &e, &f);
    v = Sim_10047bcd(SIM_PTR(uint32_t, 0x1015ed50));
    FL(W8U_CAM(), 0) = (float)((double)v + (double)Float_OfBits(d));
    v = Sim_10047bcd(SIM_PTR(uint32_t, 0x1015ee88));
    FL(W8U_CAM(), 4) = (float)((double)v + (double)Float_OfBits(e));
    v = Sim_10047bcd(SIM_PTR(uint32_t, 0x1015ed28));
    FL(W8U_CAM(), 8) = (float)((double)v + (double)Float_OfBits(f));
    v = Sim_10047bcd(SIM_PTR(uint32_t, 0x1015ed60));
    FL(W8U_CAM(), 0x10) = (float)((double)v + (double)Float_OfBits(a));
    v = Sim_10047bcd(SIM_PTR(uint32_t, 0x1015ed10));
    FL(W8U_CAM(), 0xc) = (float)((double)v + (double)Float_OfBits(b));
    v = Sim_10047bcd(SIM_PTR(uint32_t, 0x1015ed40));
    FL(W8U_CAM(), 0x14) = (float)((double)v + (double)Float_OfBits(c));
    return 1;
}

/* (ticks * [rate]) / [0x10111564] as a float, the rate unit of the camera modes. */
static float W8u_Rate(uint32_t rate)
{
    return (float)(W8U_TICKS() * (double)GFL(rate) / (double)GFL(0x10111564));
}

// FUNCTION: HGSIM 0x100445d6
void __cdecl Sim_100445d6(void)
{
    int mode;

    G32(0x1013aecc) = G32(0x1015fdb4);
    Sim_10044909(0);
    G32(0x1015ef08) = 0;
    G32(0x1015ef0c) = G32(0x1015ef08);
    if (G32(0x1015dc64) == 0) {
        mode = 2;
        Sim_100448cd(2);
        G32(0x1015ef14) = 0;
    } else {
        mode = Sim_100448ff();
        if (mode == 0)
            G32(0x1015ef14) = PTRTAB(G32(0x1017b8c8));
        if (G32(0x1015ef14) == 0)
            mode = 2;
    }
    G32(0x1014cae8) = 0;
    if (G32(0x1013aed4) == 1 && mode != 1) {
        U32(W8U_CAM(), 0x3c) = G32(0x1015eefc);
    } else if (G32(0x1013aed4) != 1 && mode == 1) {
        G32(0x1015eefc) = U32(W8U_CAM(), 0x3c);
        U32(W8U_CAM(), 0x3c) = 0x41000000;     /* 8.0 */
    }
    switch ((uint32_t)mode) {
    case 3:
        Sim_10044c7a();
        break;
    case 1: {
        float a = W8u_Rate(0x1015fdac);
        float c = W8u_Rate(0x1015fdb0);
        uint32_t b = G32(0x1015fda0);
        float d = W8u_Rate(0x1015fda4);

        if (G32(0x10179cd4) != 0 && G32(0x1015ef10) == 0)
            d = (float)(W8U_TICKS() * (double)GFL(0x10111568));
        W8U_CALL_10044d0b(Bits_OfFloat(a), Bits_OfFloat(c), b, Bits_OfFloat(d));
        break;
    }
    case 0:
    case 6:
        Sim_100451a2();
        break;
    case 4:
        Sim_100453a3();
        break;
    default: {
        float fwd = (float)(W8U_TICKS() * (double)GFL(0x1015fdb0) / (double)GFL(0x10111564) * (double)GFL(0x1011156c));
        float turn = (float)(W8U_TICKS() * (double)GFL(0x1015fdac) / (double)GFL(0x10111564) * (double)GFL(0x1011156c));
        float side = W8u_Rate(0x1015fda8);
        float pitch = W8u_Rate(0x1015fda4);
        float lift;

        if ((double)GFL(0x1015fda0) > (double)GFL(0x10111570))
            lift = W8u_Rate(0x10111574);
        else if ((G32(0x1015fda0) & 0x80000000u) != 0)
            lift = W8u_Rate(0x10111578);
        else
            lift = 0.0f;
        *GBYTES(0x1015fdc4) = 1;
        Sim_1004548f(Bits_OfFloat(fwd), Bits_OfFloat(turn), Bits_OfFloat(side), Bits_OfFloat(pitch),
                     Bits_OfFloat(lift));
        break;
    }
    }
    G32(0x1013aed4) = (uint32_t)mode;
    Sim_100a94fa();
    W8U_CALL_100456fc();
}

// FUNCTION: HGSIM 0x100451a2
void __cdecl Sim_100451a2(void)
{
    uint8_t *cam;

    if (G32(0x1013aed4) != 0) {
        G32(0x109b9b90) = G32(0x1015dc78);
        G32(0x109b9b98) = 0;
        G32(0x109b9ae0) = G32(0x1015dc78);
        G32(0x109b9ae8) = 0;
        Sim_10044255();
        Sim_10044909(0);
    }
    G32(0x1015ef0c) = 1;
    G32(0x1015ef08) = 0;
    if (*GBYTES(0x1015fdbe) != 0)
        G32(0x1015fd9c) = 0xc28c0000;      /* -70.0 */
    else if (*GBYTES(0x1015fdbf) != 0)
        G32(0x1015fd9c) = 0x428c0000;      /* 70.0 */
    else if (*GBYTES(0x1015fdc0) != 0)
        G32(0x1015fd98) = 0xc2480000;      /* -50.0 */
    else if (*GBYTES(0x1015fdc1) != 0)
        G32(0x1015fd98) = 0x42200000;      /* 40.0 */
    else if (*GBYTES(0x1015ef2c) == 0) {
        G32(0x1015fd9c) = 0;
        G32(0x1015fd98) = 0;
    }
    *GBYTES(0x1015ef2c) = (*GBYTES(0x1015fdbf) == 0 && *GBYTES(0x1015fdbe) == 0 && *GBYTES(0x1015fdc0) == 0 &&
                           *GBYTES(0x1015fdc1) == 0) ? 1 : 0;
    if (Sim_10043f1a() == 0) {
        cam = W8U_CAM();
        Sim_10045308((float *)(cam + 0x10), (float *)(cam + 0xc), (uint32_t *)(cam + 0x14), (uint32_t *)cam,
                     (uint32_t *)(cam + 4), (uint32_t *)(cam + 8));
    }
}

// FUNCTION: HGSIM 0x1004548f
void __cdecl Sim_1004548f(uint32_t f8, uint32_t fc, uint32_t f10, uint32_t f14, uint32_t f18)
{
    float s4, s8, sc, s18, s14, h, top;
    double r;

    s4 = (float)W8U_CALL_10084766(U32(W8U_CAM(), 0xc));
    s8 = (float)W8U_CALL_10084766(Bits_OfFloat((float)((double)FL(W8U_CAM(), 0xc) + (double)GFL(0x101115a8))));
    sc = (float)W8U_CALL_10084766(U32(W8U_CAM(), 0x10));
    s18 = (float)W8U_CALL_10084766(Bits_OfFloat((float)((double)FL(W8U_CAM(), 0x10) + (double)GFL(0x101115a8))));
    (void)sc;
    if (G32(0x1013aed4) != 2) {
        if (G32(0x10179cd4) != 0) {
            FL(W8U_CAM(), 0) = (float)((double)FL(W8U_CAM(), 0) - (double)GFL(0x1015eeec) * (double)s4);
            FL(W8U_CAM(), 8) = (float)((double)FL(W8U_CAM(), 8) - (double)GFL(0x1015eeec) * (double)s8);
        }
        U32(W8U_CAM(), 0x14) = 0;
        G32(0x109b9ba8) = 0;
        G32(0x109b9ba0) = G32(0x1015dc78);
        G32(0x1013aed0) = G32(0x1013aecc);
        Sim_10044909(0);
    }
    FL(W8U_CAM(), 0) = (float)((double)Float_OfBits(f10) * (double)s8 + (double)FL(W8U_CAM(), 0));
    FL(W8U_CAM(), 8) = (float)((double)FL(W8U_CAM(), 8) - (double)Float_OfBits(f10) * (double)s4);
    FL(W8U_CAM(), 0x10) = (float)((double)FL(W8U_CAM(), 0x10) - (double)Float_OfBits(f18));
    if ((double)FL(W8U_CAM(), 0x10) > (double)GFL(0x101115a8))
        U32(W8U_CAM(), 0x10) = 0x42b40000;     /* 90.0 */
    else if (!((double)FL(W8U_CAM(), 0x10) >= (double)GFL(0x101115b4)))
        U32(W8U_CAM(), 0x10) = 0xc2b40000;     /* -90.0 */
    FL(W8U_CAM(), 4) = (float)((double)Float_OfBits(f8) * (double)GFL(0x10111590) + (double)FL(W8U_CAM(), 4));
    GFL(0x109b9ba4) = (float)((double)Float_OfBits(fc) * (double)GFL(0x101115b8));
    s14 = Sim_10047bcd(SIM_PTR(uint32_t, 0x109b9ba0));
    r = W8U_FABS_1004((double)s14);
    if (!(r >= (double)GFL(0x101115bc)))
        s14 = 0.0f;
    FL(W8U_CAM(), 0xc) = (float)((double)FL(W8U_CAM(), 0xc) + (double)Float_OfBits(f14));
    FL(W8U_CAM(), 0) = (float)((double)FL(W8U_CAM(), 0) - (double)s14 * (double)s4 * (double)s18);
    FL(W8U_CAM(), 8) = (float)((double)FL(W8U_CAM(), 8) - (double)s14 * (double)s8 * (double)s18);
    h = Sim_1004487c((const float *)W8U_CAM());
    if ((double)h > (double)FL(W8U_CAM(), 4)) {
        if (!((double)FL(W8U_CAM(), 4) >= (double)h))
            top = h;
        else
            top = FL(W8U_CAM(), 4);
        FL(W8U_CAM(), 4) = top;
    }
}
