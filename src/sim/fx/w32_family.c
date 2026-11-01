/*
 * Round 32, E / F: what 1.2's particle families share. 1.2 repeats this code in every spawn and draw callback; here
 * it is written once (each helper notes the 1.2 code it stands for).
 */
#include "w32.h"
#include "../unsorted_1009/w2g.h"
#include "../util/util.h"
#include "../util/access.h"
#include "../render/w2n.h"

/* 1.2 0x1011e9d0: the particle size factors (24 floats; 23 of 1.0, the last 2.0) */
const uint32_t g_w32_1011e9d0[24] = {
    0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000,
    0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000,
    0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000, 0x40000000,
};

/*
 * One particle mark (0xffffffff at a particle's frame) of 1.2's spawn loop. 1.2's 0x20 marks for a record of 16 run
 * past the record; for the last record of the pool (1.2 0x10191060, 1000 x 0x406, ending at 0x1028c7d0) they run
 * past the pool into the data 1.2 keeps there. Ours has the same data after its pool (1.0's layout, which also follows
 * its pool), at other addresses: 1.2 0x1028c7e0..0x1028c95f is ours + 0xe2900 (the saved camera 0x1036f0e0, the
 * view's floats 0x1036f1c0.., the saved 0x1014cae8 at 0x1036f1e0, the grid row 0x1036f248), 0x1028c960..0x1028caab
 * ours + 0xe2940 (the cell list 0x1036f2a0); pairs from the identical functions' relocations (tools/v12, round 32).
 * Our pool keeps 1.2's records in 1.0's larger buffer, so a mark past its 1000th record is sent to that address.
 */
static void W32_Mark(uint8_t *p)
{
    uint8_t *end = SIM_PTR(uint8_t, 0x10217c20 + 1000 * 0x406);
    uint32_t a12;

    if (p >= end && p < end + 0x2e0) {
        a12 = 0x1028c7d0u + (uint32_t)(p - end);
        p = SIM_PTR(uint8_t, a12 + (a12 < 0x1028c960u ? 0xe2900u : 0xe2940u));
    }
    U32(p, 0) = 0xffffffffu;
}

uint8_t *W32_NewSystem(uint32_t update, uint32_t draw, uint32_t arg, int force, int32_t type)
{
    uint8_t *rec;
    int32_t i, n;

    if ((int32_t)(G32(0x1017cc24) + 1) >= GS32(0x1017cc20))
        return 0;
    rec = Sim_10090c7d(update, draw, arg, force);
    if (rec == 0)
        return 0;
    U16(rec, 0x10) = 1;
    U32(rec, 0x402) = (uint32_t)type;
    U32(rec, 0x3fa) = 0xffffffffu;
    /* As 1.2 (Diego: "do as 1.2 does"): 0x20 particles marked in a record of 16, the last 16 marks landing in the
       record itself and in the next one (W32_Mark: past the last record, where 1.2's memory has) */
    n = 0x20;
    for (i = 0; i < n; i++)
        W32_Mark(rec + i * 0x32 + 0x30);
    U32(rec, 0x3fe) = 0;
    if (g_w32_1011e700[type].count != 0) {
        switch (g_w32_1011e700[type].order) {
        case 1:
            U32(rec, 0x3fa) = 0;
            break;
        case 2:
            U32(rec, 0x3fa) = (uint32_t)Sim_1009c929((int)g_w32_1011e700[type].count);
            break;
        default:
            U32(rec, 0x3fa) = 0xffffffffu;
            break;
        }
    }
    return rec;
}

uint32_t W32_ParticleFlags(uint8_t *rec, int32_t i, uint32_t plain)
{
    int32_t type = S32(rec, 0x402), frame;
    const W32Type *t = &g_w32_1011e700[type];

    if (t->group < 1)
        return plain;
    if ((int32_t)t->count > 1) {
        frame = -1;
        if ((U16(rec, 0x10) & 4) != 0 && i >= 0) {
            frame = S32(rec + i * 0x32, 0x30);
        } else if (S32(rec, 0x3fa) >= 0) {
            frame = S32(rec, 0x3fa) + 1;
            if (frame >= (int32_t)t->count) {
                switch (t->loop) {
                case 1:
                    frame = 0;
                    break;
                case 2:
                    frame = (int32_t)t->count - 1;
                    break;
                default:
                    U32(rec, 0x3fa) = 0xffffffffu;
                    U16(rec, 0x10) = (uint16_t)(U16(rec, 0x10) | 2);
                    return 0;
                }
            }
            U32(rec, 0x3fa) = (uint32_t)frame;
        }
        if (frame < 0)
            frame = Sim_1009c929((int)t->count);
        if (i >= 0)
            U32(rec + i * 0x32, 0x30) = (uint32_t)frame;
    }
    return (uint32_t)t->group | 0x7000;
}

int32_t W32_ParticleScale(uint8_t *rec, int32_t size)
{
    const W32Type *t = &g_w32_1011e700[S32(rec, 0x402)];
    float f, scale = *(const w32fa *)(rec + 0x3fe);

    if (G32(0x1017b940) != 0) {
        f = (float)((double)(float)size * (double)t->size);
        f = (float)((double)f * (double)*(const w32fa *)&g_w32_1011e9d0[Sim_1009c929(0x18)]);
        if (f < 1.0f)
            f = 1.0f;
        if (g_w32_1014b58c != 0 && (scale < 0.0f || scale > 0.0f))   /* fcomp 0.0 / test ah, 0x40 */
            f = (float)((double)f * (double)scale);
        else
            f = (float)((double)f * (double)*(const w32fa *)(BYTES(G32(0x1014cae0)) + 0x18));
        size = Render_Ftol((double)f);
    }
    return size;
}

int32_t W32_ParticleSize(uint8_t *rec, int32_t size)
{
    size = W32_ParticleScale(rec, size);
    if (size < 1)
        size = g_w32_1011e700[S32(rec, 0x402)].size >> 1;
    return size;
}

void W32_SetTemplate(uint8_t *rec)
{
    uint8_t *v = rec + 0x36a;
    const W32Type *t = &g_w32_1011e700[S32(rec, 0x402)];
    static const uint32_t kUv[8] = {0, 0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000, 0, 0, 0};
    int32_t k;

    for (k = 0; k < 4; k++, v += 0x24) {
        U32(v, 0x10) = t->intensity;
        v[0x14] = t->red;
        v[0x15] = t->green;
        v[0x16] = t->blue;
        v[0x17] = t->unused;
        U32(v, 0x18) = 0xff000000u;
        U32(v, 0x1c) = kUv[k * 2];
        U32(v, 0x20) = kUv[k * 2 + 1];
        U32(v, 8) = 0x3f800000u;
        U32(v, 0xc) = 0x3f800000u;
    }
}
