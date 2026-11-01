#include "w10.h"
#include "../simmem.h"
#include "../util/access.h"
#include "../unsorted_1000/w5x.h"
#include "w2g.h"
#include "w5c.h"
#include "w5z.h"
#include "w7m.h"
#include "w3s.h"
#include "../fx/w32.h"
#include "../util/util.h"

#define W10_4FL(p, off) (*(float *)(BYTES(p) + (off)))
#define W10_4R562(rec) U16(rec, 0x562)

static const uint32_t W10_4FALL[5] = {8, 0xd, 0x11, 0x15, 0x19};

/* falling over (record +0x562 bit 0x800 with no other state and no +0xc3d): the fall animation of the kind (0..4), else
 * (when clear is set) bit 0x800 goes off; 1 when the fall test held */
static int W10_4Fall(uint8_t *obj, uint8_t *rec, int32_t kind, int clear)
{
    if ((W10_4R562(rec) & 0x800) == 0 || (W10_4R562(rec) & 0x60e) != 0 || U32(rec, 0xc3d) != 0)
        return 0;
    if (kind >= 0 && kind <= 4)
        U32(obj, 0x90) = U32(rec, 0x5ff) | W10_4FALL[kind];
    else if (clear)
        W10_4R562(rec) &= 0xf7ff;
    return 1;
}

/* a control byte of +0x54 set while the record is in no other state */
static int W10_4Ready(uint8_t *obj, uint8_t *rec, uint32_t byte)
{
    return U8(U32(obj, 0x54), byte) != 0 && (W10_4R562(rec) & 0x60e) == 0 && U32(rec, 0xc3d) == 0;
}

/* the animation of a speed tier: fall, or base + 1 / + 2 by the control bytes 0x57 / 0x58, or base */
static void W10_4Tier(uint8_t *obj, uint8_t *rec, int32_t kind, int clear, uint32_t base)
{
    if (W10_4Fall(obj, rec, kind, clear))
        return;
    if (W10_4Ready(obj, rec, 0x57))
        U32(obj, 0x90) = U32(rec, 0x5ff) | (base + 1);
    else if (W10_4Ready(obj, rec, 0x58))
        U32(obj, 0x90) = U32(rec, 0x5ff) | (base + 2);
    else
        U32(obj, 0x90) = U32(rec, 0x5ff) | base;
}

// FUNCTION: HGSIM 0x1004f257
void __cdecl Sim_1004f257(uint8_t *obj)
{
    uint8_t *rec = BYTES(U32(obj, 0x28));
    float out[3], v, a[3], b[3];
    int32_t kind, moving = 1;

    G32(0x1015f3a8) = 1;
    kind = Sim_1004f1df((int)U32(obj, 0x8c));
    if (!(W10_4FL(rec, 0x7c) > *SIM_PTR(float, 0x1012b40c))) {
        /* standing */
        moving = 0;
        U32(obj, 0x94) = 3;
        if (W10_4Fall(obj, rec, kind, 1)) {
        } else if (W10_4Ready(obj, rec, 0x57)) {
            U32(obj, 0x90) = U32(rec, 0x5ff) | 1;
        } else if (W10_4Ready(obj, rec, 0x58)) {
            U32(obj, 0x90) = U32(rec, 0x5ff) | 2;
        } else if ((W10_4R562(rec) & 0x10) != 0 && (W10_4R562(rec) & 0x60e) == 0 && U32(rec, 0xc3d) == 0) {
            U32(obj, 0x94) = 1;
            U32(obj, 0x90) = U32(rec, 0x5ff) | 3;
        } else if ((W10_4R562(rec) & 4) != 0) {
            U32(obj, 0x94) = 2;
            U32(obj, 0x90) = U32(rec, 0x5ff) | 4;
        } else if ((W10_4R562(rec) & 0x400) != 0) {
            U32(obj, 0x94) = 2;
            U32(obj, 0x90) = U32(rec, 0x5ff) | 5;
        } else {
            Sim_1004f158(obj);
        }
    } else {
        /* moving: the speed over [0x1012b410] picks the tier */
        double d = (double)W10_4FL(BYTES(U32(obj, 0x28)), 0x7c) - *SIM_PTR(float, 0x1012b410);

        v = (float)d;
        if (!(d >= (double)*SIM_PTR(float, 0x1012b414)))
            v = 0.0f;
        if (W10_4Fall(obj, rec, kind, 1)) {
        } else if (W10_4Ready(obj, rec, 0x57)) {
            U32(obj, 0x94) = 3;
            U32(obj, 0x90) = U32(rec, 0x5ff) | 0xb;
        } else if (W10_4Ready(obj, rec, 0x58)) {
            U32(obj, 0x94) = 3;
            U32(obj, 0x90) = U32(rec, 0x5ff) | 0xc;
        } else {
            U32(obj, 0x90) = U32(rec, 0x5ff) | 0xa;
        }
        if (!(v >= *SIM_PTR(float, 0x1012b418))) {
            U32(obj, 0x94) = 1;
        } else if (!(v >= *SIM_PTR(float, 0x1012b41c))) {
            U32(obj, 0x94) = 2;
        } else if (!(v >= *SIM_PTR(float, 0x1012b420))) {
            U32(obj, 0x94) = 3;
        } else if (!(v >= *SIM_PTR(float, 0x1012b424))) {
            U32(obj, 0x94) = 1;
            W10_4Tier(obj, rec, kind, 1, 0x12);
        } else if (!(v >= *SIM_PTR(float, 0x1012b428))) {
            U32(obj, 0x94) = 2;
            W10_4Tier(obj, rec, kind, 0, 0x12);
        } else if (!(v >= *SIM_PTR(float, 0x1012b410))) {
            U32(obj, 0x94) = 2;
            W10_4Tier(obj, rec, kind, 1, 0x12);
        }
        if (U32(obj, 0x8c) == 0xe && S32(obj, 0x94) > 2)
            U32(obj, 0x94) = 2;
        if (U8(U32(obj, 0x54), 0x59) != 0)
            W10_4Tier(obj, rec, kind, 0, 0xe);
        if (((W10_4R562(rec) & 1) != 0 || U32(U32(rec, 0), 0x15c) != 0) && (W10_4R562(rec) & 0x60e) == 0 &&
            U32(rec, 0xc3d) == 0 && (W10_4R562(rec) & 0x800) == 0)
            U32(obj, 0x90) = U32(rec, 0x5ff) | 9;
        if (U32(obj, 0) == 4 || U32(obj, 0) == 0xa)
            U32(obj, 0x8c) = U32(obj, 0x90);
    }
    if (U32(obj, 0) == 5) {
        U32(obj, 0x94) = 2;
        U32(obj, 0x90) = 0x17;
    }
    Sim_1004ee50((uint32_t *)obj);
    if ((U32(obj, 0x88) & 8) != 0) {
        /* 1.2 (0x1003fbb7): a moving unit's step raises dust at the lower foot (+0x4c4, else +0x4c0) against its
           velocity +0x548 */
        if (moving && W32_PARTICLES()) {
            Sim_100a9cfb((void *)(uintptr_t)U32(rec, 0x4c4), (uint32_t *)&a[0], (uint32_t *)&a[1], (uint32_t *)&a[2]);
            Sim_100a9cfb((void *)(uintptr_t)U32(rec, 0x4c0), (uint32_t *)&b[0], (uint32_t *)&b[1], (uint32_t *)&b[2]);
#ifdef W10FS_MUTATE /* the checker's negative check only: the higher foot */
            if (!(a[1] < b[1])) {
#else
            if (a[1] < b[1]) {
#endif
                b[0] = a[0];
                b[1] = a[1];
                b[2] = a[2];
            }
            Sim12_10086007(b[0], b[1], b[2], -W10_4FL(rec, 0x548), -W10_4FL(rec, 0x54c), -W10_4FL(rec, 0x550));
        }
        if (U32(obj, 4) == G32(0x1017b8c8)) {
            U32(obj, 0x88) &= 0xfffffff7u;
            Sim_1004f19d(obj, out);
            Sim_1000ed52(*(uint32_t *)&out[0], *(uint32_t *)&out[1], *(uint32_t *)&out[2], 0x1013bab8u,
                         G32(0x1015ef0c), (uint32_t)(uintptr_t)obj);
        }
    }
    G32(0x1015f3a8) = 0;
}
