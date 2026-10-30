#include <string.h>
#include "w7s.h"
#include "../util/access.h"

/*
 * The original does these tests with x87 compares of doubles loaded from memory (exact) and only copies
 * stored bit patterns, so double compares give the same answers bit for bit. "fcomp" sets C0 for "less" and
 * also for unordered (NaN), C3 for "equal" and also for unordered; the conditions below keep that.
 */
#define ZERO (*SIM_PTR(const double, 0x1012c140))   /* 0.0 */
#define ONE (*SIM_PTR(const double, 0x1012c148))    /* 1.0 */

/* The 8 bytes at addr. */
static void copy_value(double *res, uint32_t addr)
{
    memcpy(res, SIM_PTR(const uint8_t, addr), 8);
}

/* The double at addr after an x87 load, fchs and store: the sign flips. (A signalling NaN would be made
 * quiet by the load; the only caller passes 0x10159988, the constant +infinity, which nothing writes.) */
static void copy_negated(double *res, uint32_t addr)
{
    uint32_t w[2];

    memcpy(w, SIM_PTR(const uint8_t, addr), 8);
    w[1] ^= 0x80000000u;
    memcpy(res, w, 8);
}

static void set_words(double *res, uint32_t lo, uint32_t hi)
{
    uint32_t w[2];

    w[0] = lo;
    w[1] = hi;
    memcpy(res, w, 8);
}

// FUNCTION: HGSIM 0x101062b0
int32_t __cdecl Sim_101062b0(double x, double y, double *res)
{
    uint32_t xw[2];
    uint32_t yw[2];
    double ax;

    memcpy(xw, &x, 8);
    memcpy(yw, &y, 8);
    ax = !(x >= ZERO) ? -x : x;
    if (yw[1] == 0x7ff00000u && yw[0] == 0) {
        if (ax > ONE) {
            copy_value(res, 0x10159988);
            return 0;
        }
        if (!(ax >= ONE)) {
            set_words(res, 0, 0);
            return 0;
        }
        copy_value(res, 0x10159990);
        return 1;
    }
    if (yw[1] == 0xfff00000u && yw[0] == 0) {
        if (ax > ONE) {
            set_words(res, 0, 0);
            return 0;
        }
        if (!(ax >= ONE)) {
            copy_value(res, 0x10159988);
            return 0;
        }
        copy_value(res, 0x10159990);
        return 1;
    }
    if (xw[1] == 0x7ff00000u && xw[0] == 0) {
        if (y > ZERO)
            copy_value(res, 0x10159988);
        else if (!(y >= ZERO))
            set_words(res, 0, 0);
        else
            set_words(res, 0, 0x3ff00000u);
        return 0;
    }
    if (xw[1] == 0xfff00000u && xw[0] == 0) {
        int32_t odd = W7S_FN_101064e0(y);

        if (y > ZERO) {
            if (odd == 1)
                copy_negated(res, 0x10159988);
            else
                copy_value(res, 0x10159988);
        } else if (!(y >= ZERO)) {
            if (odd == 1)
                copy_value(res, 0x101599a8);
            else
                set_words(res, 0, 0);
        } else
            set_words(res, 0, 0x3ff00000u);
        return 0;
    }
    return 0;
}
