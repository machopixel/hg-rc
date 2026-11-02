/*
 * Round 6, letter e: functions at 0x1004131b..0x1004ff72. Helpers that are not converted yet are called through their
 * original address (SIM_FN) with the prototypes below.
 */
#ifndef HG_SIM_UNSORTED_1004_W6E_H
#define HG_SIM_UNSORTED_1004_W6E_H

#include <stdint.h>
#include "../simmem.h"
#include "../util/access.h"
#include "../libm/hg_math.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Not converted: (record +0x28, dword, byte) of 0x1004131b. */
#define W6E_CALL_100386d5 SIM_FN(uint32_t(__cdecl *)(uint32_t, uint32_t, uint32_t), 0x100386d5)
/* Not converted: takes the index of a 0x50-byte entry of the table at 0x109e9ba0. */
#define W6E_CALL_1002597b SIM_FN(void(__cdecl *)(uint32_t), 0x1002597b)
/* Hooked by hooks_sim.c (SystemError): (code, format, ...). */
#define W6E_CALL_100bc280 SIM_FN(void(__cdecl *)(uint32_t, uint32_t), 0x100bc280)
/* Not converted: C library fabs (double in, st(0) out). */
#define W6E_CALL_100fcb70 hg_fabs
/* Not converted: writes the low and the high byte of the value to the stream at [0x1015f344]; returns 2 on error. */
#define W6E_CALL_1004dd2a SIM_FN(int(__cdecl *)(uint32_t), 0x1004dd2a)

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

/*
 * For the message `msg` (byte 2: index into the table at 0x109e8b40, byte 3 and dword 4: arguments): unless that
 * entry is missing, is the current one ([0x1017b8c8], also through +0xc when +0x14 is 3) or has flag 2 or 4 at +0x1c,
 * calls 0x100386d5(entry +0x28, dword, byte) and 0x100aa799(entry +0x48).
 */
void __cdecl Sim_1004131b(const uint8_t *msg);
/*
 * When the dword at msg+2 is greater than [0x1015db80 + 4 * slot] (signed), stores it there and calls 0x1002597b(i)
 * for every i < 254 whose bit is set in the bit list at msg+6 (most significant bit first) and whose entry of the
 * table at 0x109e9ba0 (0x50 bytes each) does not have flag 4.
 */
void __cdecl Sim_10041b2d(const uint8_t *msg, int slot);
/*
 * Returns 0 when byte +0x1e or dword +0x16 of [0x1015d004] is 0. Otherwise takes the handle +0xc into [0x1015ecd4]
 * (0x100bc280(0x14, 0) when it is 0), calls the routines at [0x1015ea94] (handle, 0, 0) and [0x1015ead8] (handle,
 * result to 0x1015ec5c and 0x1015ec58), sets several globals and the two timers and returns 1.
 */
int __cdecl Sim_10041d67(void);
/*
 * Steps the index [0x1015ef18] over the [0x1015dc64] entries of the table at 0x109e8b40 (reset: to the current one
 * [0x1017b8c8]; else forward or back), wrapping around, and stores the entry in [0x1015ef14]; entries with flag 0x4800
 * at +0x1c are skipped by stepping again. Sets [0x1013aed4] to -1.
 */
void __cdecl Sim_10044a69(int forward, int reset);
/*
 * Fills six dwords from the entry [0x1015ef14] (all 0 when there is none): x/y/z from +0x64..0x6c and a/b/c from
 * +0x58..0x60; when the entry has a sub-object at +0x4c, a/b/c are its transformed point and x/y/z come from it.
 */
void __cdecl Sim_10044b2c(uint32_t *x, uint32_t *y, uint32_t *z, uint32_t *a, uint32_t *b, uint32_t *c);
void __cdecl Sim_100453a3(void);
/*
 * Moves the float obj+8 towards obj+4 by (now - obj[0]) * (obj+4 - obj+8) / obj+0xc, where now is [0x1015dc78],
 * without overshooting; stores now in obj[0]. Returns the new obj+8 (in st(0)).
 */
float __cdecl Sim_10047bcd(uint32_t *obj);
/*
 * Key handler: when 0x10048720 takes the key into the typed history, compares the history with the code strings
 * (0x10048762) and runs the action of the first one that matches.
 */
void __cdecl Sim_100488a3(uint32_t key);
/* Writes the two dwords and the three bytes at rec+8 of the record to the stream; returns 2 on an error, else 0. */
int __cdecl Sim_1004dbc5(const uint32_t *rec);
/*
 * Appends the low `bits` bits of value to the bit buffer at 0x1015f230 (byte index [0x1015f340], free bits in the
 * current byte [0x1015f348]); a full buffer (255 bytes) is written to the stream first. Returns the number of bits
 * written, or -1 on a stream error.
 */
int __cdecl Sim_1004e24a(int32_t value, int32_t bits);
void __cdecl Sim_1004ff72(void *obj, uint32_t base, uint32_t *point);

#endif
