#include "w7n.h"
#include "w2e.h"
#include "w2f.h"
#include "w3o.h"
#include "w6e.h"
#include "../unsorted_1000/w5a.h"
#include "../unsorted_1002/w2g.h"
#include "../unsorted_1002/w2i.h"
#include "../unsorted_1004/fns.h"
#include "../unsorted_1006/w5z.h"
#include "../unsorted_100d/w2h.h"
#include "../unsorted_100d/w2i.h"
#include "../unsorted_100f/w2e.h"
#include "../unsorted_100f/w6i.h"
#include "../view/view.h"
#include "../view/w2l.h"
#include "../util/w3o.h"

typedef int(__cdecl *W7nCheckFn)(void *entry, void *block);

// FUNCTION: HGSIM 0x10072a6b
void __cdecl Sim_10072a6b(uint8_t *obj)
{
    if (obj == 0)
        return;
    switch (S32(obj, 0x4f0)) {
    case 5:
        if (G32(0x10179d08) == 0) {
            U32(obj, 0x4d8) = G32(0x1015dc78) + 0x389;
            G32(0x10179d08) = G32(0x10179d08) + 1;
        } else if (GS32(0x1015dc78) < S32(obj, 0x4d8)) {
            Sim_100448cd(4);
        } else {
            G32(0x10179cd4) = 1;
        }
        return;
    case 4:
        if (G32(0x10179d04) == 0) {
            G32(0x10179d04) = 1;
            Sim_10061b62();
            G32(0x10179cd4) = 1;
            Sim_100448cd(1);
        }
        return;
    case 2:
        if (G32(0x10179ccc) != 0) {
            if (U32(obj, 0x504) == 0) {
                Sim_1000ebfb(SIM_PTR(char, 0x101454b8), 0, 0, 5, 0x32);
                Sim_100d5de5(obj);
            }
            G32(0x10179ccc) = 0;
        }
        if ((int8_t)U8(U32(U32(obj, 0), 0x54), 0x27) != 0 && U32(obj, 0x504) == 0) {
            Sim_1000ebfb(SIM_PTR(char, 0x101454c4), 0, 0, 5, 0x32);
            G32(0x10179ccc) = 1;
            Sim_100d5de5(obj);
            G32(0x10179ccc) = 0;
        }
        if ((int8_t)U8(U32(U32(obj, 0), 0x54), 0x2c) != 0 && (int8_t)U8(U32(U32(obj, 0), 0x54), 0x28) == 0)
            Sim_100d63d4();
        if (G32(0x10179cbc) != 0) {
            G32(0x10179cbc) = 0;
            Sim_100d624d(obj);
        }
        break;
    default:
        break;
    }
    if (GS32(0x10179cd8) == -1) {
        if (S32(obj, 0x4f0) != 3) {
            /* The original computes the offset of [obj] +0x58.. from the point [0x1014cae0] into locals it never
             * reads; only the memory reads remain. */
            volatile float offset;

            offset = FL(U32(obj, 0), 0x58) - FL(G32(0x1014cae0), 0);
            offset = FL(U32(obj, 0), 0x5c) - FL(G32(0x1014cae0), 4);
            offset = FL(U32(obj, 0), 0x60) - FL(G32(0x1014cae0), 8);
            (void)offset;
            U32(obj, 0x4f0) = 3;
            G32(0x10179cd8) = 0;
        }
    } else if (GS32(0x10179cd8) == 1) {
        if (((U16(obj, 0x560) & 4) == 0 || (U16(obj, 0x560) & 8) != 0) && S32(obj, 0x4f0) == 3) {
            char text[0x30];

            W7N7_CALL_100fc780(text, (const char *)(uintptr_t)G32(0x101c464c));
            Sim_1007df6c(text, (const char *)(uintptr_t)G32(0x109b98c0), 0x16a, 0x32);
            U32(obj, 0x4f0) = 0;
            U16(U32(obj, 0), 0x1c) = U16(U32(obj, 0), 0x1c) & 0xffefu;
            G32(0x10179cd8) = 0;
        }
    }
}

// FUNCTION: HGSIM 0x1007520b
void __cdecl Sim_1007520b(void)
{
    uint8_t *block = BYTES(U32(PTRTAB(G32(0x1017b8c8)), 0x28));
    int i, k, count, ok;

    Sim_1002ba28(S32(block, 0x613));
    Sim_1002b92d();
    for (i = 0; i <= 0xd; i++) {
        uint8_t *entry = BYTES(G32(0x10149958u + 4u * (uint32_t)i));
        uint8_t *checks;

        if (entry == 0)
            continue;
        ok = 0;
        if (U32(entry, 0) != 0 && U32(entry, 0x18) != 0) {
            U32(U32(entry, 0x18), 0) = (uint32_t)(uintptr_t)SIM_PTR(void, 0x109e3e20);
            U32(U32(entry, 0), 0) = U32(U32(entry, 0x18), 0);
            W7N7_CALL_10074cea(entry, i);
            checks = BYTES(U32(entry, 0x80));
            if (checks != 0) {
                ok = 1;
                if (U32(checks, 0x14) != 0)
                    ok &= ((W7nCheckFn)(uintptr_t)U32(checks, 0x14))(entry, checks);
                count = S32(checks, 0xc);
                for (k = 0; k < count; k++) {
                    uint8_t *sub = BYTES(U32(checks, k * 0x14 + 0x28));

                    if (sub != 0 && U32(sub, 0x14) != 0)
                        ok &= ((W7nCheckFn)(uintptr_t)U32(sub, 0x14))(entry, sub);
                }
            }
        }
        if (ok != 1)
            G32(0x10149958u + 4u * (uint32_t)i) = 0;
    }
}

// FUNCTION: HGSIM 0x10075585
void __cdecl Sim_10075585(void)
{
    uint8_t *e = BYTES(G32(0x1015dc14));
    int done = 0;

    while (e != 0 && done == 0) {
        int32_t state = S32(e, 4);
        int32_t wanted = S32(e, 8);

        if (state != wanted) {
            if (wanted == 1) {
                if (Sim_10075063((int *)e) != 0)
                    state = 1;
            } else if (wanted == 0) {
                Sim_10075124(e);
                state = 0;
            }
        }
        S32(e, 4) = state;
        S32(e, 8) = wanted;
        if (state == 1) {
            if ((U32(U32(e, 0xc), 4) & 2) == 0 || G32(0x109e3dac) == 0) {
                if (W7N7_CALL_10075667(U32(e, 0xc)) == 0)
                    U32(e, 8) = 0;
            }
            done = 1;
        }
        e = BYTES(U32(e, 0x14));
    }
}

/* Copies five dwords upwards, one at a time (rep movsd). */
static void W7n_Move5(uint8_t *dst, const uint8_t *src)
{
    int i;

    for (i = 0; i < 5; i++)
        U32(dst, 4 * i) = U32(src, 4 * i);
}

// FUNCTION: HGSIM 0x10078950
void __cdecl Sim_10078950(uint8_t *obj)
{
    uint8_t savedEntry[20], savedRect[20];
    uint8_t *pair;
    uint8_t *rect;

    if (S16(obj, 4) == 0 || U32(obj, 0x30) == 0 || U32(obj, 0x30) == 2 || U32(obj, 0x30) == 1)
        return;
    pair = BYTES(U32(obj, 0xf4));
    if (pair != 0) {
        if (U32(obj, 0xf8) != 1)
            Sim_1008f630((void **)pair);
        rect = (uint8_t *)Sim_1008f867(0, (const uint32_t *)pair);
        if (rect != 0) {
            W7n_Move5(savedEntry, GBYTES(0x10185400) + S32(obj, 0xc) * 0x14);
            W7n_Move5(savedRect, BYTES(U32(obj, 0xec)));
            W7n_Move5(GBYTES(0x10185400) + S32(obj, 0xc) * 0x14, rect);
            W7n_Move5(BYTES(U32(obj, 0xec)), rect);
            W7N7_CALL_100782a9(obj);
            W7n_Move5(GBYTES(0x10185400) + S32(obj, 0xc) * 0x14, savedEntry);
            W7n_Move5(BYTES(U32(obj, 0xec)), savedRect);
        } else {
            W7N7_CALL_100782a9(obj);
        }
    }
    U32(obj, 0xf8) = G32(0x109e89d8);
}

#define GROUP_RECORD(i, j) (GBYTES(0x10473800) + (i) * 0x4fc3a + (j) * 0x13f)
#define GROUP_NAME(i, j) ((const char *)(uintptr_t)(G32(0x1017ee40u + 4u * (uint32_t)(i)) + (uint32_t)(j) * 0x97u + 0x70u))

// FUNCTION: HGSIM 0x1007950a
uint32_t __cdecl Sim_1007950a(const char *name)
{
    uint32_t mask = 0;
    int i, j;

    if (*name == 0)
        return 0;
    for (i = 0; i < GS32(0x104737e0); i++) {
        if (G32(0x1017ee40u + 4u * (uint32_t)i) == 0)
            continue;
        for (j = 0; j < S32(GBYTES(0x10473800) + i * 0x4fc3a, 0); j++) {
            uint8_t state = U8(GROUP_RECORD(i, j), 0x3a);

            if (state != 0 && state != 1 && state != 7)
                continue;
            if (Str_CompareNoCase(name, GROUP_NAME(i, j)) != 0)
                continue;
            mask |= 1u << (j & 31);
            mask |= (1u << (i & 31)) << 16;
            U8(GROUP_RECORD(i, j), 0x3a) = 7;
        }
    }
    return mask;
}

// FUNCTION: HGSIM 0x10079664
void __cdecl Sim_10079664(const char *name, uint32_t bits, uint32_t value)
{
    int i, j;

    for (i = 0; i < GS32(0x104737e0); i++) {
        if (G32(0x1017ee40u + 4u * (uint32_t)i) == 0)
            continue;
        for (j = 0; j < 0x400; j++) {
            uint8_t *rec = GROUP_RECORD(i, j);
            uint32_t n;

            if (U8(rec, 0x3a) != 7)
                continue;
            if (Str_CompareNoCase(name, GROUP_NAME(i, j)) != 0)
                continue;
            if ((U32(rec, 0x3c) & bits) == 0)
                continue;
            n = U8(rec, 0x128);
            if (n >= 0x28)
                continue;
            U8(rec, 0x128) = (uint8_t)(U8(rec, 0x128) + 1);
            U16(rec, 0x129 + n * 2) = (uint16_t)value;
            Sim_1007cbb9();
        }
    }
}

// FUNCTION: HGSIM 0x10079b68
int __cdecl Sim_10079b68(uint8_t *obj, uint32_t a)
{
    int found = 0;
    uint32_t e0 = 0, e4 = 0, e8 = 0, ec = 0;

    GS32(0x1037305c) = GS32(0x1015dc78) / 0xb5;
    if (U8(obj, 0x128) != 0 && U8(obj, 0x12a) == 1) {
        uint32_t index = U8(obj, 0x129);

        if ((int32_t)index < GS32(0x1017e5a8)) {
            uint8_t *entry = GBYTES(0x109a8654) + index * 0x54;

            e4 = U32(entry, 4);
            e8 = U32(entry, 8);
            ec = U32(entry, 0xc);
            e0 = U32(entry, 0);
            found = 1;
        }
    }
    W7N7_CALL_100527fa(a, e4, e8, ec, e0);
    U32(obj, 4) = G32(0x1037305c);
    U32(obj, 0xa1) = G32(0x1037305c);
    return found;
}

// FUNCTION: HGSIM 0x1007e570
uint8_t *__cdecl ProjFile_FindEntry(const char *name, int create)
{
    const uint8_t *p = (const uint8_t *)name;
    uint32_t hash = 0;
    uint32_t chain;
    uint8_t *e;

    while (*p != 0) {
        uint32_t low;

        hash += (uint32_t)(*p | 0x20);
        p++;
        low = hash & 0xffffu;
        hash = (hash & 0xffff0000u) | (((low << 1) | (low >> 15)) & 0xffffu);
    }
    chain = hash % 0x65u;
    for (e = BYTES(G32(0x1017ad18u + chain * 4u)); e != 0; e = BYTES(U32(e, 0x10))) {
        if (Str_CompareNoCase((const char *)e, name) == 0)
            return e;
    }
    if (create == 0)
        return 0;
    if (GS32(0x1017afb0) >= 0xc8)
        W7N7_CALL_100bc280(0x23, SIM_PTR(char, 0x10149dd0), 0);
    e = GBYTES(0x10179d78) + GS32(0x1017afb0) * 0x14;
    G32(0x1017afb0) = G32(0x1017afb0) + 1;
    Str_Copy((char *)e, name);
    U32(e, 0x10) = G32(0x1017ad18u + chain * 4u);
    G32(0x1017ad18u + chain * 4u) = (uint32_t)(uintptr_t)e;
    return e;
}
