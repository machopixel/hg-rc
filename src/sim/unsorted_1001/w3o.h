/*
 * Round 3 (letter o) functions around 0x10017ad9..0x10019920: a float field
 * update and the sweep over the 0x156-byte records at 0x1096fc80 (0x200 of
 * them) that is started from a pointer-table object.
 */
#ifndef HG_SIM_UNSORTED_1001_W3O_H
#define HG_SIM_UNSORTED_1001_W3O_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * With s = the pointer at obj+0x28 (obj not NULL): if the float at s+0x520 is
 * greater than value it becomes value, otherwise if it is smaller than value
 * (or either is a NaN) the float at 0x10111108 is added to it.
 */
void __cdecl Sim_10017ad9(void *obj, float value);

/*
 * Returns 0 if obj, obj+0x50 or obj+0x28 is zero. Otherwise counts into the
 * words at 0x101653f3 (obj+4 is [0x1017b8c8]) and 0x1016540c (the dword at +8
 * of the table objects 0x109e8b40[obj+4] and 0x109e8b40[[0x1017b8c8]] is
 * equal), runs Sim_10019920(obj) and returns 2. The original also pushes two
 * more arguments for the callee, which does not read them.
 */
int __cdecl Sim_1001987a(void *obj, uint32_t unused1, uint32_t unused2);

/*
 * Visits the records of the table at 0x1096fc80 (0x200 of 0x156 bytes) that
 * have non-zero dwords at +0x48 and +0x3c, a type dword (+0) of 3, 9, 10 or 12
 * and an owner (+0x50) whose Sim_10052be7 value differs from that of obj+4.
 * With d = obj+0x58.. minus the three dwords Sim_100a9cfb reads from the object
 * at record+0x3c, and dist = Sim_10084a94(d), a record with dist <= the float at
 * 0x1011118c is processed: if the sign bit of the float at +0x64 is set it is
 * recomputed from dist - the float at 0x10111194 (set to 0 when that is not
 * greater than the float at 0x10111198 or the record's direction (+0x18..)
 * points away from d, otherwise, when Sim_1009c929(100) <= 75, to 2500.0 for
 * small values or to 0x10111194 + the float at 0x1011119c * Sim_1009c929(n) with
 * n the truncated quotient). Then, if the float at +0x64 is greater than
 * the float at 0x10111198 and dist is not greater than it, bit 0x8000 of the
 * dword at +0x48 is set. Always returns 1.
 */
int __cdecl Sim_10019920(void *obj);

#endif
