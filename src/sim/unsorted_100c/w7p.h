/*
 * Round 7, letter p: functions of 0x100c0bdd..0x100c6fbd. Most are steps of the per-unit state machines kept in
 * the dword +0x218 of a unit (game logic: their float code is bit exact with SSE2 doubles, no tolerance), plus
 * a few helpers (a segment probe, range checks, a target pick, an octal field parser, stream wrappers). Helpers
 * that are not converted yet (or that another round-7 helper converts) are called through their address.
 */
#ifndef HG_SIM_UNSORTED_100C_W7P_H
#define HG_SIM_UNSORTED_100C_W7P_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Not converted: fabs(double) of the DLL's C library (result in st(0)). */
#define W7P_FABS SIM_FN(double(__cdecl *)(double), 0x100fcb70)
/* Not converted: sprintf(buffer, format, ...) of the DLL's C library. */
#define W7P_SPRINTF SIM_FN(int(__cdecl *)(char *, const char *, ...), 0x100fc780)
/* Not converted: (unit, entry from Sim_10033c42, unit + 0x224); 1 when done. */
#define W7P_CALL_100c45dc SIM_FN(int(__cdecl *)(void *, void *, void *), 0x100c45dc)
/* Not converted: (unit); 0 when the caller should turn (Sim_100144e9) at the rate 0x3c800000 (1/64). */
#define W7P_CALL_100c2ae5 SIM_FN(int(__cdecl *)(void *), 0x100c2ae5)
/* Not converted (round 7, letter l): (segment, out pointer, dword); 0 when nothing was hit. */
#define W7P_CALL_1002dda9 SIM_FN(int(__cdecl *)(void *, uint8_t **, uint32_t), 0x1002dda9)
/* Not converted (round 7, letter m): (x, y, z as float bits, dword, out list pointer); returns the count. */
#define W7P_CALL_10033eb3 SIM_FN(int(__cdecl *)(uint32_t, uint32_t, uint32_t, uint32_t, int32_t **), 0x10033eb3)
/* Not converted (round 7, letter m): (dword, position pointer, float bits, out 3 dwords); 0 when nothing found. */
#define W7P_CALL_1003387d SIM_FN(int(__cdecl *)(uint32_t, void *, uint32_t, uint32_t *), 0x1003387d)
/* Not converted: (unit, 1). */
#define W7P_CALL_1000a6f0 SIM_FN(int(__cdecl *)(void *, int), 0x1000a6f0)
/* Not converted: (unit, other unit, 1). */
#define W7P_CALL_1000aa9e SIM_FN(int(__cdecl *)(void *, void *, int), 0x1000aa9e)
/* Not converted: (file record). */
#define W7P_CALL_100c6cf5 SIM_FN(void(__cdecl *)(void *), 0x100c6cf5)
/* Not converted: fread(buffer, size, count, stream) of the DLL's C library; returns the count read. */
#define W7P_CALL_100fe830 SIM_FN(uint32_t(__cdecl *)(void *, uint32_t, uint32_t, uint32_t), 0x100fe830)

/*
 * State step: Sim_1001403a(unit, key), Sim_1000ae00(unit, Sim_10015266(unit)), then by state +0x218: 0: when
 * Sim_100c3892(unit, 40) and Sim_100c25af(unit) Sim_100c274a(unit, 1), else Sim_100c274a(unit, 0), state 1,
 * +0x1fc = 5 when Sim_100c3892(unit, 20) and Sim_1009c929(4) is 0, +0x204 = the clock 0x1015dc78 + 0xb5;
 * 1: past +0x204 state 2 and Sim_100c274a(unit, 0), else Sim_100c274a(unit, 1); then Sim_100144e9(unit);
 * 2: returns 1 when Sim_100c264a(unit) is not 0. Returns 0 otherwise.
 */
int __cdecl Sim_100c0bdd(void *unit, int16_t key);
/*
 * State step: as Sim_100c0bdd up to Sim_1000ae00 (angle a). Result = 1 when +0x80 or the block's +0x4f4 is set;
 * returned at once in state 2. Else with |a| and the block float +0x534 (s): near = s < |a| < 3 s. When the float
 * +0xd4 is above 1000: Sim_100c3702(unit, near when Sim_100c3725(unit) is above 36 (+0xd4 above 6000) or 9, else
 * !near), Sim_100c274a(unit, 1), result 1 when Sim_100c3892(unit, 40) is 0. Otherwise: the state grows by one when
 * the byte +0x1e of the record +0x54 is set, Sim_100c274a(unit, 0) and the record's bytes +0x21, +0x22 cleared.
 * The record of the key's unit (pointer table, key & 0xff) is read.
 */
int __cdecl Sim_100c0dd7(void *unit, int16_t key);
/*
 * State step (mode 1): state 1: +0x21c += 10000, Sim_100c43e1(unit, (float)+0x21c, 1) gives an index; state 2
 * with +0x23a = index when 0 <= index < the count 0x109e4cc0, else state 1. State 2: state 3 when
 * 0x100c45dc(unit, Sim_10033c42(+0x23a), unit + 0x224) answers 1, else state 1. State 3: state 0, returns 1.
 * Other states: state 0, returns -1. Then Sim_1001403a(unit, word +0x1aa), Sim_1000ae00(unit, Sim_10015266(unit))
 * and the new state is stored. Returns 0 in the other cases. The key is not used.
 */
int __cdecl Sim_100c145b(void *unit, int16_t key);
/* The same with mode 2 for Sim_100c43e1. */
int __cdecl Sim_100c1609(void *unit, int16_t key);
/*
 * State step after Sim_1001403a(unit, key). State 1 -> 2 when the block's +0x514 is above 0 and the float +0xd4
 * is at least 5000 (else -1). State 2: Sim_100144e9 and the record float +8 = 1/64 when 0x100c2ae5(unit) is 0;
 * unless +0xd4 is above 5000: record byte +0x1e = 1, block +0x518 = 1, state 3, +0x204 = clock + 0x5a8. State 3:
 * clears the record byte +0x1e; at +0x204 state 4 (block +0x518 and +0x204 cleared, returns 1), else +0x200 =
 * clock + 0xb5 once the clock reaches it. Other states: -1 (record byte +0x1e and block +0x518 cleared when the
 * byte was 1). Ends with Sim_1000ae00(unit, Sim_10015266(unit)); returns 0 in the other cases.
 */
int __cdecl Sim_100c1c67(void *unit, int16_t key);
/*
 * As Sim_100c1c67, except: state 1 -> 2 when +0x514 is above 0 and the float +0xd4 is not above 1000, with
 * +0x21c = clock + 0x2d4 and the record byte +0x59 = 1; state 2 goes on to 3 when the clock reaches +0x21c;
 * state 3 -> 4 and the other states also clear the record byte +0x59.
 */
int __cdecl Sim_100c1e69(void *unit, int16_t key);
/* State step of the record byte +0x56 (1 between the states 2 and 4) with the timers +0x204 and 0x100c2ae5. */
int __cdecl Sim_100c209c(void *unit, int16_t key);
/*
 * Fall check: when the block's +0x510 is not negative and its float +0x540 is not 0: below y 20000 (+0x5c) the
 * flag Sim_100c274a writes becomes 1 when the block float +0x54c is below -13.73 (double 0x1012bb70) and 0 when it is
 * above -12.12 (0x1012bb78) (else the record +0x54 byte +0x1e is kept); below -16.15 (0x1012bb80) the text "%6ld :
 * %2d Mech %2d has exceded fall damage speed." (clock, +8, +4) goes to Sim_1005f9aa. Returns the dword +0x80.
 */
uint32_t __cdecl Sim_100c264a(void *unit);
/*
 * Height probe: Sim_1002d961 at the segment's end point (+0xc..+0x14). When the float 0x1015dd50 is below 0.766
 * or |height - unit float +0x7c| is above 2000, the end point goes to unit +0x24e.. and 1 is returned. Else the
 * same with a copy of the segment shortened to the unit float +0x246 (Sim_1009cfbd, Sim_1009cd36); 0 when that
 * does not hold either.
 */
int __cdecl Sim_100c292c(void *seg, void *unit);
/*
 * Builds the probe segment seg for a move of the unit: the direction entry (dir, or (16 - dir) % 16 for a negative
 * side) of the table 0x101516f8 gives x, y = scale / entry (0 entries kept), turned with the matrix
 * Sim_100aa863(unit +0x48) (Mat3x4_TransformPoint); the start is the unit's position, or with flag the point
 * (block float +0x53c - 1, or 1 - it for a negative side) turned the same way. Sim_1009cab0 fills seg; Sim_1009cc82
 * when the horizontal length Sim_10084a94 is above 3000, else Sim_1009cbfe.
 */
void __cdecl Sim_100c31a6(void *unit, void *seg, int side, int16_t dir, float scale, int flag);
/*
 * Line of sight from the unit to height y: Sim_1009cab0 / Sim_1009cbfe build a segment from (x +0x58, y, z +0x60)
 * to the floats +0xdc..+0xe4, 0x1002dda9(segment, &hit, +4). 1 when nothing is hit; else 1 when the hit's word +2
 * has 0x100 and the unit's +0xf0 has 0x200, or 0x200 and 0x400, and the hit's word +0x24 equals +0xf0 & 0xff; else 0.
 */
int __cdecl Sim_100c3550(void *unit, float y);
/*
 * When +0x26c bit 0 is set, the word +0x25e is 0 and the word +0x1f8 is not 10: a = |Sim_10015266(unit)| after
 * Sim_1001403a(unit, word +0x1a8). When a >= the block float +0x534 or (-it >= a and the record float +8 is not 0):
 * Sim_100c274a(unit, 1) if Sim_100c3892(unit, 20), +0x80 is set and the record byte +0x1e is 0; otherwise
 * Sim_100c274a(unit, 0) if +0x264 is 0, the record byte +0x1e is set and +0x80 is set.
 */
void __cdecl Sim_100c376e(void *unit);
/*
 * Side to take (0..7, doubled): with the key word +0x1aa (above 0, group 0x200) the angle Sim_100c34b7(record of
 * key & 0xff, +4 | 0x200) / 45 + 0.5 truncated (8 and above become 0). 0, 1, 7: 1 -> 3, 7 -> 5, 0 -> 2 or 6 by
 * Sim_1009c929(2) (the other one when the unit's byte +0x270 + k is set). 2..4 / 5..: the nearest free one
 * (record byte +0x270 + k zero) from 2 up / from 6 down. Returns k * 2 (only the low word is used by the caller;
 * without a key the original leaves the high word of the unit pointer above a zero low word, kept here).
 */
uint32_t __cdecl Sim_100c3d16(void *unit);
/*
 * Picks a target entry for a and b: 0x10033eb3(a's position, c, &list) gives a count; without a list
 * Sim_10033d75(a's position). Else the first entry whose Sim_100c3f0c(a, b, Sim_10033c42(entry)) holds, or
 * list[Sim_1009c929(count)]. -1 when a or b is null.
 */
int __cdecl Sim_100c4035(void *a, void *b, uint32_t c);
/*
 * When 0x1003387d(a, position of the record of the unit's key +0x1aa, the block float +0x53c, out) finds
 * something: unless *slot is 0x80000000 it takes the unit's +0x118 and Sim_100ca417(+4, *slot) must not be -1;
 * then *slot = Sim_100ca350(+4, out[0..2]) | 0x100 after Sim_100159cf(unit, 0x100).
 */
void __cdecl Sim_100c4498(void *unit, uint32_t a, uint32_t *slot);
/*
 * Target in range: Sim_100c4966(unit, &slot) gives a unit index (0 when -1). When that unit's +0xf0 (group 0x200)
 * or word +0x1aa (group 0x200) names this unit's +4: 0x1000a6f0(it, 1) when it is within 15000 (squared distance
 * against 15000 * 15000), else 0. Otherwise the slot dword +0x20c + 4 * slot = -1 and 0.
 */
int __cdecl Sim_100c4a1f(void *unit);
/*
 * Like Sim_100c4a1f against the other side's units: with the dword 0x1015ec58 every unit of the pointer table
 * (count 0x1015dc64, state +0x14 0 or 1) of another owner (Sim_10052be7) until one answers, else only the unit
 * [0x1017b8c8]; 0x1000aa9e(it, unit, 1) for one within 15000.
 */
int __cdecl Sim_100c4b67(void *unit);
/*
 * Parses an octal number of at most n characters: leading spaces (class 8, Crt_IsCType or the class table
 * 0x10159450) are skipped (-1 when n runs out), digits 0..7 are taken, and what follows must be a zero byte or a
 * space (or the end of n), else -1.
 */
int __cdecl Str_ParseOctal(int n, const char *s);
/*
 * New record of 0x4bc bytes (Heap_Malloc; 0 when that fails) with the stream fopen(name, "rb") (0 is returned
 * when that fails, the record is not freed), +0x4b4 = 0, then 0x100c6cf5(record).
 */
void *__cdecl Sim_100c6e27(const char *name);
/* fread(buffer, 1, n, record's stream): n when all came, else -1. */
int __cdecl Sim_100c6fbd(void *rec, void *buffer, uint32_t n);

#endif
