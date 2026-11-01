#include "w7l.h"
#include "w2f.h"
#include "w2i.h"
#include "w2j.h"
#include "w2l.h"
#include "w3o.h"
#include "w6d.h"
#include "../unsorted_1000/w5a.h"
#include "../unsorted_1003/w2b.h"
#include "../unsorted_1004/fns.h"
#include "../unsorted_1006/w5y.h"
#include "../unsorted_1006/w6e.h"
#include "../unsorted_1007/w2e.h"
#include "../unsorted_1007/w2g.h"
#include "../unsorted_100a/w5a.h"
#include "../unsorted_100a/w5b.h"
#include "../unsorted_100a/w5y.h"
#include "../unsorted_100a/w5z.h"
#include "../unsorted_100a/w6g.h"
#include "../unsorted_100c/w2h.h"
#include "../unsorted_100f/w6h.h"
#include "../view/view.h"
#include "../util/util.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x100200d1
// FUNCTION: HGSIM12 0x100173e1
void __cdecl Sim_100200d1(int a, int b)
{
    uint32_t first = 0, second = 0;
    int32_t n;

#ifdef W7LB_MUTATE /* the checker's negative check only: 1.0's banks 0..2 and 4 */
    if (a <= -1 || (a >= 3 && a != 4) || b < 0 || b >= 0xa)
#else
    /* 1.2 (0x100173e1): banks 0..4 (bank 3's sound index [0x10133710] is never set, -1, in 1.0 and 1.2 alike) */
    if (a <= -1 || a > 4 || b < 0 || b >= 0xa)
#endif
        return;
    if (a == 0) {
        n = GS32(0x10133700);
        if (n >= 0) {
            if (b == 0) {
                first = G32(0x101336b0 + 4 * n) + ((uint32_t)b + G32(0x10133704)) * 0x18;
                G32(0x10133704) = G32(0x10133704) == 0;
            } else {
                first = G32(0x101336b0 + 4 * n) + (uint32_t)b * 0x18;
            }
        }
        n = GS32(0x10133708);
        if (n >= 0) {
            if (b == 0) {
                second = G32(0x101336b0 + 4 * n) + ((uint32_t)b + G32(0x1013370c)) * 0x18;
                G32(0x1013370c) = G32(0x1013370c) == 0;
            } else {
                second = G32(0x101336b0 + 4 * n) + (uint32_t)b * 0x18;
            }
        }
    } else {
        n = GS32(0x101336f8 + 8 * a);
        if (n >= 0) {
            if (b == 0) {
                first = G32(0x101336b0 + 4 * n) + ((uint32_t)b + G32(0x101336fc + 8 * a)) * 0x18;
                G32(0x101336fc + 8 * a) = G32(0x101336fc + 8 * a) == 0;
            } else {
                first = G32(0x101336b0 + 4 * n) + (uint32_t)b * 0x18;
            }
        }
        second = 0;
    }
    if (first != 0 || second != 0)
        W7L_CALL_1001f790(first, second, -1);
}

// FUNCTION: HGSIM 0x1002597b
void __cdecl Sim_1002597b(int index)
{
    uint8_t *rec = GBYTES(0x109e9ba0) + index * 0x50;
    uint32_t cur = G32(0x1017e678);

    if (cur != 0 && (U16(cur, 2) & 0x200) != 0 && (uint32_t)U16(cur, 0x24) == (uint32_t)index)
        G32(0x1017e678) = 0;
    if (S16(rec, 0) & 4)
        return;
    if (GS32(0x10134898) >= 0) {
        if (G32(0x10134898) == G32(0x1017b8c8)) {
            switch (Sim_10052c02(index)) {
            case 0:
                *SIM_PTR(u16a, 0x101653f1) += 1;
                break;
            case 2:
                *SIM_PTR(u16a, 0x101653ef) += 1;
                break;
            case 1:
                *SIM_PTR(u16a, 0x101653ed) += 1;
                break;
            }
        }
        if (U32(PTRTAB(G32(0x10134898)), 8) == U32(PTRTAB(G32(0x1017b8c8)), 8)) {
            switch (Sim_10052c02(index)) {
            case 0:
                *SIM_PTR(u16a, 0x1016540a) += 1;
                break;
            case 2:
                *SIM_PTR(u16a, 0x10165408) += 1;
                break;
            case 1:
                *SIM_PTR(u16a, 0x10165406) += 1;
                break;
            }
        }
    }
    if (S16(rec, 0) & 0x40)
        Sim_1007cbb4();
    W7L_CALL_100b45b4(rec);
}

// FUNCTION: HGSIM 0x10025c8c
void __cdecl Sim_10025c8c(uint32_t a, uint32_t b, uint32_t c, int limit)
{
    int started = 0, i;

    if (limit == 0)
        limit = 0x100;
    for (i = 0; i < 0x100 && started < limit; i++) {
        uint8_t *e = GBYTES(0x109a5a40) + i * 0x2c;
        uint32_t obj;
        int slot;

        if (U32(e, 0x18) != 0 || U32(e, 0x24) != 0xb || U32(e, 0) == 0)
            continue;
        obj = U32(e, 0);
        slot = Sim_10028b90(obj, 1);
        if (slot == -1)
            continue;
        U32(e, 0x18) = 1;
        U32(e, 0x14) = G32(0x10147e14);
        Sim_100a9d47(BYTES(obj), a, b, c);
        Sim_10028c6f(slot);
        Sim_100aa39f(BYTES(obj));
        Sim_100aa799(BYTES(obj));
        started++;
    }
}

// FUNCTION: HGSIM 0x10027654
void *__cdecl Sim_10027654(void)
{
    uint32_t n;

    G32(0x109f1484) = (uint32_t)(int32_t)*SIM_PTR(s16a, 0x109f1448) * 0x2fcu;
    G32(0x109f1480) = G32(0x10134f80);
    n = (uint32_t)(int32_t)*SIM_PTR(s16a, 0x109f1448);
    G32(0x109f148c) = n * (uint32_t)Sim_10072ff2();
    G32(0x109f1488) = G32(0x10134f84);
    n = (uint32_t)(int32_t)*SIM_PTR(s16a, 0x109f144c);
    G32(0x109f1494) = n * W7L_CALL_100ab1f2();
    G32(0x109f1490) = G32(0x10134f88);
    n = (uint32_t)(int32_t)*SIM_PTR(s16a, 0x109f1454);
    G32(0x109f149c) = n * (uint32_t)Sim_100cc96c();
    G32(0x109f1498) = G32(0x10134f8c);
    G32(0x109f14a4) = (uint32_t)(int32_t)*SIM_PTR(s16a, 0x109f1454) * 0x2cu;
    G32(0x109f14a0) = G32(0x10134f90);
    G32(0x109f14ac) = (uint32_t)(int32_t)*SIM_PTR(s16a, 0x109f1456) * 0x14u;
    G32(0x109f14a8) = G32(0x10134f94);
    G32(0x109f14b4) = G32(0x109f1458);
    G32(0x109f14b0) = G32(0x10134f98);
    G32(0x109f14bc) = (uint32_t)(int32_t)*SIM_PTR(s16a, 0x109f144e) << 3;
    G32(0x109f14b8) = G32(0x10134f9c);
    G32(0x109f14c4) = (uint32_t)(int32_t)*SIM_PTR(s16a, 0x109f144e) << 2;
    G32(0x109f14c0) = G32(0x10134fa0);
    G32(0x109f14cc) = (uint32_t)(int32_t)*SIM_PTR(s16a, 0x109f144e) << 2;
    G32(0x109f14c8) = G32(0x10134fa4);
    return GBYTES(0x109f1480);
}

// FUNCTION: HGSIM 0x10027fb8
void __cdecl Sim_10027fb8(void *node)
{
    char name[8];
    char text[0x64];
    uint32_t prev, table;
    int32_t bucket;

    if (node == 0)
        return;
    U16(node, 2) = 0;
    Sim_10027cce(node);
    name[4] = 0;
    U32(name, 0) = U32(node, 4);
    bucket = ((int32_t)U16(node, 0) + (int8_t)name[0] + (int8_t)name[1] + (int8_t)name[2] + (int8_t)name[3]) % 0x3f1;
    table = G32(0x1015dc30);
    if ((uint32_t)(uintptr_t)node == U32(table, bucket * 4)) {
        U32(table, bucket * 4) = U32(node, 8);
    } else {
        for (prev = U32(table, bucket * 4); prev != 0; prev = U32(prev, 8)) {
            if (U32(prev, 8) == 0 || U32(prev, 8) == (uint32_t)(uintptr_t)node)
                break;
        }
        if (prev != 0 && U32(prev, 8) != 0) {
            U32(prev, 8) = U32(U32(prev, 8), 8);
        } else {
            W7L_CALL_100bc280(0x20, (const char *)GBYTES(0x10135018), name, (uint32_t)U16(node, 0));
            if ((uint32_t)(uintptr_t)node == G32(0x109f13f0))
                Sim_10027d77();
            if (G32(0x1015f1e8) != 0) {
                W7L_SPRINTF_1002(text, (const char *)GBYTES(0x10135040), name, (uint32_t)U16(node, 0));
                Sim_1007df6c(text, (const char *)(uintptr_t)G32(0x109b98c0), 0x712, 0x64);
            }
            return;
        }
    }
    SIM_API(W7lHeapFreeFn, 0x10ac5634)(G32(0x1017b8f8), 1, node);
    G32(0x109f13e0) = G32(0x109f13e0) - 1;
}

// FUNCTION: HGSIM 0x10028fb0
void __cdecl Sim_10028fb0(void *obj, void(__cdecl *done)(void *))
{
    int key;

    if (obj == 0)
        return;
    if (U32(obj, 0x90) == 0
        || (Crt_StrStr((const char *)BYTES(U32(obj, 0x90)), (const char *)GBYTES(0x101352ec)) == 0
            && Crt_StrStr((const char *)BYTES(U32(obj, 0x90)), (const char *)GBYTES(0x101352f0)) == 0)) {
        W7L_CALL_10029b1f(obj);
        if (done != 0)
            done(obj);
    } else {
        Sim_100aa406(obj);
        Sim_100aa799(obj);
        Sim_10038d65((int)(uintptr_t)obj);
    }
    key = Sim_1002997e((int)(uintptr_t)obj);
    if (key != -1)
        Sim_1002990f(key);
}

// FUNCTION: HGSIM 0x10028d9f
void __cdecl Sim_10028d9f(void *obj, void(__cdecl *done)(void *), int flag)
{
    int now = 0, i, slot;
    uint8_t *e;

    if (obj == 0 || U32(obj, 0x8c) == 0)
        return;
    if (U32(obj, 0x90) != 0 && U32(U32(obj, 0x90), 0) == G32(0x101352e4))
        return;
    if ((U16(U32(obj, 0x8c), 2) & 0xf0) == 0x70 || (flag != 0 && U16(U32(obj, 0x8c), 0x26) == 0)) {
        Sim_10009408(BYTES(U32(obj, 0x8c)), 0x50);
        now = 1;
    }
    for (i = 0; i < 0x80; i++) {
        if (G32(0x109f09e0 + i * 0x14) == 0)
            break;
    }
    if (i == 0x80 || now != 0) {
        Sim_100aa8cb(obj);
        Sim_10028fb0(obj, done);
        return;
    }
    e = GBYTES(0x109f09e0) + i * 0x14;
    slot = Sim_10028b90((uint32_t)(uintptr_t)obj, 2);
    if (slot < 0) {
        Sim_10028fb0(obj, done);
        return;
    }
    U32(e, 0) = 1;
    U32(e, 4) = (uint32_t)(uintptr_t)obj;
    U32(e, 0x10) = (uint32_t)(uintptr_t)done;
    U32(e, 8) = G32(0x1015dc78);
    U32(e, 0xc) = 0x41800000u;          /* 16.0 */
    Sim_100aa5e8(obj, 0x300);
    Sim_100aa4f0(obj, 0x50);
    Sim_100aa541(obj, (uint32_t)i);
    Sim_10028c6f(slot);
}

// FUNCTION: HGSIM 0x1002c76d
void __cdecl Sim_1002c76d(void *obj)
{
    if (G32(0x1015dd14) != 0) {
        int r;

        G32(0x1015dd24) = G32(0x1015dd24) + 1;
        r = Sim_1009c929(5);
        Sim_10061b78(G32(0x1015dd14) + (uint32_t)r);
        G32(0x1015dd14) = 0;
        if (GS32(0x1015dd24) > 4)
            Sim_1006863d((uint32_t *)obj);
    }
    if (G32(0x1015ef0c) != 0) {
        if (U16(obj, 0x560) & 0x80) {
            if (G32(0x1015dd1c) == 0 && U32(U32(obj, 0x84) + U32(obj, 0x4fc) * 0x128, 8) == 1
                && U32(obj, 0x4f0) == 2) {
                G32(0x1015dd1c) = 1;
                Sim_1000ebfb((const char *)GBYTES(0x101382a8), 0, 0x1388, 5, 0x32);
            }
        } else {
            G32(0x1015dd1c) = 0;
            if (U16(obj, 0x560) & 0x40) {
                if (G32(0x1015dd20) == 0 && U32(obj, 0x4f0) == 2) {
                    G32(0x1015dd20) = 1;
                    Sim_1000ebfb((const char *)GBYTES(0x101382b4), 0, 0xffffec78u, 5, 0x32);
                }
            } else {
                G32(0x1015dd20) = 0;
            }
        }
    }
    G32(0x1015dd18) = U32(obj, 0x4f0);
}

// FUNCTION: HGSIM 0x1002ff6e
uint32_t __cdecl Sim_1002ff6e(void *obj)
{
    uint32_t p;

    if (obj == 0)
        return 0;
    p = U32(obj, 0x98);
    if (p == 0)
        return 0;
    switch (U32(obj, 0x34)) {
    case 0:
        return 0x78;
    case 5:
        return W7L_CALL_1003218b(p);
    default:
        return 0;
    }
}
