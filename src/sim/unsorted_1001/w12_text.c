#include "w12.h"
#include "../simmem.h"
#include "../util/access.h"
#include "../util/util.h"
#include "../unsorted_1000/w2j.h"
#include "../unsorted_1000/w8t.h"
#include "../unsorted_1008/w8w.h"
#include "../unsorted_100a/w3t.h"
#include "../unsorted_100a/w5c.h"
#include "../unsorted_100a/w5y.h"
#include "../unsorted_100b/w2f.h"
#include "../unsorted_100d/fns.h"
#include "../unsorted_100f/w2k.h"
#include "../unsorted_100f/w6h.h"

typedef void *(__stdcall *W12HeapAllocFn)(uint32_t heap, uint32_t flags, uint32_t bytes);

// FUNCTION: HGSIM 0x1001a4fa
int __cdecl Sim_1001a4fa(int mode, char *text)
{
    uint8_t *rec;
    int key;

    if (mode == 0) {
        char name[100]; /* sscanf's name field; it only goes to Sim_100b8d5c, which ignores it (DECISIONS.md) */
        char *p;

        rec = (uint8_t *)SIM_API(W12HeapAllocFn, 0x10ac5658)(G32(0x1017b8f8), 1, 0x1e);
        if (rec == 0)
            return 0;
        *(uint32_t *)(uintptr_t)Sim_100cc8ba((int)Sim_100cc6e0()) = (uint32_t)(uintptr_t)rec;
        U32(rec, 4) = 0xffffffffu;
        U32(rec, 8) = 0;
        U32(rec, 0x18) = 1;
        p = Crt_StrChr(text, 0x3b);
        if (p != 0) {
            *p = 0;
            p++;
            Sim_100fd710(p, (const char *)GBYTES(0x10131774), rec, name, rec + 0x14);
            U32(rec, 0) = U32(rec, 0) * 100;
        }
        U16(rec, 0x1c) = (uint16_t)Sim_100b8d5c();
        key = Sim_100b38c5(Sim_100db770(Crt_AToI(text)));
        if (key == -1)
            return 0;
        U32(rec, 0xc) = (uint32_t)(uintptr_t)Sim_100b399c(key);
    } else if (mode == 1) {
        rec = BYTES(*(uint32_t *)(uintptr_t)Sim_100cc8ba((int)Sim_100cc6e0()));
        if (rec == 0)
            return 0;
        if (U32(rec, 0xc) == 0 || U32(rec, 0x14) == 0 || U32(U32(rec, 0xc), 0) == 0) {
            Sim_1000f029(rec);
            return 0;
        }
        U32(rec, 0x10) = Sim_100096ee((void *)(uintptr_t)U32(U32(rec, 0xc), 0));
        Sim_1000ee23(rec);
    } else if (mode == 2) {
        rec = BYTES(*(uint32_t *)(uintptr_t)Sim_100cc8ba((int)Sim_100cc6e0()));
        if (rec == 0)
            return 1;
        Sim_1000f029(rec);
    }
    return 1;
}

typedef union {
    float f;
    uint32_t u;
} W12Bits1;

static float W12_F(uint32_t u)
{
    W12Bits1 b;

    b.u = u;
    return b.f;
}

static uint32_t W12_U(float f)
{
    W12Bits1 b;

    b.f = f;
    return b.u;
}

// FUNCTION: HGSIM 0x10019fe1
int __cdecl Sim_10019fe1(int mode, char *text, int32_t time)
{
    uint8_t *rec;
    int key;

    if (mode == 0) {
        /* start values (DECISIONS.md): the fields sscanf leaves unfilled count as 0.0; kept as bits, moved like the
         * original moves them */
        uint32_t a = 0, b = 0, c = 0, d = 0;
        char *p;

        rec = (uint8_t *)SIM_API(W12HeapAllocFn, 0x10ac5658)(G32(0x1017b8f8), 1, 0x1c);
        if (rec == 0)
            return 0;
        *(uint32_t *)(uintptr_t)Sim_100cc8ba((int)Sim_100cc6e0()) = (uint32_t)(uintptr_t)rec;
        U32(rec, 0x10) = 0;
        U32(rec, 0xc) = 0;
        U32(rec, 8) = 0;
        U32(rec, 4) = 0;
        p = Crt_StrChr(text, 0x3b);
        if (p != 0) {
            double m;

            *p = 0;
            p++;
            Sim_100fd710(p, (const char *)GBYTES(0x1013175c), &a, &b, &c, &d);
            U32(rec, 8) = a;
            U32(rec, 0xc) = b;
            U32(rec, 0x10) = c;
            U32(rec, 0x14) = W12_U((float)((double)W12_F(d) * (double)W12_F(G32(0x101111a8))));
            m = (double)W12_F(U32(rec, 0x14));
            if (m < 0.0)
                m = -m;
            if (!(m >= (double)W12_F(G32(0x101111ac))))   /* below, or NaN */
                U32(rec, 0x14) = 0x43350000u;
            U32(rec, 0x18) = (uint32_t)time;
        }
        key = Sim_100b38c5(Sim_100db770(Crt_AToI(text)));
        if (key == -1)
            return 0;
        U32(rec, 0) = (uint32_t)(uintptr_t)Sim_100b399c(key);
    } else if (mode == 1) {
        float t;
        uint32_t x, y, z;

        rec = BYTES(*(uint32_t *)(uintptr_t)Sim_100cc8ba((int)Sim_100cc6e0()));
        if (rec == 0 || U32(rec, 0) == 0 || U32(U32(rec, 0), 0) == 0)
            return 0;
        U32(rec, 4) = Sim_100096ee((void *)(uintptr_t)U32(U32(rec, 0), 0));
        t = (float)((double)(int32_t)((uint32_t)time - U32(rec, 0x18)) / (double)W12_F(U32(rec, 0x14)));
        x = W12_U((float)((double)W12_F(U32(rec, 8)) * (double)t));
        y = W12_U((float)((double)W12_F(U32(rec, 0xc)) * (double)t));
        z = W12_U((float)((double)W12_F(U32(rec, 0x10)) * (double)t));
        U32(rec, 0x18) = (uint32_t)time;
        Sim_100aa127((void *)(uintptr_t)U32(rec, 4), x, y, z, 0);
        Sim_100aa799((void *)(uintptr_t)U32(rec, 4));
    }
    return 1;
}

/* The C library fmod of the DLL on doubles (0x100fcb30). */
#define W12_FMOD SIM_FN(double(__cdecl *)(double, double), 0x100fcb30)

// FUNCTION: HGSIM 0x1001a23e
int __cdecl Sim_1001a23e(int mode, char *text, int32_t time)
{
    uint8_t *rec;
    int key;

    if (mode == 0) {
        /* start values (DECISIONS.md): the fields sscanf leaves unfilled count as 0; the last one is not used */
        uint32_t a = 0, b = 0, n = 0, unused = 0;
        char *p;

        rec = (uint8_t *)SIM_API(W12HeapAllocFn, 0x10ac5658)(G32(0x1017b8f8), 1, 0x1c);
        if (rec == 0)
            return 0;
        *(uint32_t *)(uintptr_t)Sim_100cc8ba((int)Sim_100cc6e0()) = (uint32_t)(uintptr_t)rec;
        U32(rec, 0x14) = 0;
        U32(rec, 4) = 0;
        p = Crt_StrChr(text, 0x3b);
        if (p != 0) {
            *p = 0;
            p++;
            Sim_100fd710(p, (const char *)GBYTES(0x10131768), &a, &b, &n, &unused);
            U32(rec, 0xc) = W12_U((float)(*SIM_PTR(const double, 0x101111b0) / (double)W12_F(b)));
            U32(rec, 0x10) = W12_U((float)((double)W12_F(a) * *SIM_PTR(const double, 0x101111b8) *
                                           *SIM_PTR(const double, 0x101111c0) / (double)W12_F(b)));
            U32(rec, 0x14) = n;
            U32(rec, 8) = 0;
        }
        U32(rec, 0x18) = (uint32_t)time;
        key = Sim_100b38c5(Sim_100db770(Crt_AToI(text)));
        if (key == -1)
            return 0;
        U32(rec, 0) = (uint32_t)(uintptr_t)Sim_100b399c(key);
    } else if (mode == 1) {
        float c, s, t, a, b;
        void *node;

        rec = BYTES(*(uint32_t *)(uintptr_t)Sim_100cc8ba((int)Sim_100cc6e0()));
        if (rec == 0 || U32(rec, 0) == 0 || U32(rec, 0x14) == 0 || U32(U32(rec, 0), 0) == 0)
            return 0;
        U32(rec, 4) = Sim_100096ee((void *)(uintptr_t)U32(U32(rec, 0), 0));
        c = Sim_10084766((float)((double)W12_F(U32(rec, 8)) + (double)W12_F(G32(0x101111c8))));
        s = Sim_10084766(W12_F(U32(rec, 8)));
        t = (float)((double)(int32_t)((uint32_t)time - U32(rec, 0x18)) / (double)W12_F(G32(0x101111a8)));
        U32(rec, 0x18) = (uint32_t)time;
        a = (float)((double)W12_F(U32(rec, 0x10)) * (double)t);
        node = (void *)(uintptr_t)U32(rec, 4);
        Sim_100a9d7c(node, (float)((double)c * (double)a), 0.0f, (float)((double)-s * (double)a));
        Sim_100aa799((void *)(uintptr_t)U32(rec, 4));
        b = (float)((double)W12_F(U32(rec, 0xc)) * (double)t);
        Sim_100aa127((void *)(uintptr_t)U32(rec, 4), 0, W12_U(b), 0, 0);
        Sim_100aa799((void *)(uintptr_t)U32(rec, 4));
        U32(rec, 8) = W12_U((float)((double)W12_F(U32(rec, 8)) + (double)b));
        U32(rec, 8) = W12_U((float)W12_FMOD((double)W12_F(U32(rec, 8)), 360.0));
    }
    return 1;
}