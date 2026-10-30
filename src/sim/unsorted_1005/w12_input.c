#include "w12.h"
#include "../simmem.h"
#include "../util/access.h"
#include "../util/w3o.h"
#include "../render/w2n.h"
#include "../unsorted_1003/w4u.h"
#include "../unsorted_1005/w2h.h"
#include "../unsorted_1005/w3r.h"

/* The device methods: +0x10 (state, arg), +0x14 (state, buffer, buffer), +0x18 (&key word). */
typedef void(__cdecl *W12DevPollFn)(uint32_t state, uint32_t arg);
typedef void(__cdecl *W12DevReadFn)(uint32_t state, uint32_t a, uint32_t b);
typedef void(__cdecl *W12DevKeyFn)(int16_t *key);
#define W12_DEV_OPEN(d) G32(0x1015f868 + (uint32_t)(d) * 4)
#define W12_DEV_METHOD(d, off) U32(G32(0x1015fd20 + (uint32_t)(d) * 4), off)
#define W12_DEV_STATE(d) G32(0x1015f8f0 + (uint32_t)(d) * 0x74)

// FUNCTION: HGSIM 0x10055048
void __cdecl Sim_10055048(void)
{
    int16_t key = 0; /* start value (DECISIONS.md): no key without an open keyboard */
    int32_t i;

    if (G32(0x1015ec58) == 0 || (G32(0x1017b94c) & 1) != 0) {
        for (i = 0; i < (int32_t)G32(0x1015fde8); i++) {
            uint32_t r = 0x1015fe00 + (uint32_t)i * 0x80;
            uint32_t a = U32(r, 0x14);

            if (U32(a, 0x2a) != 0 && (int8_t)U8(U32(a, 0x2a), 0) != 0) {
                if (U32(r, 8) != 0) {
                    uint32_t d = U32(r, 0);

                    if (W12_DEV_OPEN(d) != 0)
                        ((W12DevPollFn)(uintptr_t)W12_DEV_METHOD(d, 0x10))(W12_DEV_STATE(d), U32(r, 4));
                }
                U32(a, 0x1a) = U32(a, 0x10);
                U32(U32(a, 4), 0) = 0;
                U32(r, 0xc) = 0;
            } else if (U32(a, 0x2e) != 0 && (int8_t)U8(U32(a, 0x2e), 0) != 0) {
                uint32_t raw = (uint32_t)Render_Ftol((double)FL(U32(a, 0), 0) * (double)GFL(0x1012b458) +
                                                     (double)GFL(0x1012b45c));
                double t = (double)(int32_t)(raw << ((uint32_t)(int32_t)(int8_t)U8(a, 0x14) & 31));

                t = t - (double)S32(a, 0xc);
                t = t * *SIM_PTR(const double, 0x1012b460);
                t = t / (double)S32(a, 8);
                t = t - *SIM_PTR(const double, 0x1012b468);
                U32(U32(a, 4), 0) = 0;
                S32(a, 0x1a) = Render_Ftol(t);
            }
            U8(a, 0x15) = 0;
        }
        for (i = 0; i < 0x6d; i++) {
            if (G32(0x1013f168 + (uint32_t)i * 32) != 0)
                U8(G32(0x1013f164 + (uint32_t)i * 32), 0) = 0;
        }
        for (i = 0; i < (int32_t)G32(0x10164bc0); i++) {
            if (W12_DEV_OPEN(i) != 0 && ((int32_t)G32(0x10151d50) == -2 || (uint32_t)i != G32(0x1015fdf4)))
                ((W12DevReadFn)(uintptr_t)W12_DEV_METHOD(i, 0x14))(W12_DEV_STATE(i), 0x1015fac8 + (uint32_t)i * 0x50,
                                                                   0x1015fb08 + (uint32_t)i * 0x50);
        }
        for (i = 0; i < (int32_t)G32(0x1015fdf0); i++) {
            uint32_t b = 0x101620c8 + (uint32_t)i * 0x6e;

            if (G32(0x1013f140) != 0 || U32(b, 0xa) != 0) {
                uint8_t v = (uint8_t)Sim_1005436d(S32(b, 6), (const uint32_t *)(uintptr_t)(b + 0xe));

                if ((int8_t)U8(b, 4) == 0 || (int8_t)U8(b, 5) == 0)
                    U8(U32(b, 0), 0) = (uint8_t)(U8(U32(b, 0), 0) | v);
                U8(b, 5) = v;
            }
        }
        for (i = 0; i < (int32_t)G32(0x1015fde8); i++) {
            uint32_t r = 0x1015fe00 + (uint32_t)i * 0x80;
            uint32_t a;
            int32_t w;

            if (G32(0x1013f140) == 0 && U32(r, 0x7c) == 0)
                continue;
            if (Sim_10054d98((void *)(uintptr_t)r) == 0 && U32(r, 8) != 0 &&
                Sim_1005436d(S32(r, 0x18), (const uint32_t *)(uintptr_t)(r + 0x1c)) != 0) {
                int32_t v = (int32_t)(U32(U32(r, 8), 0) * U32(r, 0x10));

                a = U32(r, 0x14);
                if (U32(r, 0) == G32(0x1013f144) && U32(G32(0x1015d1b8), 0x64) == 0 &&
                    (U32(r, 4) == G32(0x1013f148) || U32(r, 4) == G32(0x1013f14c))) {
                    if (S32(r, 0xc) != v) {
                        if (S32(a, 0x1a) >= 0) {
                            if (v > S32(a, 0x1a))
                                S32(a, 0x1a) = v;
                            else if (v < 0)
                                U32(a, 0x1a) = U32(a, 0x1a) + (uint32_t)v;
                            else if (S32(a, 0x1a) == S32(U32(a, 4), 0))
                                S32(a, 0x1a) = v;
                        } else {
                            if (v < S32(a, 0x1a))
                                S32(a, 0x1a) = v;
                            else if (v >= 0)
                                U32(a, 0x1a) = U32(a, 0x1a) + (uint32_t)v;
                            else if (S32(a, 0x1a) == S32(U32(a, 4), 0))
                                S32(a, 0x1a) = v;
                        }
                        S32(r, 0xc) = v;
                    }
                } else if (S32(r, 0xc) != v) {
                    S32(a, 0x1a) = v;
                } else if (S32(a, 0x1a) == S32(U32(a, 4), 0)) {
                    S32(a, 0x1a) = v;
                }
                S32(r, 0xc) = v;
            }
            a = U32(r, 0x14);
            w = S32(a, 0x1a);
            if (w > 0x10000)
                w = 0x10000;
            if (w < -0x10000)
                w = -0x10000;
            w = (int32_t)((uint32_t)(w + 0x10000) * U32(a, 8));
            w = w / 2;
            w = (int32_t)((uint32_t)w + U32(a, 0xc));
            w = w >> ((uint32_t)(int32_t)(int8_t)U8(a, 0x14) & 31);
            FL(U32(a, 0), 0) = (float)((double)w * (double)GFL(0x1012b470));
        }
    }
    if (W12_DEV_OPEN(G32(0x1015fdf4)) != 0)
        ((W12DevKeyFn)(uintptr_t)W12_DEV_METHOD(G32(0x1015fdf4), 0x18))(&key);
    if ((int32_t)key == 0x40d)
        ToggleFullScreen();
    else
        *SIM_PTR(int16_t, 0x1015fd4c) = key;
}