#include "w3t.h"
#include "../util/w3t.h"
#include "../render/w2n.h"
#include "../unsorted_100d/w2f.h"
#include "../unsorted_100d/w2i.h"

#define TABLE_ENTRY(n) (0x10145508u + (uint32_t)(n) * 0xd6u)
#define RECORD(owner, i) (U32(owner, 0x84) + (uint32_t)(i) * 0x128u)

// FUNCTION: HGSIM 0x1000a9bf
int __cdecl Sim_1000a9bf(const void *obj, const void *entry)
{
    float x = FL32(obj, 0xd8);
    float lo = FL32(entry, 0x4c);
    float hi;

    if (U32(entry, 0x86) & 0x40) {
        hi = FL32(entry, 0x50);
    } else {
        float scaled = (float)((double)S32(entry, 0x5c) * (double)FL32(entry, 0x3c));

        hi = (scaled >= FL32(entry, 0x50)) ? FL32(entry, 0x50) : scaled;
    }
    if (!(x >= lo))
        return 3;
    if (!(x >= hi))
        return 2;
    return x == hi;
}

// FUNCTION: HGSIM 0x1000b9f1
int __cdecl Sim_1000b9f1(void *obj, int *record, int *entryIndex, uint32_t *entry, uint32_t *recordAddr)
{
    uint32_t base, rec, ent;
    int current, index;

    if (obj == 0)
        return 0;
    base = U32(obj, 0);
    if (G32(0x1015ec58) != 0 && S32(base, 0x14) == 3 && (int16_t)(U16(base, 0x1aa) & 0xf00) == 0x200) {
        int16_t other = (int16_t)(U16(base, 0x1aa) & 0xff);
        uint32_t o = PTRTAB(other);

        if (o != 0 && (S32(o, 0x14) == 0 || S32(o, 0x14) == 1) && S32(base, 0xc) != other)
            return 0;
    }
    if (S32(obj, 0x4fc) == -1)
        return 0;
    current = S32(obj, 0x4fc);
    rec = RECORD(obj, current);
    index = S32(rec, 4);
    ent = TABLE_ENTRY(index);
    {
        int state = Sim_1000a9bf(BYTES(base), GBYTES(ent));

        if (state < 1 || state > 2)
            return 0;
    }
    if (Sim_100d6200(obj) != 1 || S32(rec, 0x20) == 0 || S32(obj, 0x4dc) > GS32(0x1015dc78))
        return 0;
    if (record)
        *record = current;
    if (entryIndex)
        *entryIndex = index;
    if (entry)
        *entry = (uint32_t)(uintptr_t)GBYTES(ent);
    if (recordAddr)
        *recordAddr = rec;
    return 1;
}

// FUNCTION: HGSIM 0x1000bf73
int __cdecl Sim_1000bf73(void *obj, int *flag)
{
    uint32_t owner = U32(obj, 0x28);
    int16_t best = 0;
    int16_t i;
    int found = 0;
    float total = 0.0f;
    uint32_t bonusBits = 0;

    for (i = 0; i < S32(owner, 0x4f8); i = (int16_t)(i + 1)) {
        uint32_t rec = RECORD(owner, (int32_t)i);
        int16_t kind = (int16_t)U16(rec, 4);
        uint32_t ent = TABLE_ENTRY((int32_t)kind);

        U32(rec, 0xc) = 0;
        if (S32(obj, 0xf0) == -1 || (U32(obj, 0xf0) & 0x1000) != 0)
            continue;
        if (Sim_1000a9bf(obj, GBYTES(ent)) != 2 || U32(rec, 0x20) == 0 || S32(rec, 8) == -1)
            continue;
        if (S32(obj, 4) != GS32(0x1017b8c8)) {
            double limit = ((double)FL32(owner, 0x4e8) + (double)total) + (double)FL32(GBYTES(ent), 0x44);

            if (!((double)S32(obj, 0x1e4) > limit)) {
                found = 0;
                break;
            }
        }
        found = 1;
        U32(rec, 0xc) = 1;
        total = (float)((double)total + (double)FL32(GBYTES(ent), 0x44));
        if (best == 0 && (U32(GBYTES(ent), 0x86) & 1) != 0 && S32(rec, 8) == 1)
            best = (int16_t)(i + 1);
        if (kind == 0x16)
            bonusBits = 0x3ff00000u;
        if (kind == 0x14)
            bonusBits = 0x40700000u;
    }
    if (found) {
        f32a *target = (f32a *)BYTES(U32(obj, 0x54));

        *target = (float)((double)*target + (double)BitsToFloat(bonusBits));
        if (best != 0)
            S32(owner, 0x4fc) = (int32_t)best - 1;
        else
            Sim_100d60b6(BYTES(owner), 1);
    } else {
        Sim_100d5cc1(BYTES(owner), 1);
    }
    if (found && best != 0)
        *flag = 1;
    return found;
}

// FUNCTION: HGSIM 0x1000cd55
int __cdecl Sim_1000cd55(int key, float value, uint32_t *out)
{
    const uint8_t *e = GBYTES(0x10a3a600);
    int i;

    for (i = 0; i < GS32(0x1015d0a0); i++, e += 0xa8) {
        float base;
        int idx;

        if (key != S32(e, 0))
            continue;
        base = FL32(GBYTES(0x10145554u + (uint32_t)key * 0xd6u), 0);
        idx = Render_Ftol((((double)value - (double)base) * (double)FL32(e, 4)) - 1.0);
        if (idx < 0)
            idx = 0;
        else if (idx >= 0x14)
            idx = 0x13;
        *out = U32(e, 0x58 + idx * 4);
        return 1;
    }
    return 0;
}

// FUNCTION: HGSIM 0x1000ce13
int __cdecl Sim_1000ce13(const void *obj, const void *recs, const void *skip, int count)
{
    int i;

    for (i = 0; i < count; i++) {
        const uint8_t *rec = BYTES(recs) + (uint32_t)i * 0x128u;
        uint32_t ent = TABLE_ENTRY(S32(rec, 4));

        if (rec == (const uint8_t *)skip)
            continue;
        if (U32(GBYTES(ent), 0x78) <= U32(GBYTES(0x10145580u + (uint32_t)S32(skip, 4) * 0xd6u), 0))
            continue;
        if (Sim_1000a9bf(obj, GBYTES(ent)) == 2)
            return 1;
    }
    return 0;
}
