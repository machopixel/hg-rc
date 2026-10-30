#include "w8y.h"
#include "w3t.h"
#include "w5a.h"
#include "w5x.h"
#include "w5y.h"
#include "w5z.h"
#include "w6g.h"
#include "../util/access.h"
#include "../util/util.h"
#include "../util/w3o.h"
#include "../unsorted_1000/w2g.h"
#include "../unsorted_1000/w2j.h"
#include "../unsorted_1000/w4w.h"
#include "../unsorted_1000/w5y.h"
#include "../unsorted_1002/w5b.h"
#include "../unsorted_1002/w7l.h"
#include "../unsorted_1004/w3s.h"
#include "../unsorted_1006/w6e.h"
#include "../unsorted_1007/w2g.h"

/*
 * Floating point as in util/w3o.h: double intermediates (the x87 runs with a 53-bit mantissa), float where the
 * original stores a float.
 */

/* The block a control record (+0) points at, read again at every use as the original does. */
#define BODY(ctl) U32(ctl, 0)
#define BODY_STATE(ctl) U32(BODY(ctl), 0x54)

// FUNCTION: HGSIM 0x100a89f0
void __cdecl Sim_100a89f0(void *obj)
{
    union { double d; uint64_t u; } fifth;
    uint32_t c = U32(obj, 0x28);

    if (c == 0)
        return;
    if (U32(BODY(c), 4) == G32(0x1017b8c8))
        W8Y_CALL_1002bca0();
    U32(c, 0x53c) = U32(U32(U32(BODY(c), 0x48), 0x8c), 0x6c);
    fifth.u = 0x3fc99999a0000000ull;      /* the double of the float 0.2 */
    Sim_10047b76(BYTES(c) + 4, 0, 0, fifth.d);
    Sim_10047b76(BYTES(c) + 0x14, 0, 0, fifth.d);
    U32(c, 0x4fc) = 0;
    U32(c, 0x4e8) = 0;
    U32(c, 0x508) = 0;
    U32(c, 0x4f4) = 0;
    U16(c, 0x560) = 0x2000;
    U32(c, 0x4f0) = 0;
    U32(c, 0x4d8) = 0;
    U32(c, 0x4e0) = 0;
    U32(c, 0x504) = 0;
    U32(c, 0x50c) = 0;
    U32(c, 0x54c) = 0;
    U32(c, 0x544) = 0;
    Sim_100a9d7c((void *)(uintptr_t)U32(BODY(c), 0x48), 0.0f, Float_OfBits(U32(c, 0x520)), 0.0f);
    Sim_100aa799((void *)(uintptr_t)U32(BODY(c), 0x48));
    Sim_100a9ca3((void *)(uintptr_t)U32(BODY(c), 0x48), BODY(c) + 0x64, BODY(c) + 0x68, BODY(c) + 0x6c);
    Sim_100a9cfb((void *)(uintptr_t)U32(BODY(c), 0x48), (uint32_t *)(uintptr_t)(BODY(c) + 0x58),
                 (uint32_t *)(uintptr_t)(BODY(c) + 0x5c), (uint32_t *)(uintptr_t)(BODY(c) + 0x60));
    U32(BODY(c), 0x94) = 0;
    U32(BODY(c), 0x90) = 0;
    U32(BODY(c), 0x154) = (uint32_t)Sim_100ac21a(1, (void *)(uintptr_t)BODY(c));
    Sim_100aa454((void *)(uintptr_t)U32(BODY(c), 0x48));
    U8(BODY_STATE(c), 0x46) = 0;
    U32(BODY_STATE(c), 8) = 0;
    U8(BODY_STATE(c), 0x31) = 0;
    W8Y_CALL_10011093(BODY(c));
    U32(c, 0x4d4) = 0;
}

// FUNCTION: HGSIM 0x100a8d09
void __cdecl Sim_100a8d09(void *ctl)
{
    uint32_t heading;
    float d;

    W8Y_CALL_10011681(BODY(ctl));
    if (U32(ctl, 0x4f0) == 2) {
        W8Y_CALL_100d4590(ctl);
        if (S32(BODY(ctl), 0x14) == 2 || (S32(BODY(ctl), 0x14) == 3 && U32(BODY(ctl), 0xc) == G32(0x1017b8c8))) {
            U32(BODY(ctl), 0xf0) = 0;
        } else if (U32(BODY(ctl), 0xf0) != 0 && (U32(BODY(ctl), 0xf0) & 0x1000) == 0
                   && W8Y_CALL_100cb4fb(BODY(ctl)) == 0) {
            U32(BODY(ctl), 0xf0) = 0;
        }
        W8Y_CALL_100d6833(ctl);
        U32(ctl, 0x50c) = 0;
        U8(BODY_STATE(ctl), 0x46) = 0;
        if ((U16(ctl, 0x560) & 4) != 0 && (U16(ctl, 0x560) & 8) == 0)
            U32(BODY_STATE(ctl), 8) = 0;
        heading = U32(BODY_STATE(ctl), 4);
        if (FL(ctl, 0x534) >= GFL(0x1012ba8c)) {
            /* the turn of the heading from the last one (+0xc), brought into -180..180 */
            d = (float)((double)Float_OfBits(heading) - FL(ctl, 0xc));
            while (d > GFL(0x1012ba90))
                d = (float)((double)d - GFL(0x1012ba8c));
            while (!(d >= GFL(0x1012ba94)))
                d = (float)((double)d + GFL(0x1012ba8c));
            U32(ctl, 8) = heading;
            FL(ctl, 0xc) = (float)((double)Float_OfBits(heading) - d);
        } else if (FL(ctl, 8) > FL(ctl, 0x534)) {
            U32(ctl, 8) = U32(ctl, 0x534);
        } else if (-FL(ctl, 0x534) > FL(ctl, 8)) {
            FL(ctl, 8) = -FL(ctl, 0x534);
        }
        U32(ctl, 0x18) = U32(BODY_STATE(ctl), 0);
    } else {
        U32(ctl, 8) = 0;
        U32(ctl, 0x18) = 0;
        U32(BODY_STATE(ctl), 8) = 0;
        U32(BODY_STATE(ctl), 4) = 0;
        U32(BODY_STATE(ctl), 0) = 0;
    }
    switch (U32(ctl, 0x4f0)) {
    case 0:
        U32(ctl, 0x4f0) = 1;
        U32(ctl, 0x4d8) = G32(0x1015dc78) + (uint32_t)Sim_1009c929(0x16a) + 0x43e;
        break;
    case 1:
        if (GS32(0x1015dc78) > S32(ctl, 0x4d8))
            U32(ctl, 0x4f0) = 2;
        break;
    case 3:
        U32(ctl, 0x4f0) = 0;
        break;
    case 4:
        if (U32(ctl, 0x4d8) == 0) {
            U32(ctl, 0x4d8) = G32(0x1015dc78) + 0x712;
            Sim_1007cbb4();
        }
        if (GS32(0x1015dc78) < S32(ctl, 0x4d8))
            Sim_10061c32((uint32_t *)ctl);
        break;
    default:
        break;
    }
}

// FUNCTION: HGSIM 0x100ab054
void __cdecl Sim_100ab054(void *node, uint32_t key)
{
    if (node == 0)
        return;
    Sim_100ab054((void *)(uintptr_t)U32(node, 8), key);
    if (U32(node, 0x8c) != 0 && U16(U32(node, 0x8c), 0x26) == key) {
        Sim_10028f38((void *)(uintptr_t)U32(node, 4), (int)(uintptr_t)SIM_PTR(void, 0x100020e5), (int)key);
        Sim_10028d9f(node, SIM_FN(void(__cdecl *)(void *), 0x100020e5), (int)key);
        return;
    }
    Sim_100ab054((void *)(uintptr_t)U32(node, 4), key);
}

// FUNCTION: HGSIM 0x100abcb3
void __cdecl Sim_100abcb3(void)
{
    uint32_t extra;
    uint8_t *obj;
    uint8_t *poly;
    uint8_t *node;
    uint16_t id;
    uint16_t i;
    int full = 0;

    G32(0x1017d1d4) = 0;
    G32(0x1017d1d0) = G32(0x1017d1d4);
    G32(0x1017d1dc) = 0;
    G32(0x1017d1d8) = G32(0x1017d1dc);
    G32(0x1017d1cc) = 0;
    for (i = 0; i < 0x40 && full == 0; i++) {
        obj = (uint8_t *)Sim_10008f1c(4, 1, 0x10, &extra);
        if (obj == 0) {
            full = 1;
            continue;
        }
        G32(0x1017d1c8) = G32(0x1017d1c8) + 1;
        id = (uint16_t)(i | 0x9000);
        /* the four corners (-1000, 0, 1000), (-1000, 0, -1000), (1000, 0, -1000), (1000, 0, 1000) */
        Sim_10009136(obj, 0xc47a0000u, 0, 0x447a0000u, 0, 0, 0, 0, 0, 0, 0x3f800000u, 0);
        Sim_10009136(obj, 0xc47a0000u, 0, 0xc47a0000u, 0, 0, 0, 0, 0, 0, 0x3f800000u, 0);
        Sim_10009136(obj, 0x447a0000u, 0, 0xc47a0000u, 0, 0, 0, 0, 0, 0, 0x3f800000u, 0);
        Sim_10009136(obj, 0x447a0000u, 0, 0x447a0000u, 0, 0, 0, 0, 0, 0, 0x3f800000u, 0);
        poly = (uint8_t *)Sim_100091f7(obj, id, extra);
        U32(poly, 0x1c) = 0x3f800000u;
        U32(poly, 0x10) = 0x3f800000u;
        U32(poly, 0x18) = 0;
        U32(poly, 0xc) = 0;
        U32(poly, 0x20) = 0;
        U32(poly, 0x14) = 0;
        Sim_10009278(obj, poly, 0);
        Sim_10009278(obj, poly, 1);
        Sim_10009278(obj, poly, 2);
        Sim_10009278(obj, poly, 3);
        node = (uint8_t *)Sim_100a9830(0, 0xc);
        Sim_100a9b4c(node, (uint32_t)(uintptr_t)obj);
        Sim_100096f9(obj, (uint32_t)(uintptr_t)node);
        Sim_10009408(obj, 0);
        Sim_100094ba(obj);
        Sim_100a9d47(node, 0, 0, 0);
        Sim_100aa742(node);
        W8Y_CALL_10089e91((uint32_t)(uintptr_t)obj);
        Sim_100aa351(node);
        Sim_100aa406(node);
        Sim_1000971b(obj, 0);
        G32(0x101c56c0 + (uint32_t)i * 4) = i;
        G32(0x101c57e0 + (uint32_t)i * 0x34) = i;
        G32(0x101c5810 + (uint32_t)i * 0x34) = (uint32_t)(uintptr_t)node;
        G32(0x101c57ec + (uint32_t)i * 0x34) = 0;
        G32(0x101c57f0 + (uint32_t)i * 0x34) = 0;
        G32(0x101c57e8 + (uint32_t)i * 0x34) = 0;
        G32(0x101c57e4 + (uint32_t)i * 0x34) = 0;
    }
    G32(0x1017d1c4) = G32(0x1017d1c8) - 1;
}
