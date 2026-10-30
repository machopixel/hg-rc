/*
 * Round 6, letter e: functions at 0x100607d5..0x10068d76. Helpers that are not converted yet are called through their
 * original address (SIM_FN) with the prototypes below.
 */
#ifndef HG_SIM_UNSORTED_1006_W6E_H
#define HG_SIM_UNSORTED_1006_W6E_H

#include <stdint.h>
#include "../simmem.h"
#include "../util/access.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Not converted: (-2, object, 2) in 0x1006863d. */
#define W6E_CALL_1006abf6 SIM_FN(void(__cdecl *)(uint32_t, void *, uint32_t), 0x1006abf6)

#ifndef HG_SIM_W6E_FLOAT_ACCESS
#define HG_SIM_W6E_FLOAT_ACCESS
/* A float of the original's data at a byte offset (may alias anything, like the U32 family in util/access.h). */
typedef float HG_MAY_ALIAS w6e_f32;
#define W6E_F(p, off) (*(w6e_f32 *)(BYTES(p) + (off)))
#define W6E_GF(addr) (*SIM_PTR(w6e_f32, addr))
static inline uint32_t W6e_Bits(float f)
{
    union {
        float f;
        uint32_t u;
    } v;
    v.f = f;
    return v.u;
}
static inline float W6e_Float(uint32_t u)
{
    union {
        float f;
        uint32_t u;
    } v;
    v.u = u;
    return v.f;
}
#endif

/* Runs 0x10060882 on the three rows and then the three columns of the 3x3 float matrix m. */
void __cdecl Sim_100607d5(float *m);
/*
 * obj[0] is the object (position at +0x58..0x60, number at +0x150). When obj+0x4d8 is 0 calls 0x10024616(number,
 * 0x24, position, position); otherwise, when 0x1009c929(100) <= 20, moves x and z by half of a value of 0x1009c969
 * each and calls 0x10024616 with 0x3e (when 0x1009c929(100) < 60) or 0x10b.
 */
void __cdecl Sim_10061c32(uint32_t *obj);
/* For the slots obj+0x4a8 and +0x4ac that are set: stores the slot in [obj[0]]+0x50 and calls 0x10024684 and 0x100a9cfb. */
void __cdecl Sim_10061dfc(uint32_t *obj);
/*
 * Calls 0x10061f8e(0, -value, 0) when obj[0]+4 is the current entry [0x1017b8c8], then 0x1000ed52 with the position
 * of obj[0] relative to the point at [0x1014cae0] (y also less obj+0x520) and the name "AFXHGCOO" (value > 140) or
 * "AFXHGJLR".
 */
void __cdecl Sim_10061ecd(uint32_t *obj, float value);
void __cdecl Sim_10061f8e(float x, float y, float z);
/*
 * Calls 0x100a2b8c eight times for the rectangle rect (centre +0/+4, half sizes +8/+0xc, colour +0x1c): the four
 * sides, then lines from the sides to 0 and to [0x109e3de8] / [0x109e3da8].
 */
void __cdecl Sim_1006326b(const uint32_t *rect);
void __cdecl Sim_1006863d(uint32_t *obj);
/* Runs 0x10068bb2 (angle a), 0x10068c48 (b) and 0x10068ce0 (c) on the point and stores it; nothing when an output is 0. */
void __cdecl Sim_10068d76(float x, float y, float z, float a, float b, float c, uint32_t *outX, uint32_t *outY,
                          uint32_t *outZ);

#endif
