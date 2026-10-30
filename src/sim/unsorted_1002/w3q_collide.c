#include "w3q.h"
#include "../unsorted_1003/w3q.h"
#include "../util/w3q.h"

#define CONST_C4 GF32(0x101112c4)
#define CONST_C0 GF32(0x101112c0)
#define CONST_D4 GF32(0x101112d4)

/* Vertex number n of a polygon's vertex list: 0x58 bytes each. */
#define LISTED_VERTEX(list, verts, n) (BYTES(verts) + (uint32_t)(list)[n] * 0x58)

// FUNCTION: HGSIM 0x1002d8be
int __cdecl Sim_1002d8be(const void *poly, const void *verts, float x, float y, float z, float *out)
{
    const uint8_t *v = BYTES(verts) + (uint32_t)BYTES(poly)[U32(poly, 4)] * 0x58;
    double s = (double)F32A(poly, 0x18) * ((double)x - (double)F32A(v, 0xc));

    s = s + (double)F32A(poly, 0x20) * ((double)z - (double)F32A(v, 0x14));
    s = (s + (double)CONST_C4) / (double)F32A(poly, 0x1c);
    *out = (float)((double)F32A(v, 0x10) - s);
    G32(0x1015dd40) = U32(poly, 0x18);
    G32(0x1015dd44) = U32(poly, 0x1c);
    G32(0x1015dd48) = U32(poly, 0x20);
    return X87_BELOW(y, *out);
}

/*
 * Shared body of Sim_1002e9f5 / Sim_1002ed0b; `axis` is the offset of the second
 * coordinate in a vertex (0x14 or 0x10), the first one is at +0xc.
 */
static int PolygonContains(const void *poly, const void *verts, float a, float b, unsigned axis)
{
    unsigned count = U16(poly, 2);
    const uint8_t *list;
    const uint8_t *prev;
    const uint8_t *cur;
    unsigned flags = 0;
    unsigned i;
    int seenBelow = 0; /* the original's [ebp-0x24] */
    int seenAbove = 0; /* the original's [ebp-0x18] */

    if ((int)count < 3)
        return 0;
    list = BYTES(poly) + U32(poly, 4);

    /* Is there a vertex on every side of the point, in a and in b? */
    for (i = count; i != 0;) {
        i--;
        cur = LISTED_VERTEX(list, verts, i);
        flags |= X87_ABOVE(F32A(cur, 0xc), a) ? 2 : 1;
        flags |= X87_ABOVE(F32A(cur, axis), b) ? 8 : 4;
        if (flags == 0xf)
            break;
    }
    if (flags != 0xf)
        return 0;

    prev = LISTED_VERTEX(list, verts, 0);
    for (i = count; i != 0;) {
        double t;
        union {
            float f;
            uint32_t bits;
        } crossing;

        i--;
        cur = LISTED_VERTEX(list, verts, i);
        if (X87_BELOW(F32A(cur, axis), b) && X87_BELOW(F32A(prev, axis), b)) {
            prev = cur;
            continue;
        }
        if (X87_ABOVE(F32A(cur, axis), b) && X87_ABOVE(F32A(prev, axis), b)) {
            prev = cur;
            continue;
        }
        if (X87_EQUAL(F32A(cur, axis), b) && X87_EQUAL(F32A(prev, axis), b)) {
            /* both ends on the line: an edge along it */
            if (X87_BELOW(F32A(cur, 0xc), a) && X87_BELOW(F32A(prev, 0xc), a))
                return 0;
            if (X87_NOT_ABOVE(F32A(cur, 0xc), a) || X87_NOT_ABOVE(F32A(prev, 0xc), a))
                return 1;
            return 0;
        }
        if (X87_EQUAL(F32A(cur, axis), b)) {
            prev = cur;
            continue;
        }
        if (X87_BELOW(F32A(cur, 0xc), a) && X87_BELOW(F32A(prev, 0xc), a)) {
            /* edge entirely on one side of a */
            if (seenBelow)
                return 0;
            if (seenAbove)
                return 1;
            seenBelow = 1;
            prev = cur;
            continue;
        }
        if (X87_ABOVE(F32A(cur, 0xc), a) && X87_ABOVE(F32A(prev, 0xc), a)) {
            if (seenAbove)
                return 0;
            if (seenBelow)
                return 1;
            seenAbove = 1;
            prev = cur;
            continue;
        }
        /* the edge crosses the line through b on both sides of a: where, relative to a? */
        t = ((double)F32A(prev, 0xc) - (double)F32A(cur, 0xc)) * ((double)b - (double)F32A(cur, axis));
        t = t / ((double)F32A(prev, axis) - (double)F32A(cur, axis));
        t = t + ((double)F32A(cur, 0xc) - (double)a);
        crossing.f = (float)t;
        if (crossing.bits & 0x80000000u) {
            if (seenBelow)
                return 0;
            if (seenAbove)
                return 1;
            seenBelow = 1;
        } else if (X87_NOT_ABOVE(crossing.f, CONST_D4)) {
            return 1;
        } else {
            if (seenAbove)
                return 0;
            if (seenBelow)
                return 1;
            seenAbove = 1;
        }
        prev = cur;
    }
    return 0;
}

// FUNCTION: HGSIM 0x1002e9f5
int __cdecl Sim_1002e9f5(const void *poly, const void *verts, float a, float b)
{
    return PolygonContains(poly, verts, a, b, 0x14);
}

// FUNCTION: HGSIM 0x1002ed0b
int __cdecl Sim_1002ed0b(const void *poly, const void *verts, float a, float b)
{
    return PolygonContains(poly, verts, a, b, 0x10);
}

// FUNCTION: HGSIM 0x1002d276
int __cdecl Sim_1002d276(const void *obj, float a, float b, float c)
{
    int i = 0;
    int done = 0;

    while (!done) {
        const uint8_t *mesh = BYTES(U32(obj, 0x30));
        const uint8_t *poly = mesh + U32(mesh, 8) + (uint32_t)i * 0x28;

        if (X87_ABOVE(F32A(poly, 0x1c), CONST_C0) && Sim_1002e9f5(poly, mesh + 0x18, a, c)) {
            float height;

            return Sim_1002d8be(poly, mesh + 0x18, a, b, c, &height) != 0;
        }
        i++;
        if (i == S16(U32(obj, 0x30), 6))
            done = 1;
    }
    return 0;
}

// FUNCTION: HGSIM 0x1002d4a7
int __cdecl Sim_1002d4a7(const void *obj, float a, float b, float c)
{
    int i = 0;
    int done = 0;
    const uint8_t *verts = BYTES(U32(obj, 0x30)) + 0x18;

    while (!done) {
        const uint8_t *mesh = BYTES(U32(obj, 0x30));
        const uint8_t *poly = mesh + U32(mesh, 8) + (uint32_t)i * 0x28;

        if (X87_BELOW(F32A(poly, 0x1c), CONST_C0) && Sim_1002e9f5(poly, verts, a, c)) {
            const uint8_t *v = verts + (uint32_t)BYTES(poly)[U32(poly, 4)] * 0x58;
            double s = (double)F32A(poly, 0x18) * ((double)a - (double)F32A(v, 0xc));
            float height;

            s = s + (double)F32A(poly, 0x20) * ((double)c - (double)F32A(v, 0x14));
            s = (s + (double)CONST_C0) / (double)F32A(poly, 0x1c);
            height = (float)((double)F32A(v, 0x10) - s);
            G32(0x1015dd40) = U32(poly, 0x18);
            G32(0x1015dd44) = U32(poly, 0x1c);
            G32(0x1015dd48) = U32(poly, 0x20);
            return X87_ABOVE(b, height);
        }
        i++;
        if (i == S16(U32(obj, 0x30), 6))
            done = 1;
    }
    return 0;
}

// FUNCTION: HGSIM 0x1002d3f8
int __cdecl Sim_1002d3f8(const void *obj, float a, float b, float c)
{
    uint32_t mesh = U32(obj, 0x2c);

    if (U32(obj, 0x98) != 0)
        return Sim_100310e2(BYTES(U32(obj, 0x98)), BYTES(mesh), a, b, c) & 1;
    return Sim_1002d276(obj, a, b, c);
}

// FUNCTION: HGSIM 0x1002d479
int __cdecl Sim_1002d479(const void *obj, float a, float b, float c, float *d)
{
    return Sim_100309aa(BYTES(U32(obj, 0x98)), BYTES(U32(obj, 0x2c)), a, b, c, d);
}
