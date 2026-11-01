#include "w6d.h"
#include "w2j.h"
#include "w5b.h"
#include "w5c.h"
#include "w3t.h"
#include "../unsorted_1000/w2e.h"
#include "../unsorted_1009/w2h.h"
#include "../unsorted_1009/w2l.h"
#include "../unsorted_100a/w5c.h"
#include "../unsorted_100a/w5x.h"
#include "../unsorted_100b/fns.h"
#include "../util/util.h"
#include "../util/w3p.h"
#include "../view/w3r.h"
#include "../win/w4u.h"
#include "../util/access.h"
#ifdef HG_MUSIC_FILES
#include "../../platform/music.h"
#endif

static uint32_t FloatBits(float f)
{
    union {
        float f;
        uint32_t u;
    } v;

    v.f = f;
    return v.u;
}

static float BitsFloat(uint32_t u)
{
    union {
        uint32_t u;
        float f;
    } v;

    v.u = u;
    return v.f;
}

// FUNCTION: HGSIM 0x10010b92
void __cdecl Sim_10010b92(void)
{
    int mode = Sim_100109e7((int)G32(0x1015d100));

    if (mode >= 0)
        Sim_10010767(mode, 0);
}

// FUNCTION: HGSIM 0x1001274b
void __cdecl Sim_1001274b(void *obj)
{
    float x1 = 0.0f, y1 = 0.0f, z1 = BitsFloat(0xc47a0000u);     /* -1000 */
    float x2 = 0.0f, y2 = 0.0f, z2 = BitsFloat(0xc4a28000u);     /* -1300 */
    const float *m = (const float *)Sim_100aa863((void *)(uintptr_t)U32(obj, 0x48));

    Mat3x4_TransformPoint(m, &x1, &y1, &z1);
    Mat3x4_TransformPoint(m, &x2, &y2, &z2);
    W6D_CALL_10095e0a(x1, y1, z1, (float)((double)x2 - (double)x1), (float)((double)y2 - (double)y1),
                      (float)((double)z2 - (double)z1));
}

// FUNCTION: HGSIM 0x100127f0
float __cdecl Sim_100127f0(const float *a, const float *b)
{
    float u;
    float v;

    W6D_CALL_100cbc6b((float)((double)a[0] - (double)b[0]), (float)((double)a[1] - (double)b[1]),
                      (float)((double)a[2] - (double)b[2]), &u, &u, &v, &u);
    return v;
}

// FUNCTION: HGSIM 0x10013516
int __cdecl Sim_10013516(void *obj, int16_t key, int16_t value, int flag)
{
    int bit = S16(PTRTAB(key & 0xff), 0x1c) & 0x10;
    int result = 1;

    if (flag == 0 || S16(obj, 0x1da) >= GS32(0x1015dc78))
        return bit == 0;
    if ((double)value > (double)F32(obj, 0xd4) && (key & 0x200) != 0 && bit != 0) {
        if (W6D_CALL_1000a6f0(obj, 1) == 0) {
            result = 0;
            *SIM_PTR(u16a, 0x10a38a74) = (uint16_t)(*SIM_PTR(u16a, 0x10a38a74) + 1);
        }
    }
    S16(obj, 0x1da) = (int16_t)(G32(0x1015dc78) + 0x21f);
    return result;
}

// FUNCTION: HGSIM 0x1001448d
float __cdecl Sim_1001448d(uint32_t unused, float f, int n)
{
    (void)unused;
    return Sim_1001655f((float)((double)f / (double)GF32(0x1011114c)), (int)((uint32_t)n << 1), 1);
}

// FUNCTION: HGSIM 0x1001655f
float __cdecl Sim_1001655f(float x, int n, int flag)
{
    double sum;
    float result;

    if (flag != 0) {
        int sign_n = n == 0 ? 0 : (n >= 0 ? 1 : -1);
        int sign_x;

        if (!(W6D_FABS((double)x) >= (double)GF32(0x10111120)))
            sign_x = 0;
        else
            sign_x = (FloatBits(x) & 0x80000000u) != 0 ? -1 : 1;
        if (sign_n != sign_x)
            n = (int)(0u - (uint32_t)n);
    }
    /* the upper limit is compared with the sum before it is rounded to a float, the lower one after (a NaN
     * ends at the lower limit) */
    sum = (double)n * (double)GF32(0x1011116c) + (double)x;
    result = (float)sum;
    if (sum > (double)GF32(0x10111118))
        result = BitsFloat(0x3c800000u);
    else if (X87_LessOrUnordered((double)result, (double)GF32(0x10111170)))
        result = BitsFloat(0xbc800000u);
    return result;
}

// FUNCTION: HGSIM 0x100157ca
void __cdecl Sim_100157ca(void *obj)
{
    uint32_t other = U32(obj, 0x28);
    float r = Sim_10014061((int16_t)U16(obj, 0x1a8));
    uint32_t target = U32(obj, 4);
    uint32_t k = U16(obj, 0x1a8) & 0xffu;
    uint32_t x, y, z;

    switch (U16(obj, 0x1a8) & 0xf00) {
    case 0x400:
        Sim_100b53f0((int)G32(0x109e9ba8u + k * 0x50u), &x, &y, &z);
        r = (float)S32(obj, 0x1d2);
        break;
    case 0x200: {
        uint32_t o = PTRTAB(k);
        uint32_t block = U32(o, 0x28);

        x = U32(o, 0x58);
        y = U32(o, 0x5c);
        z = U32(o, 0x60);
        r = (float)((double)F32(block, 0x53c) + (double)F32(other, 0x53c) + (double)GF32(0x10111168));
        if ((double)S32(obj, 0x1d2) > (double)r)
            r = (float)S32(obj, 0x1d2);
        break;
    }
    case 0x100: {
        uint32_t e = 0x109a8640u + k * 0x54u;

        x = G32(e + 0x18);
        y = G32(e + 0x1c);
        z = G32(e + 0x20);
        break;
    }
    default:
        return;
    }
    Sim_100ca350(target, FloatBits((float)((double)BitsFloat(x) - (double)r)), y, z);
    Sim_100ca350(target, x, y, FloatBits((float)((double)BitsFloat(z) - (double)r)));
    Sim_100ca350(target, FloatBits((float)((double)BitsFloat(x) + (double)r)), y, z);
    Sim_100ca350(target, x, y, FloatBits((float)((double)BitsFloat(z) + (double)r)));
    Sim_100159cf(obj, 0x100);
}

// FUNCTION: HGSIM 0x10019cc0
int __cdecl Sim_10019cc0(int16_t *key)
{
    int result = 0;
    int32_t k = key[0];
    int i;

    for (i = 0; i < GS32(0x1015d188); i++) {
        if (GS32(0x10a37aa0u + (uint32_t)i * 8u) == k)
            break;
    }
    if (i < GS32(0x1015d188)) {
        G32(0x1015d184) = G32(0x10a37aa4u + (uint32_t)i * 8u);
        return -1;
    }
    if (GS32(0x1015d188) < 0x19) {
        if (GS32(0x1015d180) > 0)
            G32(0x1015d184) = G32(0x1015d180) + 1;
        result = W6D_CALL_100463bf(key);
        G32(0x10a37aa4u + G32(0x1015d188) * 8u) = G32(0x1015d184);
        GS32(0x10a37aa0u + G32(0x1015d188) * 8u) = k;
        G32(0x1015d188) = G32(0x1015d188) + 1;
    }
    return result;
}

// FUNCTION: HGSIM 0x1001c107
void __cdecl Sim_1001c107(int index, void *obj)
{
    uint32_t node = U32(PTRTAB(index), 0x48);
    int i;

    if (G32(0x10131750) == 0)
        return;
    G32(0x10131750) = 0;
    for (i = 0; i < S32(U32(obj, 0x14), 0xc); i++) {
        uint32_t entry = (uint32_t)(uintptr_t)obj + 0x1c + (uint32_t)i * 0xc;
        void *found = Sim_100aae0f((void *)(uintptr_t)node, (const char *)(uintptr_t)(U32(entry, 0) + 4));

        if (found != 0) {
            U32(found, 0x9c) = 0;
            U32(found, 0xa0) = entry;
        }
    }
}

// FUNCTION: HGSIM 0x1001f26e
void __cdecl Sim_1001f26e(void)
{
    if (G32(0x1015d1c0) == 0) {
        G32(0x1015d1c0) = 1;
        W6D_SSCANF(SIM_PTR(const char, 0x1015dcb5), SIM_PTR(const char, 0x1013221c), SIM_PTR(uint32_t, 0x101321a8));
    }
#ifdef HG_MUSIC_FILES
    /* 1.2's WAV music start (1.2 0x10015e83), with the Ogg files of platform/music.h and no CD: a track past the
     * last file is -1; otherwise, with music on, it plays from its start and the music is not paused. */
    if (GS32(0x101321a8) < 0 || GS32(0x101321a8) > Music_LastTrack()) {
        G32(0x101321a8) = 0xffffffffu;
    } else if ((G32(0x101321c0) & 8) != 0) {
        Music_Play(GS32(0x101321a8));
        G32(0x1015d1bc) = 0;
    }
#else
    if (Sim_1009eb24() == 0) {
        G32(0x101321a8) = 0xffffffffu;
    } else if (Cd_GetStatus() == 1) {
        G32(0x101321a8) = 0xffffffffu;
    } else if (Sim_1009eb2e((int32_t)G32(0x101321a8)) == 0) {
        G32(0x101321a8) = 0xffffffffu;
    } else if ((G32(0x101321c0) & 8) != 0) {
        Cd_PlayTrack((int)G32(0x101321a8));
        if (Cd_UpdateStatus() != 3)
            G32(0x101321a8) = 0xffffffffu;
    }
    G32(0x101321ac) = 0xffffffffu;
#endif
}

// FUNCTION: HGSIM 0x1001fb6d
void __cdecl Sim_1001fb6d(void)
{
    int go = 0;
    int done = 0;

    if (G32(0x1015d528) == 0)
        return;
    if (U32(G32(0x1015d528), 0x2c) != 0) {
        if (S32(G32(0x1015d528), 0x2c) > GS32(0x1015dc78))
            go = 1;
    } else if (G32(0x1015d52c) == 0) {
        go = 1;
    }
    if (!go)
        return;
    do {
        U32(G32(0x1015d528), 0) = 0;
        G32(0x1015d528) = U32(G32(0x1015d528), 0x67);
        if (G32(0x1015d528) != 0 && W6D_CALL_1001fc18(G32(0x1015d528)) >= 0)
            done = 1;
    } while (G32(0x1015d528) != 0 && !done);
}

// FUNCTION: HGSIM 0x1001fe29
int __cdecl Sim_1001fe29(void)
{
    uint32_t block = U32(PTRTAB(G32(0x1017b8c8)), 0x28);
    int index;

    if ((G32(0x101321c0) & 2) != 0) {
        index = Sim_1000df9c(SIM_PTR(const char, 0x10133968u + U32(block, 0x613) * 0x60u));
        return W6D_CALL_1000e737_1001(0, 0, (uint32_t)index, 0, 0, G32(0x101321b8), 0, 0x5622, 0xffffffffu, 0x250, 1, 0);
    }
    index = Sim_1000df9c(SIM_PTR(const char, 0x10133950u + U32(block, 0x613) * 0x60u));
    return W6D_CALL_1000e737_1001(0, 0, (uint32_t)index, 0, 0, 0x10000, 0, 0x89d, 0xffffffffu, 0x250, 1, 0);
}

// FUNCTION: HGSIM 0x1001fee5
void __cdecl Sim_1001fee5(int flag)
{
    uint32_t prev;
    uint32_t cur;

    if (G32(0x1015d528) == 0 || G32(0x1015d530) != 0)
        return;
    /* the original tests the address of the field +4, which is never 0 for a real element */
    if (G32(0x1015d528) + 4 != 0 && S32(G32(0x1015d528), 0x30) < 0x50) {
        G32(0x1015d528) = Sim_1001ffc8((void *)(uintptr_t)G32(0x1015d528));
        if (G32(0x1015d528) == 0)
            return;
    }
    prev = G32(0x1015d528);
    cur = U32(G32(0x1015d528), 0x67);
    while (cur != 0) {
        int drop = 1;

        if (flag != 0) {
            if (U32(cur, 0x24) != 0)
                drop = 0;
            else if (cur + 4 != 0 && S32(cur, 0x30) >= 0x50)
                drop = 0;
        }
        if (drop) {
            U32(prev, 0x67) = Sim_1001ffc8((void *)(uintptr_t)cur);
            cur = U32(prev, 0x67);
        } else {
            prev = cur;
            cur = U32(cur, 0x67);
        }
    }
    G32(0x1015d530) = 1;
}
