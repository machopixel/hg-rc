#include "w10.h"
#ifdef _MSC_VER
#include <intrin.h>
#endif
#include "../simmem.h"
#include "../util/access.h"
#include "../util/util.h"
#include "w5a.h"
#include "w6d.h"
#include "../unsorted_1004/fns.h"
#include "../unsorted_1005/w5b.h"
#include "../unsorted_1008/w7n.h"
#include "../unsorted_100c/w7p.h"
#include "../unsorted_100f/w2e.h"
#include "../unsorted_100f/w6i.h"
#include "../unsorted_100f/w8z.h"
#include "w5y.h"
#include "../render/w2n.h"
#include "../unsorted_1003/w2e.h"
#include "../libm/hg_math.h"

typedef double(__cdecl *W10_1SqrtFn)(double x);

/* the original's frame bytes ebp-k (k <= 0xc4) that hold the log line's strings: the sprintf output at ebp-0xa0 may run
 * on into the words above it as in the original */
#define W10_1FR(k) (frame + 0xc4 - (k))

// FUNCTION: HGSIM 0x1001484e
void __cdecl Sim_1001484e(uint8_t *obj, uint32_t order)
{
    uint8_t frame[0xc4];
    uint32_t tgt, me, side, p, unit, cc, c8;
    int16_t at, z;
    int32_t zone;
    float dx, dy, dz, dist, ax, ay, az, sign, zero = 0.0f;

    if ((S16(obj, 0x1c) & 0x10) != 0) {
        U16(obj, 0x1c) &= 0xffef;
        U32(U32(obj, 0x28), 0x4f0) = 0;
    }
    if ((order & 0xffff) != 3 && U32(obj, 8) == U32(PTRTAB(G32(0x1017b8c8)), 8))
        U32(obj, 0x1ae) &= 3;
    switch (order & 0xffff) {
    case 0:
        U32(U32(obj, 0x54), 8) = 0;
        U32(obj, 0xd4) = 0;
        break;
    case 2:
        U32(obj, 0x1be) = G32(0x1015dc78) + (uint32_t)Sim_1009c929(10) * 0x16u;
        if ((uint32_t)Sim_10052aaf(S32(obj, 8)) != G32(0x1017b8c8) && S16(obj, 0x1a6) == 7 &&
            (U32(obj, 0x1d6) & 0x1000) != 0) {
            zone = Sim_100ca350(U32(obj, 4), U32(obj, 0x58), U32(obj, 0x5c), U32(obj, 0x60));
            if (zone != -1) {
                *SIM_PTR(u16a, 0x109a8664 + (uint32_t)zone * 0x54u) |= 1;
                G32(0x109a8648 + (uint32_t)zone * 0x54u) = U32(obj, 4) | 0x200u;
                U32(obj, 0x1d6) = (uint32_t)zone | 0x100u;
            }
        }
        break;
    case 3:
        U32(U32(obj, 0x54), 8) = 0;
        break;
    case 4:
        U8(U32(obj, 0x54), 0x59) = 0;
        U32(obj, 0x1ae) = 1;
        break;
    case 5:
        Sim_10015a63(obj);
        break;
    case 7:
        Sim_100157ca(obj);
        break;
    case 8:
        side = U32(obj, 8);
        p = side * 0x4fc3au + G32(0x104727a0 + side * 4u) * 0x13fu + 0x1047383au;
        if (U32(SIM_PTR(uint8_t, p), 2) == 0x100 && S32(SIM_PTR(uint8_t, p), 0x6f) > 0 && U32(obj, 0) == 1) {
            U8(U32(obj, 0x54), 0x56) = 0;
            U32(obj, 0x224) = 0x100;
            U32(obj, 0x228) = 1;
        }
        break;
    case 11:
        U32(obj, 0x1ae) = 1;
        /* fall through */
    case 10:
        U16(obj, 0x1c) |= 0x10;
        U32(U32(obj, 0x28), 0x4f0) |= 3;
        U32(U32(obj, 0x54), 8) = 0;
        U32(obj, 0xd4) = 0;
        break;
    case 13:
        at = S16(obj, 0x1aa);
        if (at != 0 && (at & 0x200) != 0) {
            tgt = PTRTAB((uint32_t)(int32_t)at & 0xff);
            me = PTRTAB(Sim_10052aaf(S32(obj, 8)));
            dx = (float)((double)*(float *)BYTES(me + 0x58) - *(float *)BYTES(tgt + 0x58));
            dy = (float)((double)*(float *)BYTES(me + 0x5c) - *(float *)BYTES(tgt + 0x5c));
            dz = (float)((double)*(float *)BYTES(me + 0x60) - *(float *)BYTES(tgt + 0x60));
            dist = (float)hg_sqrt((double)dx * dx + (double)dy * dy + (double)dz * dz);
            sign = Sim_1009c929(2) != 0 ? 1.0f : -1.0f;
            /* the cross product with (sign, 0, 0) as the original computes it */
            ax = (float)((double)dz * sign - (double)dy * zero);
            ay = (float)((double)dx * zero - (double)dz * zero);
            az = (float)((double)dy * zero - (double)dx * sign);
            Sim_10084cee(&ax, &ay, &az);
            ax = (float)((double)ax * dist);
            ay = (float)((double)ay * dist);
            az = (float)((double)az * dist);
            ax = (float)((double)*(float *)BYTES(tgt + 0x58) + ax);
            ay = (float)((double)*(float *)BYTES(tgt + 0x5c) + ay);
            az = (float)((double)*(float *)BYTES(tgt + 0x60) + az);
            z = (int16_t)Sim_100ca350(U32(obj, 4), *(uint32_t *)&ax, *(uint32_t *)&ay, *(uint32_t *)&az);
            if (z != -1)
                U16(obj, 0x1a8) = (uint16_t)(z | 0x100);
        }
        U32(U32(obj, 0x54), 8) = 0x3c800000;
        break;
    case 14:
        U32(U32(obj, 0x54), 8) = 0x3c800000;
        break;
    case 15:
        U32(obj, 0x23a) = 0;
        U32(obj, 0x228) = 0x80000000u;
        U32(obj, 0x23a) = (uint32_t)Sim_100c4035(obj, (void *)(uintptr_t)PTRTAB(U16(obj, 0x1aa) & 0xff), 0x46ea6000u);
        break;
    default:     /* 1, 6, 9, 12 */
        break;
    }
    U16(obj, 0x1a6) = (uint16_t)order;
    /* the AI log line "<time> : <side> Mech <id> : state <name> <kind> <n>, objective <kind> <goal>" */
    U32(W10_1FR(0xc4), 0) = G32(0x10131314);     /* "mech" */
    *W10_1FR(0xc0) = *GBYTES(0x10131318);
    U16(W10_1FR(0xb8), 0) = *SIM_PTR(u16a, 0x1013131c);     /* "gt" */
    *W10_1FR(0xb6) = *GBYTES(0x1013131e);
    U32(W10_1FR(0x50), 0) = G32(0x10131320);     /* "nav" */
    unit = U32(U32(obj, 0x28), 0);
    cc = (uint32_t)(uintptr_t)((U16(unit, 0x1a8) & 0x200) != 0 ? W10_1FR(0xc4) : W10_1FR(0xb8));
    if ((U16(unit, 0x1a8) & 0x100) != 0)
        cc = (uint32_t)(uintptr_t)W10_1FR(0x50);
    c8 = (uint32_t)(uintptr_t)((U16(unit, 0x1aa) & 0x200) != 0 ? W10_1FR(0xc4) : W10_1FR(0xb8));
    if ((U16(unit, 0x1aa) & 0x100) != 0)
        c8 = (uint32_t)(uintptr_t)W10_1FR(0x50);
    if ((U16(unit, 0x1aa) & 0x6000) != 0) {
        Str_Copy((char *)W10_1FR(0xb4),
                 (const char *)(uintptr_t)Sim_10011b9e((short)U16(unit, 0x1aa), SIM_PTR(void, 0x10130560), 7));
        c8 += Str_Length((const char *)(uintptr_t)c8);
    } else {
        Text_IntToRadix(U16(unit, 0x1aa) & 0xff, (char *)W10_1FR(0xb4), 10);
    }
    Crt_SPrintf((char *)W10_1FR(0xa0), SIM_PTR(const char, 0x10131324), G32(0x1015dc78), U32(unit, 8), U32(unit, 4),
                G32(0x10130458 + (uint32_t)(int32_t)S16(unit, 0x1a6) * 8u), (const char *)(uintptr_t)cc,
                U16(unit, 0x1a8) & 0xff, (const char *)(uintptr_t)c8, (const char *)W10_1FR(0xb4));
    Sim_1005f9aa((const char *)W10_1FR(0xa0));
}


/* the original's frame from ebp-0x378: the log line (0x6c bytes), the table line at ebp-0x30c (0x68 bytes) and the copy
 * of the unit at ebp-0x2a4 (0x2a0 bytes), one block so that a long line runs on as in the original */
#define W10_1LOG (block)
#define W10_1LINE (block + 0x6c)
#define W10_1COPY (block + 0xd4)

// FUNCTION: HGSIM 0x10011c28
void __cdecl Sim_10011c28(void)
{
    uint8_t block[0x6c + 0x68 + 0x2a0];
    uint32_t unit, rec, ctl, p, side, colour;
    const char *aim, *mode, *order, *n1, *n2, *n3, *n4, *n5;
    int32_t i, base, d4, c8;
    uint16_t want;
    char mark, fire;

    /* (a debug branch the original can never take, for a [ebp-0x314] it sets to 0, is left out) */
    if (G32(0x10a3ade8) != 0)
        Mono_PutText(U8(G32(0x1017b8c4), 1) != 0 ? SIM_PTR(const char, 0x10130ff4) : SIM_PTR(const char, 0x10130ff8), 0,
                     0, 0x78);
    if (G32(0x10a3ade8) != 0)
        Mono_PutText(SIM_PTR(const char, 0x10130ffc), 0, 3, 0x78);
    for (i = 0; i < GS32(0x1015dc64); i++) {
        mark = 0x20;
        unit = PTRTAB(i);
        side = U32(unit, 8);
        p = side * 0x4fc3au + G32(0x104727a0 + side * 4u) * 0x13fu + 0x1047383au;
        if (GS32(0x104727a0 + side * 4u) == -1 || S32(SIM_PTR(uint8_t, p), 0x6f) < 0)
            base = 0;
        else
            base = (int32_t)(U32(SIM_PTR(uint8_t, p), 0x6f) - (G32(0x1037305c) - U32(SIM_PTR(uint8_t, p), 0x67)));
        want = U32(unit, 4) == G32(0x1017b8c8) ? (uint16_t)U32(unit, 0xf0) : U16(unit, 0x1a8);
        Mem_Move(W10_1COPY, (const void *)(uintptr_t)unit, 0x2a0);
        Sim_1001403a((void *)(uintptr_t)unit, (short)want);
        if (!(*(float *)BYTES(unit + 0xd4) >= *SIM_PTR(float, 0x10111110))) {
#ifdef _MSC_VER
            __debugbreak();
#else
            __asm__ volatile("int3");
#endif
        }
        ctl = U32(unit, 0x54);
        if (U8(ctl, 0x1e) != 0) {
            if (U8(ctl, 0x1f) != 0)
                mark = 0x3c;
            else if (U8(ctl, 0x20) != 0)
                mark = 0x3e;
            else if (U8(ctl, 0x21) != 0)
                mark = 0x5e;
            else if (U8(ctl, 0x22) != 0)
                mark = 0x76;
            else
                mark = 0x2b;
            if ((int8_t)U8(ctl, 0x1f) + (int8_t)U8(ctl, 0x20) + (int8_t)U8(ctl, 0x21) + (int8_t)U8(ctl, 0x22) > 1)
                mark = 0x3f;
        }
        if (U32(unit, 0x24a) != 0)
            aim = (const char *)(uintptr_t)Sim_10011b9e((short)(U16(U32(unit, 0x24a), 2) & 0xf00),
                                                        SIM_PTR(void, 0x101305c8), 3);
        else
            aim = SIM_PTR(const char, 0x10131050);
        fire = (U8(U32(unit, 0x54), 0x26) != 0 || U8(U32(unit, 0x54), 0x28) != 0) ? 0x2a : 0x20;
        if (S16(unit, 0x1a6) == 3)
            mode = (const char *)(uintptr_t)Sim_10011b9e((short)U16(unit, 0x1f8), SIM_PTR(void, 0x10130710), 0x1d);
        else
            mode = SIM_PTR(const char, 0x10131054);
        rec = U32(unit, 0x28);
        n5 = (const char *)(uintptr_t)Sim_10011b9e((short)U16(rec, 0x4f0), SIM_PTR(void, 0x10130638), 8);
        c8 = Render_Ftol((double)*(float *)BYTES(U32(unit, 0x54) + 8));
        d4 = Render_Ftol((double)*(float *)BYTES(unit + 0xd4));
        n4 = (const char *)(uintptr_t)Sim_10011b9e((short)(U16(unit, 0x1aa) & 0xf00), SIM_PTR(void, 0x101305b0), 3);
        n3 = (const char *)(uintptr_t)Sim_10011b9e((short)((int32_t)(int16_t)want & 0xf00), SIM_PTR(void, 0x101305b0), 3);
        n2 = (const char *)(uintptr_t)Sim_10011b9e((short)U16(unit, 0x1a6), SIM_PTR(void, 0x101304d8), 0x11);
        n1 = (const char *)(uintptr_t)Sim_10011b9e((short)U16(SIM_PTR(uint8_t, p), 2), SIM_PTR(void, 0x10130678), 0x13);
        order = (const char *)(unit + 0x124);
        Crt_SPrintf((char *)W10_1LINE, SIM_PTR(const char, 0x1013105c), unit != G32(0x1015ef14) ? 0x20 : 0x3e,
                    U32(unit, 4), U32(unit, 8), G32(0x104727a0 + U32(unit, 8) * 4u), n1, base, n2, n3,
                    (int32_t)(int16_t)want & 0xff, n4, U16(unit, 0x1aa) & 0xff, mode, d4, c8, n5,
                    (double)*(float *)BYTES(U32(rec, 0x88) + 0x44) * *SIM_PTR(float, 0x10111114),
                    (double)*(float *)BYTES(U32(unit, 0x54) + 0xc), order, (int)mark, fire, aim);
        colour = 7;
        if ((U32(U32(unit, 0x28), 0x4f0) & 4) == 0)
            colour |= 8;
        if (S32(unit, 4) + 4 < 0x19 && G32(0x10a3ade8) != 0)
            Mono_PutText((const char *)W10_1LINE, 0, (short)(U32(unit, 4) + 4), (short)colour);
        if (U16(unit, 0x1aa) == 0 || U16(unit, 0x1a8) == 0) {
            *SIM_PTR(u16a, 0x1015d10c) += 1;
            if (*SIM_PTR(s16a, 0x1015d10c) < 10) {
                Crt_SPrintf((char *)W10_1LOG, SIM_PTR(const char, 0x101310d4), G32(0x1015dc78), U32(unit, 8),
                            U32(unit, 4));
                Sim_1005f9aa((const char *)W10_1LOG);
            }
        }
        Mem_Move(BYTES(unit + 0xd4), W10_1COPY + 0xd4, 0x28);
    }
}