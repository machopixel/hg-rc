#include "w2h.h"
#include "../unsorted_1005/w2h.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x100119a7
void __cdecl Sim_100119a7(void *obj)
{
    uint32_t flag = 0; /* shared by both inner searches, as the original's single stack slot */
    int16_t k;

    U32(U32(obj, 0x170), 0) = 0;
    for (k = 2; k >= 0; k--) {
        uint32_t set = U32(obj, 0x174 + (uint32_t)(int32_t)k * 4);
        uint16_t count;
        int16_t i = 0;
        int16_t used = 0;
        int16_t n = 0;
        uint32_t item;
        int16_t done;

        if (set == 0)
            continue;
        count = U16(set, 0);
        for (;;) {
            uint16_t left = count;

            count--;
            if (left == 0)
                break;
            flag = S16(U32(set, 2), (uint32_t)(int32_t)i * 4) == S16(obj, 0x1a6);
            if (flag != 0) {
                n = S16(U32(set, 2), (uint32_t)(int32_t)i * 4 + 2);
                break;
            }
            used = (int16_t)(used + S16(U32(set, 2), (uint32_t)(int32_t)i * 4 + 2));
            i++;
        }
        if (flag == 0)
            continue;
        item = U32(set, 6) + (uint32_t)(int32_t)used * 0xc;
        for (done = 0; done < n; done++, item += 0xc) {
            int16_t j = 0;

            if (S16(item, 0) == 0)
                break;
            flag = 0;
            while (flag == 0 && U32(U32(obj, 0x170), (uint32_t)(int32_t)j * 4) != 0) {
                uint32_t other = U32(U32(obj, 0x170), (uint32_t)(int32_t)j * 4);

                if (U16(other, 2) == U16(item, 2) && S16(other, 0) == S16(item, 0))
                    flag = 1;
                j++;
            }
            if (flag == 0) {
                U32(U32(obj, 0x170), (uint32_t)(int32_t)j * 4) = item;
                U32(U32(obj, 0x170), (uint32_t)(int32_t)j * 4 + 4) = 0;
            }
        }
    }
}

// FUNCTION: HGSIM 0x10013f2a
uint32_t __cdecl Sim_10013f2a(const void *obj)
{
    int16_t key = (int16_t)U16(obj, 4);
    uint32_t last = (uint32_t)(uintptr_t)obj; /* eax at the end of the loop: the slot address once it has stepped */
    u32a *slot = &PTRTAB(0);
    int16_t i;

    for (i = 0; i < GS32(0x1015dc64); i++, slot++, last = (uint32_t)(uintptr_t)slot) {
        uint32_t rec = *slot;

        if (rec == 0)
            continue;
        if ((S16(rec, 0x1c) & 6) != 0)
            continue;
        if (U16(rec, 0x1aa) != ((uint32_t)(int32_t)key | 0x200))
            continue;
        if (Sim_10052c60((int32_t)U32(rec, 4), key) != 0)
            continue;
        if (S16(rec, 0x1a6) != 3)
            continue;
        return U32(rec, 4) | 0x200;
    }
    return (last & 0xffff0000u) | 0xffff;
}

// FUNCTION: HGSIM 0x100152fc
int __cdecl Sim_100152fc(unsigned a, unsigned b, int c)
{
    uint32_t kind = (b & 0xffff) & 0xff00;
    uint32_t index = (b & 0xffff) & 0xff;
    uint32_t flags1 = 0; /* address of the word holding the request bits */
    uint32_t flags2 = 0; /* address of the word holding the per-number bits */
    int result = 1;
    int16_t mask = 0;

    if ((kind & 0xf000) != 0)
        return 1;
    if (c == 0x4000) {
        uint32_t p = 0;

        if (kind == 0x200)
            p = PTRTAB(index) + 0x150;
        else if (kind == 0x400)
            p = 0x109e9bec + index * 0x50;
        if ((U32(p, 0) & 0x40000000) == 0x40000000) {
            U32(p, 0) = U32(p, 0) & 0xbfffffffu;
            return 1;
        }
        return 0;
    }
    if (kind == 0x100) {
        flags1 = (uint32_t)(uintptr_t)GBYTES(0x109a8664 + index * 0x54);
        flags2 = (uint32_t)(uintptr_t)GBYTES(0x109a8666 + index * 0x54);
    } else if (kind == 0x200) {
        flags1 = PTRTAB(index) + 0x1c;
        flags2 = PTRTAB(index) + 0x1e;
    } else if (kind == 0x400) {
        flags1 = (uint32_t)(uintptr_t)GBYTES(0x109e9ba0 + index * 0x50);
        flags2 = (uint32_t)(uintptr_t)GBYTES(0x109e9ba6 + index * 0x50);
    }
    if (flags1 == 0)
        return 1;
    if (c > 4) {
        if (c == 8 || c == 0x100) {
            if (kind == 0x200 || kind == 0x400 || kind == 0x100)
                mask = 0x20;
            goto test;
        }
        if (c != 0x1000)
            goto test;
    } else if (c != 4) {
        if (c <= 0 || c > 2)
            goto test;
    }
    if (kind == 0x200)
        mask = 4;
    else if (kind == 0x400)
        mask = 4;
    else if (kind == 0x100) {
        flags1 = 0;
        result = 0;
    }
test:
    if (flags1 != 0) {
        if ((S16(flags1, 0) & mask) == 0)
            result = 0;
        else if (((uint32_t)(int32_t)S16(flags2, 0) & (1u << (a & 0x1f))) == 0 && (c == 8 || c == 0x100))
            result = 0;
    }
    return result;
}

static const uint32_t kSlotTables[10] = {
    0x10130cd8, 0x10130d98, 0x10130d18, 0x10130dd8, 0x10130e18,
    0x10130d58, 0, 0x10130e18, 0x10130cd8, 0x10130dd8,
};

// FUNCTION: HGSIM 0x10016a6b
void __cdecl Sim_10016a6b(void *obj, int16_t slot, int16_t sel, int16_t other)
{
    uint32_t kind;
    uint32_t differs;
    uint32_t table;

    if (sel == -1) {
        U32(obj, 0x174 + (uint32_t)(int32_t)slot * 4) = 0;
        return;
    }
    differs = S32(obj, 4) != (int32_t)other;
    kind = U32(obj, 0) - 1;
    if (kind > 9)
        return;
    table = kSlotTables[kind];
    if (table == 0)
        return;
    U32(obj, 0x174 + (uint32_t)(int32_t)slot * 4) =
        G32(0x10a38a40 + (uint32_t)S16(GBYTES(table), (uint32_t)(int32_t)sel * 4 + differs * 2) * 4);
}

// FUNCTION: HGSIM 0x10017a05
int __cdecl Sim_10017a05(const void *obj)
{
    uint32_t record;

    if (obj == 0)
        return 0;
    if (S32(obj, 0) != 1 && S32(obj, 0) != 9)
        return 0;
    if (S32(obj, 0x14) == 1)
        return 0;
    if (S32(obj, 0x14) == 3 && U32(obj, 0xc) != G32(0x1017b8c8))
        return 0;
    record = U32(obj, 0x28);
    if ((U16(record, 0x562) & 0x60e) == 0)
        return 0;
    if ((U16(record, 0x562) & 8) == 0)
        return 1;
    if ((U16(record, 0x560) & 0x20) == 0)
        U8(U32(obj, 0x54), 0x55) = 1;
    return 0;
}

/* 1.2's unit flag tests and the action gate that replaces Sim_10017a05 (1.2 0x10010bb4..0x10010cd2). 1.2 reads the
   unit at +0x24 and the control block at +0x50: ours at +0x28 / +0x54. */
// FUNCTION: HGSIM12 0x10010bb4
int __cdecl Sim12_10010bb4(const void *obj)
{
    if (obj == 0)
        return 0;
    return (U16(U32(obj, 0x28), 0x562) & 0x60e) != 0 ? 1 : 0;
}

// FUNCTION: HGSIM12 0x10010bf4
int __cdecl Sim12_10010bf4(const void *obj, int unused)
{
    (void)unused;
    if (obj == 0)
        return 0;
    return (U16(U32(obj, 0x28), 0x560) & 0x20) != 0 ? 1 : 0;
}

// FUNCTION: HGSIM12 0x10010c72
int __cdecl Sim12_10010c72(const void *obj)
{
    if (obj == 0)
        return 0;
    return (U16(U32(obj, 0x28), 0x562) & 8) != 0 ? 1 : 0;
}

// FUNCTION: HGSIM12 0x10010cb0
void __cdecl Sim12_10010cb0(const void *obj)
{
    U8(U32(obj, 0x54), 0x55) = 1;
}

// FUNCTION: HGSIM12 0x10010cd2
int __cdecl Sim12_10010cd2(const void *obj)
{
    int jets, busy;

    if (obj == 0)
        return 0;
    if ((S32(obj, 0) == 1 || S32(obj, 0) == 9) &&
        (S32(obj, 0x14) == 2 || (S32(obj, 0x14) == 3 && U32(obj, 0xc) == G32(0x1017b8c8)))) {
        jets = Sim12_10010bf4(obj, 0);
        busy = Sim12_10010bb4(obj);
#ifdef W2HG_MUTATE /* the checker's negative check only: the jets ignored */
        jets = 0;
#endif
        if (jets == 0 && busy == 0)
            return 1;
        if (Sim12_10010c72(obj) != 0 && jets == 0)
            Sim12_10010cb0(obj);
        return 0;
    }
    return 1;
}
