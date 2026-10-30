#include "w9e.h"
#include "w3o.h"
#include "../util/access.h"

#define POOL(i) (GBYTES(0x101ce4a0) + (uint32_t)(i) * 0x18)

/* One plane: keeps the vertices with keep(prev) and adds the crossing of each edge whose ends differ in
 * side(.) (Sim_100a2c08(mode, value, cur, prev, new)). Returns the number of vertices written to out. */
#define PLANE(KEEP, SIDE, MODE, VALUE)                                                              \
    do {                                                                                            \
        kept = 0;                                                                                   \
        for (n = count; n != 0; n--, cur++) {                                                       \
            const uint8_t *p = *prev, *c = *cur;                                                    \
            if (KEEP(p)) {                                                                          \
                *out++ = *prev;                                                                     \
                kept++;                                                                             \
            }                                                                                       \
            if ((SIDE(p) && !SIDE(c)) || (!SIDE(p) && SIDE(c))) {                                   \
                Sim_100a2c08(MODE, VALUE, (const int *)c, (const int *)p, (int *)POOL(next));       \
                *out++ = POOL(next);                                                                \
                next++;                                                                             \
                kept++;                                                                             \
            }                                                                                       \
            prev = cur;                                                                             \
        }                                                                                           \
    } while (0)

#define X_LE_MAX(v) (S32(v, 0) <= 0x2000)
#define X_GE_MIN(v) (S32(v, 0) >= -0x2000)
#define Y_LE_MAX(v) (S32(v, 4) <= 0x2000)
#define Y_LT_MAX(v) (S32(v, 4) < 0x2000)
#define Y_GE_MIN(v) (S32(v, 4) >= -0x2000)

// FUNCTION: HGSIM 0x100a2d91
uint32_t __cdecl Clip_Polygon2D(uint32_t count, uint32_t planes)
{
    uint8_t *bufA[256], *bufB[256];
    uint8_t **prev, **cur, **out;
    uint32_t next = count; /* the next free vertex of the pool */
    uint32_t passes = 0;
    uint32_t n, i, kept;

    for (i = 0; i < count; i++)
        bufA[i] = POOL(i);
    prev = bufA + (count - 1);
    cur = bufA;
    out = bufB;
    if ((planes & 1) != 0) {
        PLANE(X_LE_MAX, X_LE_MAX, 1, 0x2000);
        if (kept == 0)
            return 0xffffffffu;
        count = kept;
        passes++;
        if ((passes & 1) != 0) {
            prev = out - 1;
            cur = bufB;
            out = bufA;
        } else {
            prev = bufA + (count - 1);
            cur = bufA;
            out = bufB;
        }
    }
    if ((planes & 4) != 0) {
        PLANE(Y_GE_MIN, Y_GE_MIN, 2, -0x2000);
        if (kept == 0)
            return 0xffffffffu;
        count = kept;
        passes++;
        if ((passes & 1) != 0) {
            prev = out - 1;
            cur = bufB;
            out = bufA;
        } else {
            prev = bufA + (count - 1);
            cur = bufA;
            out = bufB;
        }
    }
    if ((planes & 2) != 0) {
        PLANE(X_GE_MIN, X_GE_MIN, 1, -0x2000);
        if (kept == 0)
            return 0xffffffffu;
        count = kept;
        passes++;
        if ((passes & 1) != 0) {
            prev = out - 1;
            cur = bufB;
            out = bufA;
        } else {
            prev = bufA + (count - 1);
            cur = bufA;
            out = bufB;
        }
    }
    if ((planes & 8) != 0) {
        PLANE(Y_LT_MAX, Y_LE_MAX, 2, 0x2000);
        if (kept == 0)
            return 0xffffffffu;
        count = kept;
        passes++;
        cur = (passes & 1) != 0 ? bufB : bufA;
    }
    if (passes == 0)
        return 0;
    for (i = 0; i < count; i++) {
        const u32a *src = (const u32a *)cur[i];
        u32a *dst = (u32a *)(GBYTES(0x101ccca0) + i * 0x18);

        dst[0] = src[0];
        dst[1] = src[1];
        dst[2] = src[2];
        dst[3] = src[3];
        dst[4] = src[4];
        dst[5] = src[5];
    }
    return count;
}
