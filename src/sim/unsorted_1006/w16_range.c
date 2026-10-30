#include <string.h>

#include "w16.h"
#include "w10.h"
#include "w3o.h"
#include "w3p.h"
#include "w6e.h"
#include "w8w.h"
#include "w9d.h"
#include "../util/access.h"
#include "../util/util.h"
#include "../util/w3o.h"
#include "../unsorted_1002/w7l.h"
#include "../unsorted_1009/w3o.h"
#include "../unsorted_1009/w3p.h"
#include "../unsorted_1010/w6k.h"
#include "../unsorted_100a/w5x.h"
#include "../unsorted_1008/w3o.h"
#include "../render/w7o.h"
#include "../view/view.h"

// FUNCTION: HGSIM 0x100687a3
int __cdecl Sim_100687a3(const float *box, float x, float y, float z, float radius, float *outX, float *outY,
                         float *outZ)
{
    float r2, dx, dy, dz;
    double d2;

    if (box == 0 || outX == 0 || outY == 0 || outZ == 0)
        return 0;
    /* (the original's checks of three unset locals against 0.0 are left out: see w16.h) */
    if (Sim_10068716(box, x, y, z, outX, outY, outZ) == 0)
        return 0;
    r2 = (float)((double)radius * radius);
    dx = (float)((double)x - *outX);
    dy = (float)((double)y - *outY);
    dz = (float)((double)z - *outZ);
    d2 = (double)dx * dx + (double)dy * dy;
    d2 = d2 + (double)dz * dz;
    return !(d2 > (double)r2);
}

// FUNCTION: HGSIM 0x100689c0
int __cdecl Sim_100689c0(const uint32_t *a, const uint32_t *b, float x, float y, float z, float *outX, float *outY,
                         float *outZ)
{
    float p[3], q[3];
    union {
        float f;
        uint32_t u;
    } r[3];
    uint32_t node;
    union {
        float f;
        uint32_t u;
    } radius;

    (void)x;
    (void)y;
    (void)z;
    if (a == 0 || b == 0)
        return 0;
    node = U32(*a, 0x48);
    Sim_100a9cfb((void *)(uintptr_t)node, (uint32_t *)&p[0], (uint32_t *)&p[1], (uint32_t *)&p[2]);
    Sim_100a9ca3((void *)(uintptr_t)node, (uint32_t)(uintptr_t)&q[0], (uint32_t)(uintptr_t)&q[1],
                 (uint32_t)(uintptr_t)&q[2]);
    Sim_10068e1d(FL(*b, 0x58), FL(*b, 0x5c), FL(*b, 0x60), -p[0], -p[1], -p[2], &r[0].f,
                 &r[1].f, &r[2].f);
    Sim_10068d76(r[0].f, r[1].f, r[2].f, -q[0], -q[1], -q[2], &r[0].u, &r[1].u, &r[2].u);
    radius.u = U32(b, 0x53c);
    return Sim_100687a3((const float *)(uintptr_t)(*a + 0x284), r[0].f, r[1].f, r[2].f, radius.f, outX, outY, outZ);
}

/* The original's float arithmetic: each operation at 53-bit precision, stored as a float. */
static float W16_Sub(float a, float b)
{
    return (float)((double)a - b);
}

/* The push after a contact with the gear `gear` (record `other`): the direction [0x1015dd58..60] from the offset
 * (dx, dy, dz) and its length `dist`, the new position at `reach` from the other unit's position, and the record's
 * push +0x548..+0x550 from half the speed difference (at least 1). A box unit (selfBox) keeps its position. */
static void W16_Push(uint8_t *self, const uint8_t *other, const uint8_t *gear, float *x, float *y, float *z,
                     const float d[3], float dist, float reach, int selfBox, const uint32_t saved[3])
{
    double q;
    float k;

    if (!(dist > GFL(0x1012b5f8))) {
        G32(0x1015dd5c) = 0;
        G32(0x1015dd58) = G32(0x1015dd5c);
        G32(0x1015dd60) = 0x3f800000;
    } else {
        GFL(0x1015dd58) = (float)((double)d[0] / dist);
        GFL(0x1015dd5c) = (float)((double)d[1] / dist);
        GFL(0x1015dd60) = (float)((double)d[2] / dist);
    }
    *x = (float)((double)GFL(0x1015dd58) * reach + FL(gear, 0x58));
    *y = (float)((double)GFL(0x1015dd5c) * reach + FL(gear, 0x5c));
    *z = (float)((double)GFL(0x1015dd60) * reach + FL(gear, 0x60));
    q = Sim_10084a94(W16_Sub(FL(self, 0x554), FL(other, 0x554)), W16_Sub(FL(self, 0x558), FL(other, 0x558)),
                     W16_Sub(FL(self, 0x55c), FL(other, 0x55c))) /
        GFL(0x1012b604);
    k = (float)q;
    if (!(q >= GFL(0x1012b5fc)))
        k = 1.0f;
    FL(self, 0x548) = (float)((double)GFL(0x1015dd58) * k);
    FL(self, 0x54c) = (float)((double)GFL(0x1015dd5c) * k);
    FL(self, 0x550) = (float)((double)GFL(0x1015dd60) * k);
    if (selfBox) {
        memcpy(x, &saved[0], 4);
        memcpy(y, &saved[1], 4);
        memcpy(z, &saved[2], 4);
    }
}

// FUNCTION: HGSIM 0x10066907
int __cdecl Sim_10066907(uint8_t *self, float *x, float *y, float *z, uint32_t *hit)
{
    uint32_t saved[3], selfId = U32(U32(self, 0), 4);
    int selfBox = U32(U32(self, 0), 0x29c) == 1, result = 0, i;
    float selfRadius = FL(self, 0x53c);

    memcpy(&saved[0], x, 4);
    memcpy(&saved[1], y, 4);
    memcpy(&saved[2], z, 4);
    for (i = 0; i < GS32(0x1015dc64); i++) {
        uint8_t *gear = BYTES(PTRTAB(i)), *other;
        int otherBox, counted;
        float d[3], dist, reach, out[3];
        double len;

        if (gear == 0 || U32(gear, 4) == selfId)
            continue;
        other = BYTES(U32(gear, 0x28));
        otherBox = U32(gear, 0x29c) == 1;
        counted = Sim_10079feb((int32_t)U32(gear, 0));
        if (other == 0 || (counted != 0 && U32(other, 0x58c) != 2) || (U16(other, 0x560) & 0x100) != 0 ||
            (S16(U32(other, 0), 0x1c) & 0x4000) != 0)
            continue;
        if (U32(gear, 0x84) == selfId && i < (int)selfId) {
            if (U32(other, 0x4f0) == 4) {
                Sim_1006863d((uint32_t *)other);
                return result;
            }
            result = 1;
            *hit = (uint32_t)(uintptr_t)gear;
            d[0] = W16_Sub(*x, FL(gear, 0x58));
            d[1] = W16_Sub(*y, FL(gear, 0x5c));
            d[2] = W16_Sub(*z, FL(gear, 0x60));
            dist = (float)Sim_10084a94(d[0], d[1], d[2]);
            reach = (float)((double)selfRadius + FL(other, 0x53c));
            W16_Push(self, other, gear, x, y, z, d, dist, reach, selfBox, saved);
            return result;
        }
        d[0] = W16_Sub(*x, FL(gear, 0x58));
        d[1] = W16_Sub(*y, FL(gear, 0x5c));
        d[2] = W16_Sub(*z, FL(gear, 0x60));
        reach = (float)((double)selfRadius + FL(other, 0x53c));
        len = Sim_10084a94(d[0], d[1], d[2]);
        dist = (float)len;
        if (len >= reach)
            continue;
        if (!selfBox && !otherBox) {
            if (U32(other, 0x4f0) == 4) {
                Sim_1006863d((uint32_t *)other);
                return result;
            }
            result = 1;
            *hit = (uint32_t)(uintptr_t)gear;
            W16_Push(self, other, gear, x, y, z, d, dist, reach, selfBox, saved);
            return result;
        }
        if (!selfBox)
            result = Sim_100688a3(self, (const uint32_t *)other, *x, *y, *z, &out[0], &out[1], &out[2]);
        else if (!otherBox)
            result = Sim_100689c0((const uint32_t *)self, (const uint32_t *)other, *x, *y, *z, &out[0], &out[1],
                                  &out[2]);
        else
            result = Sim_100690d4((uint8_t **)self, (uint8_t **)other, &out[0], &out[1], &out[2]);
        if (result != 1)
            continue;
        if (U32(other, 0x4f0) == 4) {
            result = 0;
            Sim_1006863d((uint32_t *)other);
            continue;
        }
        result = 1;
        *hit = (uint32_t)(uintptr_t)gear;
        W16_Push(self, other, gear, x, y, z, d, dist, reach, selfBox, saved);
    }
    return result;
}

#define W16_FABS SIM_FN(double(__cdecl *)(double), 0x100fcb70)

/* The original's x87 test v == c, which an unordered compare (NaN) also passes. */
static int W16_Zero(double v, double c)
{
    return !(v < c) && !(v > c);
}

/* The original's test of a move component: |v| below [0x1012b5f8], or NaN. */
static int W16_Tiny(float v)
{
    return !(W16_FABS((double)v) >= GFL(0x1012b5f8));
}

// FUNCTION: HGSIM 0x100663c0
int __cdecl Sim_100663c0(uint8_t *rec, uint32_t *hitObject, uint32_t *hitGear, float dx, float dy, float dz,
                         float *x, float *y, float *z)
{
    uint8_t *unit = BYTES(U32(rec, 0));
    float px = FL(unit, 0x58), py = FL(unit, 0x5c), pz = FL(unit, 0x60);
    float tx = (float)((double)px + dx), ty = (float)((double)py + dy), tz = (float)((double)pz + dz);
    float seg[14], seg2[14], len, k, reach, push[3], s, back, d80, rest;
    uint32_t object = 0, kind;
    int result = 0;
    double dot, q, left;

    *x = tx;
    *y = ty;
    *z = tz;
    kind = U32(unit, 0);
    if (U32(unit, 4) != G32(0x1017b8c8) && U32(rec, 0x4f0) != 2 && U32(rec, 0x4f0) != 4) {
        U32(rec, 0x4f4) = 0;
        return 0;
    }
    if (U32(rec, 0x58c) != 2 && (kind == 8 || kind == 5)) {
        result = 0;
    } else if (Sim_10066907(rec, x, y, z, hitGear) != 0) {
        U32(unit, 0x84) = U32(*hitGear, 4);
        result = 1;
        U32(rec, 0x4f4) += 1;
        *hitObject = 0;
    } else if (Sim_1006754e(rec, x, y, z, &object) != 0) {
        result = 1;
        U32(rec, 0x4f4) += 1;
        *hitObject = object;
    } else {
        if (W16_Tiny(dx) && W16_Tiny(dy) && W16_Tiny(dz)) {
            if (U32(rec, 0x4f4) != 0) {
                U32(rec, 0x4f4) += 1;
                return (int)U32(rec, 0x4f4);
            }
            return 0;
        }
        Sim_1009cab0(seg, px, py, pz, tx, ty, tz);
        len = Sim_1009cbde(seg);
        k = seg[10];
        if ((Bits_OfFloat(k) & 0x80000000u) != 0)
            k = -k;
        reach = (float)((double)FL(rec, 0x520) * k + ((double)GFL(0x1012b5fc) - k) * FL(rec, 0x53c));
        Sim_1009cd36(seg, (float)((double)len + reach));
        if (Sim_1002e362(seg, &object) != 0) {
            *hitObject = object;
            if (*hitObject != 0 && (kind == 10 || kind == 4) &&
                Sim_1010d240(BYTES(*hitObject) + 0xa0, SIM_PTR(const uint8_t, 0x10145200), 4) == 0) {
                result = 0;
            } else {
                result = 1;
                U32(rec, 0x4f4) += 1;
                if (W16_Zero((double)GFL(0x1015dd58), (double)GFL(0x1012b600)) &&
                    W16_Zero((double)GFL(0x1015dd5c), (double)GFL(0x1012b600)) &&
                    W16_Zero((double)GFL(0x1015dd60), (double)GFL(0x1012b600)) && object != 0) {
                    Sim_1009cab0(seg2, FL(object, 0x44), FL(object, 0x48), FL(object, 0x4c), seg[3], seg[4],
                                 seg[5]);
                    Sim_1009cbfe(seg2);
                    G32(0x1015dd58) = U32(seg2, 0x24);
                    G32(0x1015dd5c) = U32(seg2, 0x28);
                    G32(0x1015dd60) = U32(seg2, 0x2c);
                }
                dot = (double)GFL(0x1015dd58) * GFL(0x1012b604) * seg[9];
                dot = dot + (double)GFL(0x1015dd5c) * GFL(0x1012b604) * seg[10];
                dot = dot + (double)GFL(0x1015dd60) * GFL(0x1012b604) * seg[11];
                d80 = (float)dot;
                push[0] = (float)((double)seg[9] - dot * GFL(0x1015dd58));
                push[1] = (float)((double)seg[10] - (double)d80 * GFL(0x1015dd5c));
                push[2] = (float)((double)seg[11] - (double)d80 * GFL(0x1015dd60));
                q = Sim_10084a94(FL(rec, 0x554), FL(rec, 0x558), FL(rec, 0x55c)) / GFL(0x1012b608);
                s = (float)q;
                FL(rec, 0x548) = (float)((double)push[0] * s);
                FL(rec, 0x54c) = (float)((double)push[1] * s);
                FL(rec, 0x550) = (float)((double)push[2] * s);
                back = (float)((double)Sim_1009cbde(seg) - reach);
                Sim_1009cd36(seg, back);
                left = (double)len - Sim_1009cbde(seg);
                rest = (float)left;
                if (left > (double)GFL(0x1012b600)) {
                    *x = (float)((double)push[0] * rest / GFL(0x1012b608) + seg[3]);
                    *y = (float)((double)push[1] * rest / GFL(0x1012b608) + seg[4]);
                    *z = (float)((double)push[2] * rest / GFL(0x1012b608) + seg[5]);
                } else {
                    memcpy(x, &seg[3], 4);
                    memcpy(y, &seg[4], 4);
                    memcpy(z, &seg[5], 4);
                }
            }
        }
    }
    if (result == 0) {
        U32(rec, 0x4f4) = 0;
        *hitObject = 0;
    }
    return (int)U32(rec, 0x4f4);
}
