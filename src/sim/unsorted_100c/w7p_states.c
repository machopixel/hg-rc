/*
 * Unit state steps and probes (game logic). The x87 originals run with the Windows control word 0x27f (53-bit
 * mantissa); every float operation below is written in the same order and with the same rounding points
 * (float stores) so that SSE2 doubles give the same bits. The x87 compares are written so that a NaN takes the
 * same branch as in the original (fcomp sets C0 and C3 for unordered operands).
 */
#include <string.h>
#include "w7p.h"
#include "w2l.h"
#include "w3t.h"
#include "w2i.h"
#include "w5a.h"
#include "w5b.h"
#include "w6h.h"
#include "../util/access.h"
#include "../util/util.h"
#include "../unsorted_1000/w5b.h"
#include "../unsorted_1001/w3t.h"
#include "../unsorted_1001/w5c.h"
#include "../unsorted_1001/w5y.h"
#include "../unsorted_1002/w3o.h"
#include "../unsorted_1003/fns.h"
#include "../unsorted_1004/fns.h"
#include "../unsorted_1008/w3o.h"
#include "../unsorted_1009/w2e.h"
#include "../unsorted_1009/w3o.h"
#include "../unsorted_1009/w3p.h"
#include "../unsorted_1009/w6g.h"
#include "../unsorted_100a/w5x.h"
#include "../view/w3r.h"

#ifdef __GNUC__
typedef float __attribute__((may_alias)) w7p_f32;
#else
typedef float w7p_f32;
#endif

#define F32(p, off) (*(w7p_f32 *)(BYTES(p) + (off)))
#define GF32(addr) (*SIM_PTR(w7p_f32, addr))
#define GF64(addr) (*SIM_PTR(double, addr))
#define CLOCK GS32(0x1015dc78)
#define RECORD(unit) U32(unit, 0x54)
#define S8(p, off) (*(int8_t *)(BYTES(p) + (off)))

static uint32_t bits_of(float f)
{
    uint32_t u;

    memcpy(&u, &f, 4);
    return u;
}

/* fchs: the sign bit flipped (also of a NaN; written on the bits so that the compiler cannot turn -a + b into
 * b - a, which keeps the NaN's sign). */
static float negated(float f)
{
    uint32_t u = bits_of(f) ^ 0x80000000u;

    memcpy(&f, &u, 4);
    return f;
}

static int32_t ftol(double value)
{
    return (int32_t)(int64_t)value;
}

/* Sim_1000ae00(unit, Sim_10015266(unit)): the angle goes through a float store. */
static void turn(void *unit)
{
    Sim_1000ae00(unit, bits_of(Sim_10015266(unit)));
}

// FUNCTION: HGSIM 0x100c0bdd
int __cdecl Sim_100c0bdd(void *unit, int16_t key)
{
    int result = 0;

    Sim_1001403a(unit, key);
    turn(unit);
    switch (S32(unit, 0x218)) {
    case 0:
        if (Sim_100c3892(unit, 0x28) != 0 && Sim_100c25af(unit) != 0) {
            Sim_100c274a(unit, 1);
        } else {
            Sim_100c274a(unit, 0);
            U32(unit, 0x218) = 1;
            if (Sim_100c3892(unit, 0x14) != 0 && Sim_1009c929(4) == 0)
                U32(unit, 0x1fc) = 5;
            U32(unit, 0x204) = (uint32_t)CLOCK + 0xb5;
        }
        break;
    case 1:
        if (CLOCK > S32(unit, 0x204)) {
            U32(unit, 0x218) = 2;
            Sim_100c274a(unit, 0);
        } else {
            Sim_100c274a(unit, 1);
        }
        Sim_100144e9(unit);
        break;
    case 2:
        if (Sim_100c264a(unit) != 0)
            result = 1;
        break;
    }
    return result;
}

// FUNCTION: HGSIM 0x100c0dd7
int __cdecl Sim_100c0dd7(void *unit, int16_t key)
{
    float angle;
    float limit;
    uint32_t block;
    int16_t nearby;
    int result;

    Sim_1001403a(unit, key);
    angle = Sim_10015266(unit);
    Sim_1000ae00(unit, bits_of(angle));
    result = (U32(unit, 0x80) != 0 || U32(U32(unit, 0x28), 0x4f4) != 0) ? 1 : 0;
    if (S32(unit, 0x218) == 2)
        return result;
    angle = (float)W7P_FABS((double)angle);
    (void)*(volatile u32a *)(uintptr_t)(PTRTAB((int32_t)key & 0xff) + 0x28);
    block = U32(unit, 0x28);
    nearby = (angle > F32(block, 0x534) && (double)GF32(0x1012bb50) * F32(block, 0x534) > angle) ? 1 : 0;
    if (F32(unit, 0xd4) > GF32(0x1012bb10)) {
        float rate = Sim_100c3725(unit);

        limit = F32(unit, 0xd4) > GF32(0x1012bb54) ? 36.0f : 9.0f;
        if (rate > limit)
            Sim_100c3702(unit, nearby);
        else
            Sim_100c3702(unit, nearby == 0);
        Sim_100c274a(unit, 1);
        if (Sim_100c3892(unit, 0x28) == 0)
            result = 1;
    } else {
        if (S8(RECORD(unit), 0x1e) != 0)
            U32(unit, 0x218) = U32(unit, 0x218) + 1;
        Sim_100c274a(unit, 0);
        U8(RECORD(unit), 0x21) = 0;
        U8(RECORD(unit), 0x22) = 0;
    }
    return result;
}

static int search_step(void *unit, int mode)
{
    int result = 0;
    int32_t state = S32(unit, 0x218);

    switch (state) {
    case 1: {
        int index;

        U32(unit, 0x21c) = U32(unit, 0x21c) + 0x2710;
        index = Sim_100c43e1(unit, bits_of((float)S32(unit, 0x21c)), mode);
        if (index >= 0 && index < GS32(0x109e4cc0)) {
            state = 2;
            U32(unit, 0x23a) = (uint32_t)index;
        } else {
            state = 1;
        }
        break;
    }
    case 2:
        if (W7P_CALL_100c45dc(unit, Sim_10033c42((int)U32(unit, 0x23a)), BYTES(unit) + 0x224) == 1)
            state = 3;
        else
            state = 1;
        break;
    case 3:
        state = 0;
        result = 1;
        break;
    default:
        result = -1;
        state = 0;
        break;
    }
    Sim_1001403a(unit, S16(unit, 0x1aa));
    turn(unit);
    S32(unit, 0x218) = state;
    return result;
}

// FUNCTION: HGSIM 0x100c145b
int __cdecl Sim_100c145b(void *unit, int16_t key)
{
    (void)key;
    return search_step(unit, 1);
}

// FUNCTION: HGSIM 0x100c1609
int __cdecl Sim_100c1609(void *unit, int16_t key)
{
    (void)key;
    return search_step(unit, 2);
}

// FUNCTION: HGSIM 0x100c1c67
int __cdecl Sim_100c1c67(void *unit, int16_t key)
{
    int result;
    uint32_t block = U32(unit, 0x28);

    Sim_1001403a(unit, key);
    if (S32(unit, 0x218) == 1) {
        if (S32(block, 0x514) <= 0 || !(F32(unit, 0xd4) >= GF32(0x1012bb24))) {
            result = -1;
        } else {
            U32(unit, 0x218) = 2;
            result = 0;
        }
    } else if (S32(unit, 0x218) == 2) {
        if (W7P_CALL_100c2ae5(unit) == 0) {
            Sim_100144e9(unit);
            U32(RECORD(unit), 8) = 0x3c800000;
        }
        if (!(F32(unit, 0xd4) > GF32(0x1012bb24))) {
            U8(RECORD(unit), 0x1e) = 1;
            U32(block, 0x518) = 1;
            U32(unit, 0x218) = 3;
            U32(unit, 0x204) = (uint32_t)CLOCK + 0x5a8;
        }
        result = 0;
    } else if (S32(unit, 0x218) == 3) {
        if (S8(RECORD(unit), 0x1e) == 1)
            U8(RECORD(unit), 0x1e) = 0;
        if (CLOCK >= S32(unit, 0x204)) {
            U32(block, 0x518) = 0;
            U32(unit, 0x218) = 4;
            U32(unit, 0x204) = 0;
            result = 1;
        } else {
            if (CLOCK >= S32(unit, 0x200))
                U32(unit, 0x200) = (uint32_t)CLOCK + 0xb5;
            result = 0;
        }
    } else {
        if (S8(RECORD(unit), 0x1e) == 1) {
            U8(RECORD(unit), 0x1e) = 0;
            U32(block, 0x518) = 0;
        }
        result = -1;
    }
    turn(unit);
    return result;
}

// FUNCTION: HGSIM 0x100c1e69
int __cdecl Sim_100c1e69(void *unit, int16_t key)
{
    int result;
    uint32_t block = U32(unit, 0x28);

    Sim_1001403a(unit, key);
    if (S32(unit, 0x218) == 1) {
        if (S32(block, 0x514) <= 0 || F32(unit, 0xd4) > GF32(0x1012bb10)) {
            result = -1;
        } else {
            U32(unit, 0x218) = 2;
            U32(unit, 0x21c) = (uint32_t)CLOCK + 0x2d4;
            U8(RECORD(unit), 0x59) = 1;
            result = 0;
        }
    } else if (S32(unit, 0x218) == 2) {
        if (W7P_CALL_100c2ae5(unit) == 0) {
            Sim_100144e9(unit);
            U32(RECORD(unit), 8) = 0x3c800000;
        }
        if (CLOCK >= S32(unit, 0x21c)) {
            U8(RECORD(unit), 0x1e) = 1;
            U32(block, 0x518) = 1;
            U32(unit, 0x218) = 3;
            U32(unit, 0x204) = (uint32_t)CLOCK + 0x5a8;
        }
        result = 0;
    } else if (S32(unit, 0x218) == 3) {
        if (S8(RECORD(unit), 0x1e) == 1)
            U8(RECORD(unit), 0x1e) = 0;
        if (CLOCK >= S32(unit, 0x204)) {
            U32(block, 0x518) = 0;
            U32(unit, 0x218) = 4;
            U32(unit, 0x204) = 0;
            U8(RECORD(unit), 0x59) = 0;
            result = 1;
        } else {
            if (CLOCK >= S32(unit, 0x200))
                U32(unit, 0x200) = (uint32_t)CLOCK + 0xb5;
            result = 0;
        }
    } else {
        if (S8(RECORD(unit), 0x1e) == 1) {
            U8(RECORD(unit), 0x1e) = 0;
            U32(block, 0x518) = 0;
        }
        U8(RECORD(unit), 0x59) = 0;
        result = -1;
    }
    turn(unit);
    return result;
}

static void stop_record(void *unit)
{
    U8(RECORD(unit), 0x56) = 0;
    U32(RECORD(unit), 8) = 0;
    U8(RECORD(unit), 0x59) = 0;
}

// FUNCTION: HGSIM 0x100c209c
int __cdecl Sim_100c209c(void *unit, int16_t key)
{
    int result = 0;

    Sim_1001403a(unit, key);
    turn(unit);
    if (S32(unit, 0x218) == 1) {
        U32(RECORD(unit), 8) = 0x3b800000;
        U8(RECORD(unit), 0x59) = 1;
        U32(unit, 0x218) = 2;
        U32(unit, 0x204) = (uint32_t)CLOCK + 0xb5;
    } else if (S32(unit, 0x218) == 2) {
        if (CLOCK >= S32(unit, 0x204)) {
            U8(RECORD(unit), 0x56) = 1;
            U32(unit, 0x218) = 3;
            U32(unit, 0x204) = (uint32_t)CLOCK + 0x17feb;
        }
    } else if (S32(unit, 0x218) == 3) {
        if (W7P_CALL_100c2ae5(unit) == 0) {
            Sim_100144e9(unit);
            U32(RECORD(unit), 8) = 0x3c800000;
            if (CLOCK >= S32(unit, 0x204)) {
                stop_record(unit);
                U32(unit, 0x204) = 0;
                U32(unit, 0x218) = 4;
                result = 1;
            }
        } else {
            stop_record(unit);
            U32(unit, 0x204) = 0;
            U32(unit, 0x218) = 4;
            result = -1;
        }
    } else {
        stop_record(unit);
        result = -1;
    }
    return result;
}

/* 1 when the probe at height h must stop: the float 0x1015dd50 below 0.766, or |h - unit +0x7c| above 2000. */
static int probe_blocked(float h, void *unit)
{
    int steep = !(GF32(0x1015dd50) >= GF32(0x1012bb84));
    int far = W7P_FABS((double)h - F32(unit, 0x7c)) > (double)GF32(0x1012bb48);

    return steep != 0 || far != 0;
}

// FUNCTION: HGSIM 0x100c292c
int __cdecl Sim_100c292c(void *seg, void *unit)
{
    uint8_t part[0x38];
    float h = (float)Sim_1002d961(F32(seg, 0xc), F32(seg, 0x10), F32(seg, 0x14));

    if (probe_blocked(h, unit)) {
        U32(unit, 0x24e) = U32(seg, 0xc);
        U32(unit, 0x252) = U32(seg, 0x10);
        U32(unit, 0x256) = U32(seg, 0x14);
        return 1;
    }
    Sim_1009cfbd(part, seg);
    Sim_1009cd36(part, F32(unit, 0x246));
    h = (float)Sim_1002d961(F32(part, 0xc), F32(part, 0x10), F32(part, 0x14));
    if (probe_blocked(h, unit)) {
        U32(unit, 0x24e) = U32(part, 0xc);
        U32(unit, 0x252) = U32(part, 0x10);
        U32(unit, 0x256) = U32(part, 0x14);
        return 1;
    }
    return 0;
}

static int direction(int side, int16_t dir)
{
    return side >= 0 ? (int)dir : (0x10 - (int)dir) % 16;
}

// FUNCTION: HGSIM 0x100c31a6
void __cdecl Sim_100c31a6(void *unit, void *seg, int side, int16_t dir, float scale, int flag)
{
    uint32_t block = U32(unit, 0x28);
    const float *m = Sim_100aa863((void *)(uintptr_t)U32(unit, 0x48));
    float x = GF32(0x101516f8 + (uint32_t)direction(side, dir) * 8);
    float z = GF32(0x101516fc + (uint32_t)direction(side, dir) * 8);
    float y = 0.0f;
    float sx, sy, sz;

    if (x < GF32(0x1012bb18) || x > GF32(0x1012bb18))
        x = (float)((double)scale / x);
    if (z < GF32(0x1012bb18) || z > GF32(0x1012bb18))
        z = (float)((double)scale / z);
    Mat3x4_TransformPoint(m, &x, &y, &z);
    if (flag != 0) {
        if (side >= 0)
            sx = (float)((double)F32(block, 0x53c) - GF32(0x1012bb20));
        else
            sx = (float)((double)negated(F32(block, 0x53c)) + GF32(0x1012bb20));
        sz = 0.0f;
        Mat3x4_TransformPoint(m, &sx, &y, &sz);
        sy = F32(unit, 0x5c);
    } else {
        sy = F32(unit, 0x5c);
        sx = F32(unit, 0x58);
        sz = F32(unit, 0x60);
    }
    Sim_1009cab0(seg, sx, sy, sz, x, sy, z);
    if (Sim_10084a94((float)((double)x - sx), 0.0f, (float)((double)z - sz)) > (double)GF32(0x1012bb4c))
        Sim_1009cc82(seg);
    else
        Sim_1009cbfe(seg);
}

// FUNCTION: HGSIM 0x100c376e
void __cdecl Sim_100c376e(void *unit)
{
    uint32_t block = U32(unit, 0x28);
    float a;

    if ((U32(unit, 0x26c) & 1) == 0 || S16(unit, 0x25e) != 0 || S16(unit, 0x1f8) == 10)
        return;
    Sim_1001403a(unit, S16(unit, 0x1a8));
    a = (float)W7P_FABS((double)Sim_10015266(unit));
    if (a >= F32(block, 0x534)
        || (-(double)F32(block, 0x534) >= a
            && (F32(RECORD(unit), 8) < GF32(0x1012bb18) || F32(RECORD(unit), 8) > GF32(0x1012bb18)))) {
        if (Sim_100c3892(unit, 0x14) != 0 && U32(unit, 0x80) != 0 && S8(RECORD(unit), 0x1e) == 0)
            Sim_100c274a(unit, 1);
    } else if (U32(unit, 0x264) == 0 && S8(RECORD(unit), 0x1e) != 0 && U32(unit, 0x80) != 0) {
        Sim_100c274a(unit, 0);
    }
}

// FUNCTION: HGSIM 0x100c3d16
uint32_t __cdecl Sim_100c3d16(void *unit)
{
    int16_t key = S16(unit, 0x1aa);
    uint32_t record;
    double angle;
    int32_t k;

    if (key <= 0)
        return (uint32_t)(uintptr_t)unit & 0xffff0000u;
    if ((key & 0x200) == 0)
        return 0;
    record = PTRTAB((int32_t)key & 0xff);
    angle = (double)Sim_100c34b7((void *)(uintptr_t)record, (int16_t)(U32(unit, 4) | 0x200)) / GF32(0x1012bbbc);
    k = ftol(angle + GF32(0x1012bb28));
    if (k >= 8)
        k = 0;
    if (k == 0 || k == 1 || k == 7) {
        if (k == 0) {
            k = Sim_1009c929(2) != 0 ? 2 : 6;
            if (S8(unit, k + 0x270) != 0)
                k = k != 2 ? 2 : 6;
        }
        if (k == 1)
            k = 3;
        if (k == 7)
            k = 5;
    } else {
        int found = 0;
        int j;

        if (k > 4) {
            for (j = 6; j > 3 && found == 0; j--)
                if (S8(record, j + 0x270) == 0)
                    found = 1;
            j++;
        } else {
            for (j = 2; j < 5 && found == 0; j++)
                if (S8(record, j + 0x270) == 0)
                    found = 1;
            j--;
        }
        k = j;
    }
    return (uint32_t)k << 1;
}

/* Squared distance of the positions (+0x58..+0x60) of a and b against 15000 * 15000, as the x87 code does it. */
static int within_range(uint32_t a, const void *b)
{
    float dz = (float)((double)F32(a, 0x60) - F32(b, 0x60));
    float dy = (float)((double)F32(a, 0x5c) - F32(b, 0x5c));
    double dx = (double)F32(a, 0x58) - F32(b, 0x58);
    float dxf = (float)dx;
    double sum = dx * dxf;

    sum = sum + (double)dy * dy;
    sum = sum + (double)dz * dz;
    return (double)GF32(0x1012bbc4) * GF32(0x1012bbc4) >= sum;
}

// FUNCTION: HGSIM 0x100c4a1f
int __cdecl Sim_100c4a1f(void *unit)
{
    int slot;
    int index = Sim_100c4966(unit, &slot);
    uint32_t other;

    if (index == -1)
        return 0;
    other = PTRTAB(index);
    if (((U32(other, 0xf0) & 0x200) != 0 && (U32(other, 0xf0) & 0xff) == U32(unit, 4))
        || ((U16(other, 0x1aa) & 0x200) != 0 && (U16(other, 0x1aa) & 0xffu) == U32(unit, 4))) {
        if (within_range(other, unit))
            return W7P_CALL_1000a6f0((void *)(uintptr_t)other, 1);
        return 0;
    }
    U32(unit, (uint32_t)slot * 4 + 0x20c) = 0xffffffffu;
    return 0;
}

// FUNCTION: HGSIM 0x100c4b67
int __cdecl Sim_100c4b67(void *unit)
{
    int result = 0;

    if (G32(0x1015ec58) != 0) {
        int i;

        for (i = 0; i < GS32(0x1015dc64) && result == 0; i++) {
            uint32_t other = PTRTAB(i);

            if (S32(other, 0x14) == 1 || S32(other, 0x14) == 0) {
                int owner = Sim_10052be7((int)U32(other, 4));

                if (owner != Sim_10052be7((int)U32(unit, 4)) && within_range(other, unit))
                    result = W7P_CALL_1000aa9e((void *)(uintptr_t)other, unit, 1);
                else
                    result = 0;
            } else {
                result = 0;
            }
        }
    } else {
        uint32_t other = PTRTAB(G32(0x1017b8c8));
        int owner = Sim_10052be7((int)U32(other, 4));

        if (owner != Sim_10052be7((int)U32(unit, 4))) {
            if (within_range(other, unit))
                result = W7P_CALL_1000aa9e((void *)(uintptr_t)other, unit, 1);
            else
                result = 0;
        }
    }
    return result;
}
