#include "w10d.h"
#include "../simmem.h"
#include "../util/access.h"
#include "../view/view.h"
#include "../unsorted_100b/fns.h"
#include "../unsorted_1000/w5a.h"
#include "../unsorted_1006/w9b.h"

#define F(p, off) (*(float *)(BYTES(p) + (off)))
#define GF(addr) (*SIM_PTR(float, addr))

/* Squared distance as the x87 at 53-bit precision: the float differences are stored, their products are exact. */
static double W10d_Dist2(float dx, float dy, float dz)
{
    return (double)dx * (double)dx + (double)dy * (double)dy + (double)dz * (double)dz;
}

static float W10d_Sub(float a, float b)
{
    return (float)((double)a - (double)b);
}

// FUNCTION: HGSIM 0x1007a4af
int __cdecl Sim_1007a4af(const uint8_t *id, int32_t side)
{
    uint32_t index = id[0];
    uint32_t kind = (uint32_t)id[1] << 8;
    float dy = 0.0f, dx, dz, r;
    uint32_t me, o, rec;
    float px, py, pz;
    int32_t i, count, inside;

    if (GS32(0x109b7d40 + (uint32_t)side * 0x6cu) < 0 && G32(0x1015ec58) == 0)
        return 0;
    if (kind == 0x200) {
        me = PTRTAB(index);
        for (i = 0; i < GS32(0x1015dc64); i++) {
            o = PTRTAB(i);
            if (S32(o, 8) != side || (S16(o, 0x1c) & 6) != 0)
                continue;
            if (Sim_10079feb(S32(o, 0)) == Sim_10079feb(S32(me, 0)))
                dy = W10d_Sub(F(o, 0x5c), F(me, 0x5c));
            else
                dy = 0.0f;
            dz = W10d_Sub(F(o, 0x60), F(me, 0x60));
            dx = W10d_Sub(F(o, 0x58), F(me, 0x58));
            if ((double)GF(0x1012b844) * (double)GF(0x1012b844) >= W10d_Dist2(dx, dy, dz))
                return 1;
        }
        return 0;
    }
    if (kind == 0x400) {
        rec = (uint32_t)(uintptr_t)GBYTES(0x109e9ba0u + index * 0x50u);
        Sim_100b53f0(S32(rec, 8), (uint32_t *)&px, (uint32_t *)&py, (uint32_t *)&pz);
        for (i = 0; i < GS32(0x1015dc64); i++) {
            o = PTRTAB(i);
            if (S32(o, 8) != side)
                continue;
            if (Sim_10079feb(S32(o, 0)) != 0)
                dy = W10d_Sub(F(o, 0x5c), py);
            else
                dy = 0.0f;
            dz = W10d_Sub(F(o, 0x60), pz);
            dx = W10d_Sub(F(o, 0x58), px);
            if ((double)GF(0x1012b84c) * (double)GF(0x1012b84c) >= W10d_Dist2(dx, dy, dz))
                return 1;
        }
        return 0;
    }
    if (kind == 0x100) {
        count = 0;
        inside = 0;
        rec = (uint32_t)(uintptr_t)GBYTES(0x109a8640u + index * 0x54u);
        r = F(rec, 0x10);
        if (!(r > GF(0x1012b848)))
            r = 20000.0f;     /* 0x469c4000 */
        for (i = 0; i < GS32(0x1015dc64); i++) {
            o = PTRTAB(i);
            if (S32(o, 8) != side || (S16(o, 0x1c) & 4) != 0)
                continue;
            count++;
            if (Sim_10079feb(S32(o, 0)) != 0)
                dy = W10d_Sub(F(o, 0x5c), F(rec, 0x1c));
            else
                dy = 0.0f;
            dz = W10d_Sub(F(o, 0x60), F(rec, 0x20));
            dx = W10d_Sub(F(o, 0x58), F(rec, 0x18));
            if (!((double)r * (double)r >= W10d_Dist2(dx, dy, dz)))
                continue;
            if (i == GS32(0x1017b8c8) && (S16(o, 0x1c) & 0x2000) != 0 && (S16(rec, 0x24) & 0x20) == 0 &&
                U32(rec, 0) != 0) {
                U16(rec, 0x24) |= 0x20;
                U16(rec, 0x26) |= (uint16_t)(1u << ((uint32_t)side & 31u));
                Sim_1000ebfb(SIM_PTR(const char, 0x10149c14), 0xfffff63cu, 0, 5, 0x50);
            }
            inside++;
        }
        return count != 0 && inside != 0 && count == inside;
    }
    return 0;
}

// FUNCTION: HGSIM 0x1007a955
int __cdecl Sim_1007a955(const uint8_t *id, int32_t side)
{
    uint32_t index = id[0];
    uint32_t kind = (uint32_t)id[1] << 8;
    float dy = 0.0f, dx, dz, r;
    uint32_t me, o, rec;
    float px, py, pz;
    int32_t i;

    if (GS32(0x109b7d40 + (uint32_t)side * 0x6cu) < 0 && G32(0x1015ec58) == 0)
        return 0;
    if (kind == 0x200) {
        me = PTRTAB(index);
        for (i = 0; i < GS32(0x1015dc64); i++) {
            o = PTRTAB(i);
            if (S32(o, 8) != side || (S16(o, 0x1c) & 6) != 0)
                continue;
            if (Sim_10079feb(S32(o, 0)) == Sim_10079feb(S32(me, 0)))
                dy = W10d_Sub(F(o, 0x5c), F(me, 0x5c));
            else
                dy = 0.0f;
            dz = W10d_Sub(F(o, 0x60), F(me, 0x60));
            dx = W10d_Sub(F(o, 0x58), F(me, 0x58));
            if ((double)GF(0x1012b844) * (double)GF(0x1012b844) >= W10d_Dist2(dx, dy, dz))
                return 1;
        }
        return 0;
    }
    if (kind == 0x400) {
        rec = (uint32_t)(uintptr_t)GBYTES(0x109e9ba0u + index * 0x50u);
        Sim_100b53f0(S32(rec, 8), (uint32_t *)&px, (uint32_t *)&py, (uint32_t *)&pz);
        for (i = 0; i < GS32(0x1015dc64); i++) {
            o = PTRTAB(i);
            if (S32(o, 8) != side)
                continue;
            if (Sim_10079feb(S32(o, 0)) != 0)
                dy = W10d_Sub(F(o, 0x5c), py);
            else
                dy = 0.0f;
            dz = W10d_Sub(F(o, 0x60), pz);
            dx = W10d_Sub(F(o, 0x58), px);
            if ((double)GF(0x1012b84c) * (double)GF(0x1012b84c) >= W10d_Dist2(dx, dy, dz))
                return 1;
        }
        return 0;
    }
    if (kind == 0x100) {
        rec = (uint32_t)(uintptr_t)GBYTES(0x109a8640u + index * 0x54u);
        r = F(rec, 0x10);
        if (!(r > GF(0x1012b848)))
            r = 20000.0f;     /* 0x469c4000 */
        for (i = 0; i < GS32(0x1015dc64); i++) {
            o = PTRTAB(i);
            if (S32(o, 8) != side)
                continue;
            if (Sim_10079feb(S32(o, 0)) != 0)
                dy = W10d_Sub(F(o, 0x5c), F(rec, 0x1c));
            else
                dy = 0.0f;
            dz = W10d_Sub(F(o, 0x60), F(rec, 0x20));
            dx = W10d_Sub(F(o, 0x58), F(rec, 0x18));
            if ((double)r * (double)r >= W10d_Dist2(dx, dy, dz)) {
                if (i == GS32(0x1017b8c8) && (S16(o, 0x1c) & 0x2000) != 0 && (S16(rec, 0x24) & 0x20) == 0 &&
                    U32(rec, 0) != 0) {
                    U16(rec, 0x24) |= 0x20;
                    U16(rec, 0x26) |= (uint16_t)(1u << ((uint32_t)side & 31u));
                    Sim_1000ebfb(SIM_PTR(const char, 0x10149c20), 0xfffff63cu, 0, 5, 0x50);
                }
                return 1;
            }
            /* outside: the entity's count +0x16c; at 2 its block +0x28 is scored to nobody (-2) */
            U32(o, 0x16c) += 1;
            if (U32(o, 0x16c) == 2 && U32(o, 0x28) != 0)
                Score_RecordKill(-2, BYTES(U32(o, 0x28)));
        }
        return 0;
    }
    return 0;
}
/* dx * dx + c * c + dz * dz as the x87 at 53-bit precision (the height term is the constant c squared) */
static double W10d_Dist2Flat(float dx, float c, float dz)
{
    return (double)dx * (double)dx + (double)c * (double)c + (double)dz * (double)dz;
}

// FUNCTION: HGSIM 0x1007a005
int __cdecl Sim_1007a005(const uint8_t *id, int32_t side)
{
    uint32_t index = id[0];
    uint32_t kind = (uint32_t)id[1] << 8;
    /* the side's entity, read before the side is tested (as the original does) */
    uint32_t me = *(volatile uint32_t *)&PTRTAB(G32(0x109b7d40 + (uint32_t)side * 0x6cu));
    uint32_t o, rec;
    float px, py, pz, r, dx, dy, dz;
    double d2;

    if (GS32(0x109b7d40 + (uint32_t)side * 0x6cu) < 0 && G32(0x1015ec58) == 0)
        return 0;
    if (kind == 0x200) {
        o = PTRTAB(index);
        if (Sim_10079feb(S32(me, 0)) == Sim_10079feb(S32(o, 0))) {
            if ((S16(o, 0x1c) & 6) != 0)
                return 0;
            dz = W10d_Sub(F(me, 0x60), F(o, 0x60));
            dy = W10d_Sub(F(me, 0x5c), F(o, 0x5c));
            dx = W10d_Sub(F(me, 0x58), F(o, 0x58));
            d2 = W10d_Dist2(dx, dy, dz);
        } else {
            if ((S16(o, 0x1c) & 6) != 0)
                return 0;
            dz = W10d_Sub(F(me, 0x60), F(o, 0x60));
            dx = W10d_Sub(F(me, 0x58), F(o, 0x58));
            d2 = W10d_Dist2Flat(dx, GF(0x1012b848), dz);
        }
        return (double)GF(0x1012b844) * (double)GF(0x1012b844) >= d2;
    }
    if (kind == 0x400) {
        rec = (uint32_t)(uintptr_t)GBYTES(0x109e9ba0u + index * 0x50u);
        Sim_100b53f0(S32(rec, 8), (uint32_t *)&px, (uint32_t *)&py, (uint32_t *)&pz);
        if (Sim_10079feb(S32(me, 0)) != 0) {
            dz = W10d_Sub(F(me, 0x60), pz);
            dx = W10d_Sub(F(me, 0x58), px);
            d2 = W10d_Dist2Flat(dx, GF(0x1012b848), dz);
        } else {
            dz = W10d_Sub(F(me, 0x60), pz);
            dy = W10d_Sub(F(me, 0x5c), py);
            dx = W10d_Sub(F(me, 0x58), px);
            d2 = W10d_Dist2(dx, dy, dz);
        }
        return (double)GF(0x1012b84c) * (double)GF(0x1012b84c) >= d2;
    }
    if (kind == 0x100) {
        rec = (uint32_t)(uintptr_t)GBYTES(0x109a8640u + index * 0x54u);
        r = F(rec, 0x10);
        if (!(r > GF(0x1012b848)))
            r = 20000.0f;     /* 0x469c4000 */
        if (Sim_10079feb(S32(me, 0)) != 0) {
            dz = W10d_Sub(F(me, 0x60), F(rec, 0x20));
            dx = W10d_Sub(F(me, 0x58), F(rec, 0x18));
            d2 = W10d_Dist2Flat(dx, GF(0x1012b848), dz);
        } else {
            dz = W10d_Sub(F(me, 0x60), F(rec, 0x20));
            dy = W10d_Sub(F(me, 0x5c), F(rec, 0x1c));
            dx = W10d_Sub(F(me, 0x58), F(rec, 0x18));
            d2 = W10d_Dist2(dx, dy, dz);
        }
        if (!((double)r * (double)r >= d2))
            return 0;
        if (G32(0x109b7d40 + (uint32_t)side * 0x6cu) == G32(0x1017b8c8) && (S16(me, 0x1c) & 0x2000) != 0 &&
            (S16(rec, 0x24) & 0x20) == 0 && U32(rec, 0) != 0) {
            U16(rec, 0x24) |= 0x20;
            U16(rec, 0x26) |= (uint16_t)(1u << (U32(me, 8) & 31u));
            Sim_1000ebfb(SIM_PTR(const char, 0x10149c08), 0xfffff63cu, 0, 5, 0x50);
        }
        return 1;
    }
    return 0;
}