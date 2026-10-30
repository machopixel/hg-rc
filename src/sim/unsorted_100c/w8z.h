/*
 * Round 8, letter z: functions of HGSIM.DLL at 0x100c0670..0x100cfd28.
 *
 * "unit" is an entity of the pointer table 0x109e8b40 (PTRTAB): +4 id, +8 bit number, +0x28 block, +0x54 record
 * (float +8), +0x58/+0x5c/+0x60 position, +0xd4 float, +0xf0 selection (low byte index, 0xf00 kind 0x100 /
 * 0x200 / 0x400, 0x1000 flag), +0x118, +0x1a8/+0x1aa words, +0x1fc, +0x204 time, +0x218 state, +0x21c counter,
 * +0x23a entry index. The x87 arithmetic of the originals runs with 53-bit precision (control word 0x27f, the
 * one the game sets), so it is done here in double with every float store kept where the original rounds:
 * the results are the same bit for bit.
 *
 * Callees that are not converted (or patched elsewhere) are called through their original address (SIM_FN):
 *   0x100c2ae5 (unit) -> int
 *   0x1001412d (unit, float) -> float in st(0)
 *   0x1000a6f0 (unit, int) -> int
 *   0x100cada2 (who, index, flags) -> int
 *   0x10064824 (record, two out pointers) -> int
 *   0x100fcb30 (double, double) -> x87 result (fmod), stored as float by the callers
 *   0x100fca04 (double) -> x87 result (sqrt)
 *   0x1008486e (float, float) -> float in st(0); 0x10084766 (float) -> float in st(0)
 *   0x100652e8 (x, y, text, record), 0x100619a0 (5 arguments)
 *   0x100fe830 (buffer, size, count, stream) -> count (fread); 0x100feb20 (stream, offset, origin) (fseek)
 *   0x100ba7d5 (name, extension, out) -> resource; 0x100f2f01 / 0x100f1e10 / 0x100ecea6 (six arguments)
 */
#ifndef HG_SIM_UNSORTED_100C_W8Z_H
#define HG_SIM_UNSORTED_100C_W8Z_H

#include <stdint.h>
#include "../simmem.h"
#include "../unsorted_100f/w8z.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

typedef int(__cdecl *W8zUnitFn)(void *unit);
typedef float(__cdecl *W8zUnitFloatFn)(void *unit, float f);
typedef int(__cdecl *W8zUnitIntFn)(void *unit, int flag);
typedef int(__cdecl *W8zPickFn)(int who, int index, uint32_t flags);
typedef int(__cdecl *W8zRecordOutFn)(void *record, void *a, void *b);
typedef float(__cdecl *W8zFmodFn)(double x, double y);
typedef double(__cdecl *W8zSqrtFn)(double x);
typedef float(__cdecl *W8zAngle2Fn)(float a, float b);
typedef float(__cdecl *W8zAngle1Fn)(float a);
typedef void(__cdecl *W8zLabelFn)(int x, int y, const char *text, void *record);
typedef void(__cdecl *W8zView5Fn)(uint32_t a, uint32_t b, void *rec, uint32_t c, uint32_t d);
typedef uint32_t(__cdecl *W8zReadFn)(void *buf, uint32_t size, uint32_t count, void *stream);
typedef int(__cdecl *W8zSeekFn)(void *stream, int32_t offset, int origin);
typedef uint32_t(__cdecl *W8zResourceFn)(const char *name, const char *ext, void *out);
typedef void(__cdecl *W8zText6Fn)(uint32_t a, uint32_t b, uint32_t c, uint32_t res, void *text, void *out);
typedef void(__cdecl *W8zCallbackFn)(void *obj, uint32_t value);

#define W8Z_CALL_100c2ae5 SIM_FN(W8zUnitFn, 0x100c2ae5)
#define W8Z_CALL_1001412d SIM_FN(W8zUnitFloatFn, 0x1001412d)
#define W8Z_CALL_1000a6f0 SIM_FN(W8zUnitIntFn, 0x1000a6f0)
#define W8Z_CALL_100cada2 SIM_FN(W8zPickFn, 0x100cada2)
#define W8Z_CALL_10064824 SIM_FN(W8zRecordOutFn, 0x10064824)
#define W8Z_CALL_100fcb30 SIM_FN(W8zFmodFn, 0x100fcb30)
#define W8Z_CALL_100fca04 SIM_FN(W8zSqrtFn, 0x100fca04)
#define W8Z_CALL_1008486e SIM_FN(W8zAngle2Fn, 0x1008486e)
#define W8Z_CALL_10084766 SIM_FN(W8zAngle1Fn, 0x10084766)
#define W8Z_CALL_100652e8 SIM_FN(W8zLabelFn, 0x100652e8)
#define W8Z_CALL_100619a0 SIM_FN(W8zView5Fn, 0x100619a0)
#define W8Z_CALL_100fe830 SIM_FN(W8zReadFn, 0x100fe830)
#define W8Z_CALL_100feb20 SIM_FN(W8zSeekFn, 0x100feb20)
#define W8Z_CALL_100ba7d5 SIM_FN(W8zResourceFn, 0x100ba7d5)
#define W8Z_CALL_100f2f01 SIM_FN(W8zText6Fn, 0x100f2f01)
#define W8Z_CALL_100f1e10 SIM_FN(W8zText6Fn, 0x100f1e10)
#define W8Z_CALL_100ecea6 SIM_FN(W8zText6Fn, 0x100ecea6)

/* ---- w8z_units.c: unit states (the unit's words +0x1a8 / +0x1aa go to Sim_1001403a) ---- */
/*
 * Sets the unit's key, then (when 0x100c2ae5 says 0) stores 0x1001412d(unit, Sim_100c05cb(unit)) in the record's
 * float +8 and hands Sim_100144e9 to Sim_100c0379, else takes Sim_10015266. The limit is the int +0x278 as a
 * float, at most the float [0x1012bb2c] (else 4500); +0xd4 below it sets +0x1fc to 10. Calls Sim_1000ae00 with
 * the float; for a unit with dword +0 = 1 where that and 0x1000a6f0(unit, 1) give 0, stores
 * 0x1001412d(unit, limit + [0x1012bb30]) in the record's float +8.
 */
void __cdecl Sim_100c0670(void *unit, short key);
/*
 * Sets +0x21c to 1 when it is 0, the record's float +8 is within [0x1012bb34] of [0x1012bb18] (after
 * 0x1001412d(unit, 4000) when 0x100c2ae5 says 0) and |Sim_10015266| is above the double [0x1012bb38]. With
 * +0x21c = 1: Sim_100c0379 of Sim_10015266 or Sim_100144e9 (cleared record float when it says 0), and +0x21c
 * back to 0 once +0xd4 is above [0x1012bb2c].
 */
void __cdecl Sim_100c07c5(void *unit, short unused);
/*
 * Sets the key; 0x1001412d(unit, 5500) into the record float (when 0x100c2ae5 says 0); returns 1 when
 * Sim_100c3725 / [0x1012bb40] (1.0 below [0x1012bb20]) times [0x1012bb44] is at least +0xd4.
 */
int __cdecl Sim_100c0965(void *unit, short key);
/*
 * Sets the key; stores 0x1001412d(unit, 0) in the record float when 0x100c2ae5 says 0 or Sim_100c38fe(key)
 * equals +0x24a; returns 1 (and sets the record byte +0x4c) unless +0xd8 is above [0x1012bb48].
 */
int __cdecl Sim_100c0a2d(void *unit, short key);
/* 0x1001412d(unit, 3000) into the record float when 0x100c2ae5 says 0; 1 unless +0xd4 is above [0x1012bb4c]. */
int __cdecl Sim_100c0b1d(void *unit, short unused);
/* Like Sim_100c0b1d, with a timed Sim_1001570f / Sim_100157ca step first; ends with Sim_100159cf; returns 0. */
int __cdecl Sim_100c1049(void *unit, short unused);
/* Like Sim_100c0b1d; when +0xd4 is not above the limit: Sim_1001570f and Sim_100c00c7 or a key change. 0. */
int __cdecl Sim_100c1169(void *unit, short unused);
/* State machine on +0x218 (1 pick an entry with Sim_100c4035, 2 run Sim_100c45dc, 3 done): 0, 1 or -1. */
int __cdecl Sim_100c12d7(void *unit);
/* State machine on the nibbles of +0x218 (Sim_100c4e1c / Sim_100c45dc steps with timers in +0x204). */
int __cdecl Sim_100c18c8(void *unit);
/*
 * State 1: the partner is the entity of the low byte of +0x1aa; the point partner + [0x1012bb44] * the
 * normalised (Sim_10084cee) product (0, s, 0) x (unit - partner), with s = +-1 from Sim_1009c929(2), goes to
 * Sim_100ca350; its answer (with 0x100) to Sim_100159cf and state 2, or -1. State 2: 0x1001412d(unit, 2000)
 * into the record float and back to state 1 unless +0xd4 is above [0x1012bb48]; other states (or 0x100c2ae5
 * not 0) give -1 and copy +0x1aa to +0x1a8 after Sim_100ca417.
 */
int __cdecl Sim_100c2279(void *unit, short unused);
/*
 * Checks the entry: 2 when Sim_100c459f rejects it, else Sim_100c4498, the key +0x1a8, the record float from
 * 0x1001412d(unit, 1000) (when 0x100c2ae5 says 0); returns 1 (record float cleared) unless +0xd4 is above
 * [0x1012bb10].
 */
int __cdecl Sim_100c45dc(void *unit, void *entry, uint32_t *slot);
/*
 * Returns 0x1000a6f0(unit, 1) when it is not 0. Else, when |Sim_100144e9| is below [0x1012bb60] (or NaN): the
 * direction is 0 for a negative *slot, 1 for a positive one; a zero *slot becomes [0x1015dc78] + 0x16a, negated
 * unless Sim_1009c929(2) (which is the direction) is not 0. Calls Sim_100c48fa(unit, direction) and returns 1
 * (*slot cleared, Sim_100c48fa(unit, 2)) when [0x1015dc78] is above the size of *slot, else 0.
 */
int __cdecl Sim_100c4e1c(void *unit, void *entry, int32_t *slot);

/* ---- w8z_targets.c: target choice and relative positions ---- */
/*
 * For the difference (dx, dy, dz): a = 0x1008486e(dx, dz); c = dx / 0x10084766(a) when |dz| < |dx| (or not
 * comparable), else dz / 0x10084766(a + [0x1012bc58]); d = 0x1008486e(dy, c); b = c / 0x10084766(d +
 * [0x1012bc58]). A divisor whose size is below [0x1012bc54] (or NaN) gives 0 instead. Stores a, c, b, d in that
 * order (0x1008486e / 0x10084766 answer float values in st(0); fabs is Crt_Fabs).
 */
void __cdecl Sim_100cbc6b(float dx, float dy, float dz, float *a, float *b, float *c, float *d);
/*
 * Applies the unit's selection +0xf0 (kind 0x100 / 0x200 / 0x400) through Sim_100cab7b / 0x100cada2 /
 * Sim_100cb299 and marks the target (bits in the target's words, 0x1017e5ac for the current unit). Returns 0
 * (and sets 0x1000 in +0xf0) when the selection is not valid, else 1.
 */
int __cdecl Sim_100cb4fb(void *unit);
/* Takes the record [0x1017e678] as the current unit's new selection when it is valid; sets [0x1017e5b0]. */
void __cdecl Sim_100cbaf8(void);
/*
 * For the unit u = *pp with the bit of value 4 in +0x26c, when Sim_1009c929(3) is not 0 and the type of rec
 * ([rec+4], table 0x10145508, 0xd6 bytes) has bit 0 in its dword +0x86: takes the unit's selection (+0xf0, or
 * its word +0x1aa with 0x200); only kind 0x200 with an index that is not 0 and an entity whose +0x1fc is not 4
 * goes on. The entity becomes the target directly for units of type +0x14 = 2 (or 3 when +0xc is the current
 * one); else when 0x10064824(pp, ..) and Sim_100d2669 of its position accept it and fmod([pp+0x1c] + [u+0xec],
 * 360) is inside (-3, 3) and fmod(Sim_10015266(u), 360) inside (-15, 15) (NaN passes these tests). With a target
 * that Sim_100c3892(target, 0x41) accepts and Sim_1009c929(3) not 0: tries Sim_100c31a6 to both sides
 * (Sim_1009c929(2) picks the first) with the direction ((trunc(Sim_100c34b7 / [0x1012bbb8]) + 1) % 4) * 4 until
 * Sim_1002dda9 finds nothing, then sets +0x1fc = 11 and +0x218 = direction. Finally +0x1fc -1 becomes 4 when
 * +0x26c has the bit of value 2.
 */
void __cdecl Sim_100c398d(void *rec, void *pp);
/*
 * Sets obj+0x50 from the block (+0x28) record table; [0x1012bc2c] when it is 0 (or kind is not 0x200 and
 * Sim_100cba68 gives nothing). Takes the height h (entity `who` +0x5c for kind 0x200, else the second value of
 * Sim_100a9cfb of that object), the direction of Sim_100d6ea7 (Sim_10084c95), the type's float +0x3c (0x45 in its
 * +0x10 adds the double [0x1012bc00] to y) and g = -[0x1013abc0] * type+0x38 * [0x1012bbf8]; solves
 * t = (-vy - sqrt(vy*vy - [0x1012bbf4]*g*(y - h))) / ([0x1012bc30]*g) (the other root when that is negative)
 * with y from Sim_100a9cfb of obj+0x50, hands (vx, vy, vz) * t to Sim_100cbc6b and returns its second answer
 * / [0x1012bc20] as a double in st(0).
 */
double __cdecl Sim_100c9572(void *obj, int kind, int who);

/* ---- w8z_misc.c ---- */
/*
 * Reads the tar headers of the stream [idx+0] (0x200 bytes into [0x10151ce4], Tar_ReadHeader into 0x1017e4d0)
 * and stores up to 100 entries of 12 bytes at idx+4 (name copy, position, size [0x1017e4ec]; count +0x4b4),
 * skipping each file's data rounded up to 0x200.
 */
void __cdecl Sim_100c6cf5(void *idx);
/*
 * For an object with a non-zero word +4 while [0x10151cf8] is set: stores [0x109e89d8] in +0xf8; word +6 = 1
 * alternates with Sim_100c81b5 by the random flag 0x1017e590, above 2 always goes to Sim_100c81b5. Otherwise,
 * for the current unit's selection (block +0x28, its unit +0xf0): kind 0x100 hands the text "VTGT_NP" /
 * "VTGT_NR" and the quarter differences of the record [obj+0xec] to 0x100652e8; other kinds (not with the
 * 0x1000 flag or a 0x400 entry with bit 4) fill a view record (Sim_10044992 of [0x1014cae0]), save state with
 * Sim_100787f4, reset the dwords 0x1014cb04.. and call 0x100619a0 while [0x109e89d8] is 2, then copy the saved
 * 0x68 bytes to 0x1014cae8.
 */
void __cdecl Sim_100c7deb(void *obj);
/*
 * For an object with a non-zero word +4 while [0x10179cc4] is set: calls its routine +0x114(obj, [0x101c4c54]),
 * loads the resource [0x109b9900] (0x100ba7d5) and hands it with the object's text +0x10 and its dwords to
 * 0x100f2f01 ([0x101668e8] set), 0x100f1e10 ([0x1017b940] set) or 0x100ecea6, then releases it.
 */
void __cdecl Sim_100cfd28(void *obj);

#endif
