#include "w6e.h"
#include "w5a.h"
#include "w2e.h"
#include "w5y.h"
#include "w3s.h"
#include "fns.h"
#include "../core/w2a.h"
#include "../core/w2b.h"
#include "../unsorted_1000/w5x.h"
#include "../unsorted_1007/w2e.h"
#include "../unsorted_100a/w5x.h"
#include "../unsorted_100a/w5y.h"
#include "../view/w3r.h"
#include "../util/access.h"
#include "w3p.h"

typedef uint32_t(__cdecl *W6eCall3Fn)(uint32_t, uint32_t, uint32_t);
typedef uint32_t(__cdecl *W6eCall1Fn)(uint32_t);

// FUNCTION: HGSIM 0x1004131b
void __cdecl Sim_1004131b(const uint8_t *msg)
{
    uint32_t entry;

#ifndef NETG_MUTATE /* the checker's negative check only: no guard */
    if (g_w34NetOver != 0)          /* 1.2 (0x10034fce): ignored once the game is over */
        return;
#endif

    if (PTRTAB(msg[2]) == 0)
        return;
    entry = PTRTAB(msg[2]);
    if (U32(entry, 4) == G32(0x1017b8c8))
        return;
    if (U32(entry, 0x14) == 3 && U32(entry, 0xc) == G32(0x1017b8c8))
        return;
    if ((S16(entry, 0x1c) & 2) != 0)
        return;
    if ((S16(entry, 0x1c) & 4) != 0)
        return;
    W6E_CALL_100386d5(U32(entry, 0x28), U32(msg, 4), msg[3]);
    Sim_100aa799((void *)(uintptr_t)U32(entry, 0x48));
}

// FUNCTION: HGSIM 0x10041b2d
void __cdecl Sim_10041b2d(const uint8_t *msg, int slot)
{
    const uint8_t *p = msg + 2;
    int32_t stamp = S32(p, 0);
    int bit = 0;
    uint8_t bits;
    int i;

#ifndef NETG_MUTATE /* the checker's negative check only: no guard */
    if (g_w34NetOver != 0)          /* 1.2 (0x10035923): ignored once the game is over */
        return;
#endif

    p += 4;
    if (stamp <= g_net10SnStamp[slot])
        return;
    g_net10SnStamp[slot] = stamp;
    bits = *p;
    for (i = 0; i < 0xfe; i++) {
        if ((bits & 0x80) != 0 && (S16(GBYTES(0x109e9ba0), i * 0x50) & 4) == 0)
            W6E_CALL_1002597b((uint32_t)i);
        bits = (uint8_t)(bits << 1);
        bit++;
        if (bit == 8) {
            bit = 0;
            p++;
            bits = *p;
        }
    }
}

// FUNCTION: HGSIM 0x10041d67
int __cdecl Sim_10041d67(void)
{
    if ((int8_t)U8(G32(0x1015d004), 0x1e) == 0)
        return 0;
    if (U32(G32(0x1015d004), 0x16) == 0)
        return 0;
    G32(0x1015ecd4) = U32(G32(0x1015d004), 0xc);
    if (G32(0x1015ecd4) == 0)
        W6E_CALL_100bc280(0x14, 0);
    ((W6eCall3Fn)(uintptr_t)G32(0x1015ea94))(G32(0x1015ecd4), 0, 0);
    G32(0x1015ec5c) = ((W6eCall1Fn)(uintptr_t)G32(0x1015ead8))(G32(0x1015ecd4));
    G32(0x1015ec58) = G32(0x1015ec5c);
    *SIM_PTR(u16a, 0x1013acac) = U16(G32(0x1015d004), 0x10);
    G32(0x1015eccc) = 1;
    G32(0x1015dc78) = (uint32_t)Sim_1002ab9d();
    G32(0x1015dc7c) = (uint32_t)Sim_1002ac1b();
    U8(G32(0x1017b8c4), 9) = 1;
    if (U8(G32(0x1015d004), 0x136) == 2)
        G32(0x1013ac9c) = 0x2d;
    G32(0x1015ec54) = 0;
    return 1;
}

// FUNCTION: HGSIM 0x10044a69
void __cdecl Sim_10044a69(int forward, int reset)
{
    uint32_t step = 0xffffffffu;

    if (G32(0x1015dc64) == 0)
        return;
    if (reset != 0) {
        G32(0x1015ef18) = G32(0x1017b8c8);
        step = 0;
    } else if (forward != 0) {
        step = 1;
    }
    G32(0x1015ef18) = G32(0x1015ef18) + step;
    if (GS32(0x1015ef18) < 0)
        G32(0x1015ef18) = G32(0x1015dc64) - 1;
    else if (GS32(0x1015ef18) >= GS32(0x1015dc64))
        G32(0x1015ef18) = 0;
    G32(0x1015ef14) = PTRTAB(GS32(0x1015ef18));
    if ((S16(G32(0x1015ef14), 0x1c) & 0x4800) != 0)
        Sim_10044a69(forward, reset);
    G32(0x1013aed4) = 0xffffffffu;
}

// FUNCTION: HGSIM 0x10044b2c
void __cdecl Sim_10044b2c(uint32_t *x, uint32_t *y, uint32_t *z, uint32_t *a, uint32_t *b, uint32_t *c)
{
    uint32_t entry;
    uint32_t px, py, pz;

    *(u32a *)c = 0;
    *(u32a *)b = 0;
    *(u32a *)a = 0;
    *(u32a *)z = 0;
    *(u32a *)y = 0;
    *(u32a *)x = 0;
    entry = G32(0x1015ef14);
    if (entry == 0)
        return;
    *(u32a *)x = U32(entry, 0x64);
    *(u32a *)y = U32(entry, 0x68);
    *(u32a *)z = U32(entry, 0x6c);
    if (U32(entry, 0x4c) != 0) {
        if (G32(0x1015ef1c) != 0)
            W6E_F(b, 0) = W6E_F(b, 0) + W6E_F(G32(0x1015ef1c), 0);
        Mat3x4_TransformPoint(Sim_100aa863((void *)(uintptr_t)U32(entry, 0x4c)), (float *)a, (float *)b, (float *)c);
        Sim_100a9ca3((void *)(uintptr_t)U32(entry, 0x4c), (uint32_t)(uintptr_t)&px, (uint32_t)(uintptr_t)&py,
                     (uint32_t)(uintptr_t)&pz);
        if (G32(0x10179cd4) != 0) {
            *(u32a *)x = px;
            *(u32a *)y = py;
            *(u32a *)z = pz;
        } else {
            W6E_F(x, 0) = W6E_F(x, 0) + W6E_F(entry, 0x70);
            W6E_F(y, 0) = W6E_F(y, 0) + W6E_F(entry, 0x74);
            W6E_F(z, 0) = W6e_Float(pz) / 10.0f;
        }
    } else {
        *(u32a *)a = U32(entry, 0x58);
        *(u32a *)b = U32(entry, 0x5c);
        *(u32a *)c = U32(entry, 0x60);
    }
}

// FUNCTION: HGSIM 0x100453a3
void __cdecl Sim_100453a3(void)
{
    uint32_t x, y, z, a, b, c;
    int32_t ticks;
    double product;
    float step;

    Sim_10044b2c(&x, &y, &z, &a, &b, &c);
    if (G32(0x1013aed4) != 4) {
        G32(0x1015ef24) = 0;
        U32(G32(0x1014cae0), 0x10) = 0x42b40000; /* 90.0f */
        G32(0x1015ef28) = G32(0x1015dc78);
        Sim_10044909(0);
    }
    ticks = GS32(0x1015dc80);
    /* the original keeps the unrounded product on the FPU stack and also stores it rounded to a float */
    product = (double)ticks * (double)W6E_GF(0x1013aee4);
    step = (float)product;
    W6E_F(G32(0x1014cae0), 4) =
        (float)((product * 0.5 + (double)W6E_GF(0x1015ef24)) * (double)ticks + (double)W6E_F(G32(0x1014cae0), 4));
    W6E_GF(0x1015ef24) = W6E_GF(0x1015ef24) + step;
    U32(G32(0x1014cae0), 0) = a;
    U32(G32(0x1014cae0), 8) = c;
    W6E_F(G32(0x1014cae0), 0xc) =
        (float)((double)(int32_t)(G32(0x1015dc78) - G32(0x1015ef28)) * (double)0.007f +
                (double)W6E_F(G32(0x1014cae0), 0xc));
}

// FUNCTION: HGSIM 0x10047bcd
float __cdecl Sim_10047bcd(uint32_t *obj)
{
    float delta = W6E_F(obj, 4) - W6E_F(obj, 8);
    float step;

    if (!(W6E_CALL_100fcb70((double)W6E_F(obj, 4) - (double)W6E_F(obj, 8)) >= (double)1.0e-7f)) {
        obj[2] = obj[1];
    } else {
        step = (float)((double)(int32_t)(G32(0x1015dc78) - obj[0]) * (double)delta / (double)W6E_F(obj, 0xc));
        if ((W6e_Bits(delta) & 0x80000000u) != 0 && !(step > delta))
            obj[2] = obj[1];
        else if (!(delta > 1.0e-7f) || !(step >= delta))
            W6E_F(obj, 8) = W6E_F(obj, 8) + step;
        else
            obj[2] = obj[1];
    }
    obj[0] = G32(0x1015dc78);
    return W6E_F(obj, 8);
}

/* One code of 0x100488a3: the text that is shown with label [0x109b98c0]. */
static void ShowCode(uint32_t text)
{
    Sim_1007df6c((const char *)(uintptr_t)text, (const char *)(uintptr_t)G32(0x109b98c0), 0x16a, 0x32);
}

static int Matches(uint32_t code)
{
    return Sim_10048762((const char *)(uintptr_t)code);
}

// FUNCTION: HGSIM 0x100488a3
void __cdecl Sim_100488a3(uint32_t key)
{
    if (G32(0x1015f1f8) == 0) {
        G32(0x1015f1f8) = 1;
        Sim_100487d0();
    }
    if (Sim_10048720((short)key) == 0)
        return;
    if (Matches(G32(0x1013b380))) {
        G32(0x101655b0) = 1;
        if (U8(G32(0x1017b8c4), 1) == 0) {
            U8(G32(0x1017b8c4), 1) = 1;
            ShowCode(G32(0x1013b384));
        } else {
            U8(G32(0x1017b8c4), 1) = 0;
            ShowCode(G32(0x1013b388));
        }
        return;
    }
    if (Matches(G32(0x1013b38c))) {
        G32(0x101655b0) = 1;
        if (U8(G32(0x1017b8c4), 0) == 0) {
            U8(G32(0x1017b8c4), 0) = 1;
            ShowCode(G32(0x1013b390));
        } else {
            U8(G32(0x1017b8c4), 0) = 0;
            ShowCode(G32(0x1013b394));
        }
        return;
    }
    if (Matches(G32(0x1013b398))) {
        G32(0x1015cfec) = (uint32_t)((int32_t)(G32(0x1015cfec) + 1u) % 2);
        if (U8(G32(0x1017b8c4), 4) == 0) {
            U8(G32(0x1017b8c4), 4) = 1;
            ShowCode(G32(0x1013b39c));
        } else {
            U8(G32(0x1017b8c4), 4) = 0;
            ShowCode(G32(0x1013b3a0));
        }
        return;
    }
    if (Matches(G32(0x1013b3a4)))
        return;
    if (Matches(G32(0x1013b3bc)))
        return;
    if (Matches(G32(0x1013b3d4))) {
        G32(0x101655b0) = 1;
        G32(0x10179d2c) = 1;
        return;
    }
    if (Matches(G32(0x1013b3e0)))
        return;
    if (Matches(G32(0x1013b3ec)))
        return;
    if (Matches(G32(0x1013b3f8)))
        return;
    if (Matches(G32(0x1013b404))) {
        G32(0x101655b0) = 1;
        Sim_100448cd(2);
        G32(0x1015da24) = 1;
        ShowCode(G32(0x1013b408));
        return;
    }
    if (Matches(G32(0x1013b410))) {
        G32(0x10176930) = G32(0x10176930) ^ 1u;
        return;
    }
    if (Matches(G32(0x1013b41c)))
        return;
    if (Matches(G32(0x1013b428)))
        return;
    if (Matches(G32(0x1013b434))) {
        G32(0x1015d030) = G32(0x1015d02c) == 0;
        G32(0x1015d02c) = G32(0x1015d02c) == 0;
    }
}

// FUNCTION: HGSIM 0x1004ff72
void __cdecl Sim_1004ff72(void *obj, uint32_t base, uint32_t *point)
{
    s32a *last = SIM_PTR(s32a, 0x109b8b60);
    int32_t slot = S32(obj, 4);
    float local[3];
    uint32_t text;

    if (U32(obj, 0x8c) != U32(obj, 0x90)) {
        if (U32(obj, 0x8c) == (uint32_t)last[slot])
            return;
    } else if (last[slot] == -1) {
        return;
    }
    if (U32(obj, 0x8c) == 10 && last[slot] == 0xe)
        text = base + 0xf0;
    else
        text = base + (U32(obj, 0x8c) + 1) * 0x40 + (U32(obj, 0x90) + 1) * 0x10;
    if ((int8_t)U8(text, 0) != 0) {
        if (point == 0)
            point = (uint32_t *)Sim_1004f19d(obj, local);
        Sim_1000ed52(point[0], point[1], point[2], text, G32(0x1015ef0c), (uint32_t)(uintptr_t)obj);
    }
    if (U32(obj, 0x8c) != U32(obj, 0x90))
        last[slot] = S32(obj, 0x8c);
    else
        last[slot] = -1;
}
