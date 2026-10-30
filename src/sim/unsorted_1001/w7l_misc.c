#include "w7l.h"
#include "w2h.h"
#include "w2e.h"
#include "w5y.h"
#include "w4u.h"
#include "w6d.h"
#include "../unsorted_1000/w2h.h"
#include "../unsorted_1000/w2l.h"
#include "../unsorted_1003/w2e.h"
#include "../unsorted_1004/fns.h"
#include "../unsorted_1005/w2h.h"
#include "../unsorted_1006/w3q.h"
#include "../unsorted_1008/w3o.h"
#include "../unsorted_1009/w2l.h"
#include "../unsorted_100d/fns.h"
#include "../unsorted_100f/w2e.h"
#include "../unsorted_100f/w2g.h"
#include "../unsorted_100f/w2k.h"
#include "../unsorted_100f/w6h.h"
#include "../win/w4u.h"
#include "../util/util.h"
#include "../util/w3o.h"

// FUNCTION: HGSIM 0x1001150d
void __cdecl Sim_1001150d(void *obj)
{
    int i;

    W7L_CALL_1001457f(obj);
    U16(obj, 0x1a8) = 0;
    U16(obj, 0x1aa) = 0;
    U32(obj, 0x1ae) = 0;
    if (U32(obj, 0x14) == 2 || (U32(obj, 0x14) == 3 && U32(obj, 0xc) == G32(0x1017b8c8)))
        U16(obj, 0x1a6) = 0xc;
    else
        U16(obj, 0x1a6) = 0;
    for (i = 0; i < 3; i++)
        Sim_10016a6b(obj, (int16_t)i, -1, 0);
    U32(U32(obj, 0x170), 0) = 0;
    U16(obj, 0x198) = 0;
    U16(obj, 0x1a0) = 0;
    U32(obj, 0x1be) = 0;
    U32(obj, 0x1c6) = 0;
    for (i = 0; i < 3; i++)
        U32(obj, 0x20c + 4 * i) = 0xffffffffu;
    U32(obj, 0x208) = 0;
    U32(obj, 0x228) = 0;
    U32(obj, 0x224) = 0;
    U32(obj, 0x220) = 0;
    U32(obj, 0x21c) = 0;
    U32(obj, 0x218) = 0;
    Sim_10017852(obj);
    Sim_100119a7(obj);
}

/* -1, 0 or 1: 0 when |x| is below the float at 0x10111120 (or a NaN), else the sign bit of x. */
static int32_t W7l_Sign(float x)
{
    if (!(W7L_FABS_1001(x) >= GFL(0x10111120)))
        return 0;
    return (Bits_OfFloat(x) & 0x80000000u) ? -1 : 1;
}

// FUNCTION: HGSIM 0x100125d9
void __cdecl Sim_100125d9(void *obj, float *value)
{
    float q = (float)((double)*value / GFL(0x10111100) / GFL(0x10111118));
    float r = (float)((double)FL(U32(obj, 0x54), 8) * q);
    float w = (float)((double)*value - r);
    float e = (float)((double)FL(obj, 0x5c) - w);
    float t;

    if (W7L_FABS_1001(e) > GFL(0x1011111c))
        w = (float)((double)FL(obj, 0x5c) - (double)W7l_Sign(e) * GFL(0x1011111c));
    *(u32a *)value = Bits_OfFloat(w);
    t = (float)((double)W7l_Sign(e) * GFL(0x10111124));
    if (FL(obj, 0x64) != t && FL(obj, 0x64) == FL(obj, 0x64) && t == t) {
        float d = (float)((double)FL(obj, 0x64) - t);

        FL(obj, 0x64) = (float)((double)FL(obj, 0x64) - (double)W7l_Sign(d) * GFL(0x10111128));
    }
}

// FUNCTION: HGSIM 0x10012970
void __cdecl Sim_10012970(void)
{
    char text[0x64];
    int i;

    if (G32(0x10a3ade8) != 0)
        Mono_PutText((const char *)GBYTES(0x10131104), 0, 3, 0x78);
    for (i = 0; i < GS32(0x1015dc64); i++) {
        uint8_t *rec = BYTES(PTRTAB(i));
        uint32_t attr;

        W7L_SPRINTF_1001(text, (const char *)GBYTES(0x10131158), (uintptr_t)rec != G32(0x1015ef14) ? 0x20 : 0x3e,
                         U32(rec, 4), U32(rec, 8), (int)(int8_t)U8(rec, 0x1b7), (int)(int8_t)U8(rec, 0x1b6),
                         rec + 0x124);
        attr = 7;
        if ((U32(U32(rec, 0x28), 0x4f0) & 4) == 0)
            attr |= 8;
        if ((int32_t)(U32(rec, 4) + 4) < 0x19 && G32(0x10a3ade8) != 0)
            Mono_PutText(text, 0, (short)(U32(rec, 4) + 4), (short)attr);
    }
}

// FUNCTION: HGSIM 0x10015018
int16_t __cdecl Sim_10015018(void *obj, int16_t sel, int16_t key)
{
    int16_t result = -1;
    uint32_t group = U32(obj, 4);

    if ((sel & 0x6000) == 0) {
        if ((sel & 0xfff) != (key & 0xfff))
            return sel;
        return -1;
    }
    if (key != -1) {
        if (sel & 0x2000)
            return -1;
        key = (int16_t)(key & 0xff);
    }
    if (sel & 0x200) {
        if (sel & 0x4000) {
            if (sel == 0x4201) {
                for (;;) {
                    key = (int16_t)(key + 1);
                    if (key >= GS32(0x1015dc64))
                        break;
                    if (Sim_10052c60(key, (int)group) == 0 && Sim_10052be7(key) != 2) {
                        result = key;
                        break;
                    }
                }
            } else if (sel == 0x4200) {
                for (;;) {
                    key = (int16_t)(key + 1);
                    if (key >= GS32(0x1015dc64))
                        break;
                    if ((uint32_t)Sim_10052aaf((int)U32(PTRTAB(key), 8)) == G32(0x1017b8c8)
                        || (G32(0x1015d110) != 0 && Sim_10052be7(key) == 0)) {
                        result = key;
                        break;
                    }
                }
            }
        } else if (sel == 0x2200) {
            result = (int16_t)*SIM_PTR(u16a, 0x1017b8c8);
        } else if (sel == 0x2201) {
            result = (int16_t)Sim_10052aaf((int)U32(obj, 8));
        } else if (sel == 0x2202) {
            result = S16(obj, 4);
        }
    } else if ((sel & 0x400) == 0 && (sel & 0x100) != 0) {
        if (sel == 0x2100)
            result = (int16_t)W7L_CALL_1007cbcb(U32(obj, 8));
        else if (sel == 0x2101)
            result = S16(obj, 0x1d6);
    }
    if (result != -1 && (result & 0xf00) == 0)
        result = (int16_t)(result | (sel & 0xf00));
    return result;
}

// FUNCTION: HGSIM 0x10015f5b
int16_t __cdecl Sim_10015f5b(void *a, void *obj)
{
    uint32_t found = 0;
    int i, best_index;
    uint32_t best;
    uint8_t saved[0x28];

    if (W7L_CALL_10033eb3(U32(obj, 0x58), U32(obj, 0x5c), U32(obj, 0x60), 0x46ea6000u, &found) == 0 || found == 0)
        return 0x1000;
    if ((S16(obj, 0x1a6) == 3 || S16(obj, 0x1a6) == 2) && (U16(obj, 0x1aa) & 0x200) != 0)
        return S16(obj, 0x1aa);
    if (U32(a, 0xf0) != 0xffffffffu && (U32(a, 0xf0) & 0x200) != 0 && Sim_10052be7((int)(U32(a, 0xf0) & 0xff)) != 0)
        return S16(a, 0xf0);
    best_index = 0;
    best = 0x7f7fc99eu;         /* 3.4e38 */
    Mem_Move(saved, BYTES(obj) + 0xd4, 0x28);
    for (i = 0; i < GS32(0x1015dc64); i++) {
        if (Sim_10052be7(i) != 0) {
            Sim_1001403a(obj, (short)(i | 0x200));
            if (!(FL(obj, 0xd8) >= Float_OfBits(best))) {
                best_index = i;
                best = U32(obj, 0xd8);
            }
        }
    }
    Mem_Move(BYTES(obj) + 0xd4, saved, 0x28);
    return (int16_t)(best_index | 0x200);
}

// FUNCTION: HGSIM 0x100178f4
void __cdecl Sim_100178f4(void *obj)
{
    uint32_t period = W7L_FABS_1001(FL(U32(obj, 0x54), 8)) >= GFL(0x10111120) ? 0x20 : 0x40;
    uint32_t step;

    if (U32(obj, 0) != 1)
        return;
    if (S32(obj, 0x200) > (int32_t)(G32(0x1015dc78) + period))
        U32(obj, 0x200) = G32(0x1015dc78) + period;
    if (S32(obj, 0x200) > GS32(0x1015dc78))
        return;
    step = U32(obj, 0x218) != 0 ? 0x40a00000u : 0xc0a00000u;   /* 5.0 or -5.0 */
    FL(U32(obj, 0x54), 4) = (float)((double)FL(U32(obj, 0x54), 4) + Float_OfBits(step));
    if (W7L_FABS_1001(FL(U32(obj, 0x54), 4)) > GFL(0x10111140))
        U32(obj, 0x218) = U32(obj, 0x218) == 0;
    U32(obj, 0x200) = G32(0x1015dc78) + period;
}

// FUNCTION: HGSIM 0x100196d5
int __cdecl Sim_100196d5(void *a, void *b, void *c)
{
    int mine, i;

    if (U32(c, 8) != 1)
        return 1;
    mine = Sim_10052be7((int)U32(a, 4));
    for (i = 0; i < 0x200; i++) {
        uint8_t *e = GBYTES(0x1096fc80) + i * 0x156;
        uint32_t px, py, pz;
        float dx, dy, dz;

        if (U32(e, 0x48) == 0 || U32(e, 0x3c) == 0)
            continue;
        if (U32(e, 0) != 3 && U32(e, 0) != 0xa && U32(e, 0) != 0xc && U32(e, 0) != 9)
            continue;
        if (mine == Sim_10052be7((int)U32(e, 0x50)))
            continue;
        Sim_100a9cfb(BYTES(U32(e, 0x3c)), &px, &py, &pz);
        dx = (float)((double)FL(a, 0x58) - Float_OfBits(px));
        dy = (float)((double)FL(a, 0x5c) - Float_OfBits(py));
        dz = (float)((double)FL(a, 0x60) - Float_OfBits(pz));
        if (Sim_10084a94(dx, dy, dz) > GFL(0x1011118c))
            continue;
        if ((double)FL(e, 0x18) * dx + (double)FL(e, 0x1c) * dy + (double)FL(e, 0x20) * dz > GFL(0x10111190)) {
            U32(c, 8) = 2;
            U32(c, 0x2c) = U32(b, 0x2c);
            U32(c, 0x10) = 0;
            U32(c, 0x1c) = 0;
            U32(c, 0x18) = 0;
            W7L_CALL_100c398d(c, U32(a, 0x28));
        }
    }
    return 1;
}

// FUNCTION: HGSIM 0x10019da0
int __cdecl Sim_10019da0(int mode, char *text, int n, int div)
{
    uint8_t *rec;
    u32a *slot;
    int key;

    if (div == 0)
        div = 1;
    if (mode == 0) {
        char *p;

        rec = (uint8_t *)SIM_API(W7lHeapAllocFn, 0x10ac5658)(G32(0x1017b8f8), 1, 0x50);
        if (rec == 0)
            return 0;
        slot = (u32a *)(uintptr_t)Sim_100cc8ba((int)Sim_100cc6e0());
        *slot = (uint32_t)(uintptr_t)rec;
        U32(rec, 0xc) = 0;
        U32(rec, 4) = 0;
        U32(rec, 8) = 0xffffffffu;
        p = Crt_StrChr(text, 0x3b);
        if (p != 0) {
            *p = 0;
            p++;
            for (p = Str_NextToken_100fd560(p, (const char *)GBYTES(0x10131754)); p != 0;
                 p = Str_NextToken_100fd560(0, (const char *)GBYTES(0x10131758))) {
                if (S32(rec, 0xc) >= 0x10)
                    break;
                U32(rec, 0x10 + 4 * U32(rec, 0xc)) = (uint32_t)Sim_100fd540(p, 0, 0) << 4;
                U32(rec, 0xc) = U32(rec, 0xc) + 1;
            }
        }
        key = Sim_100db770(Crt_AToI(text));
        key = Sim_100b38c5(key);
        if (key == -1)
            return 0;
        U32(rec, 0) = (uint32_t)(uintptr_t)Sim_100b399c(key);
    } else if (mode == 1) {
        int32_t unused, start, i;

        slot = (u32a *)(uintptr_t)Sim_100cc8ba((int)Sim_100cc6e0());
        rec = BYTES(*slot);
        if (rec == 0 || U32(rec, 0) == 0 || U32(U32(rec, 0), 0) == 0)
            return 0;
        U32(rec, 4) = U32(U32(rec, 0), 0);
        if (S32(rec, 8) == -1)
            Sim_100094df(BYTES(U32(rec, 4)), &unused, (int32_t *)(rec + 8));
        start = n / div % S32(rec, 0xc);
        for (i = 0; i < S32(rec, 8); i++)
            Sim_100096b2(BYTES(U32(rec, 4)), i, U32(rec, 0x10 + 4 * ((start + i) % S32(rec, 0xc))));
    }
    return 1;
}

/* Copies a 16-byte key value as four dwords. */
static void W7l_Copy16(void *dst, const void *src)
{
    U32(dst, 0) = U32(src, 0);
    U32(dst, 4) = U32(src, 4);
    U32(dst, 8) = U32(src, 8);
    U32(dst, 0xc) = U32(src, 0xc);
}

// FUNCTION: HGSIM 0x1001bb9d
int __cdecl Sim_1001bb9d(int t, int period, const void *keys, float *out)
{
    int32_t count = S32(keys, 0);
    int32_t i, t0 = 0, t1 = 0;
    uint32_t q0[4], q1[4];
    float f;

    if (out == 0 || count <= 0)
        return 1;
    t %= period;
    for (i = 0; i < count; i++) {
        int32_t kt = S32(keys, 4 + i * 0x14);

        if (t < kt) {
            int32_t j = i - 1 < 0 ? count - 1 : i - 1;

            t0 = S32(keys, 4 + j * 0x14);
            W7l_Copy16(q0, BYTES(keys) + 8 + j * 0x14);
            t1 = S32(keys, 4 + i * 0x14);
            if (i - 1 < 0)
                t1 = (int32_t)((uint32_t)t1 + (uint32_t)period);
            W7l_Copy16(q1, BYTES(keys) + 8 + i * 0x14);
            break;
        }
        if (t == kt) {
            W7l_Copy16(out, BYTES(keys) + 8 + i * 0x14);
            return 0;
        }
        if (i + 1 == count) {
            t0 = S32(keys, 4 + i * 0x14);
            W7l_Copy16(q0, BYTES(keys) + 8 + i * 0x14);
            t1 = (int32_t)((uint32_t)S32(keys, 4) + (uint32_t)period);
            W7l_Copy16(q1, BYTES(keys) + 8);
            break;
        }
    }
    t0 = (int32_t)((uint32_t)t0 + 1);
    f = (float)((double)(int32_t)((uint32_t)t - (uint32_t)t0) / (double)(int32_t)((uint32_t)t1 - (uint32_t)t0));
    W7L_CALL_100608ef(q0, q1, f, out);
    return 0;
}

// FUNCTION: HGSIM 0x1001cedf
void __cdecl Sim_1001cedf(uint32_t a)
{
    u32a *slot = (u32a *)W7L_CALL_1001b902(a);
    uint8_t *obj, *e1 = 0, *e2 = 0, *e3 = 0;
    int32_t i, n;

    if (slot == 0)
        return;
    obj = BYTES(*slot);
    if (obj == 0)
        return;
    for (i = 0; i < S32(U32(obj, 0x14), 0xc); i++) {
        uint8_t *entry = obj + 0x1c + i * 0xc;
        uint32_t id = U32(U32(entry, 0), 4);

        if (id == G32(0x1015d190))
            e1 = entry;
        else if (id == G32(0x1015d194))
            e2 = entry;
        else if (id == G32(0x1015d19c))
            e3 = entry;
        if (e3 != 0 && e2 != 0 && e1 != 0)
            break;
    }
    if (e1 == 0 || e2 == 0 || e3 == 0)
        return;
    n = S32(U32(e1, 8), 0);
    if (n != S32(U32(e2, 8), 0) || n != S32(U32(e3, 8), 0))
        return;
    for (i = 0; i < n; i++) {
        float qa[4], qb[4], q[4];
        uint8_t *dst;

        W7l_Copy16(qa, BYTES(U32(e1, 8)) + 8 + i * 0x14);
        W7l_Copy16(qb, BYTES(U32(e2, 8)) + 8 + i * 0x14);
        Quat_Multiply(qa, qb, q);
        q[0] = -q[0];
        q[1] = -q[1];
        q[2] = -q[2];
        dst = BYTES(U32(e3, 8)) + 8 + i * 0x14;
        W7l_Copy16(dst, q);
    }
}

// FUNCTION: HGSIM 0x1001ee7c
void __cdecl Sim_1001ee7c(int index, uint32_t value)
{
    uint32_t fn = 0, old;

    if (index < 0 || index >= 5)
        return;
    if (index != 2)
        Sim_1001f335();
    switch (index) {
    case 0:
        old = G32(0x101321b4);
        G32(0x101321b4) = value;
        if (old != G32(0x101321b4))
            fn = W7L_FN_100029eb;
        break;
    case 1:
        old = G32(0x101321b8);
        G32(0x101321b8) = value;
        if (value != 0)
            G32(0x101321c0) = G32(0x101321c0) | 2;
        else
            G32(0x101321c0) = G32(0x101321c0) & ~2u;
        if (old != G32(0x101321b8))
            fn = W7L_FN_10001dfc;
        break;
    case 2:
        old = G32(0x101321bc);
        G32(0x101321bc) = value;
        if (old != G32(0x101321bc)) {
            if (Sim_1009eb24() != 0)
                fn = W7L_FN_10003102;
            if (old == 0)
                Sim_1001f26e();
        }
        Sim_1009e441();
        break;
    case 3:
        old = G32(0x101321b0);
        G32(0x101321b0) = value;
        if (old != G32(0x101321b0))
            fn = 0;
        break;
    }
    if (fn != 0)
        SIM_FN(void(__cdecl *)(void), fn)();
}
