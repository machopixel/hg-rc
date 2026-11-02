/*
 * Round 7, letter l: functions in 0x100200d1..0x1002ff6e. Helpers that are not converted (or that another
 * round-7 list converts, or that a hook file patches) are called through their original address with SIM_FN;
 * prototypes below. Floating point follows the original x87 code with the game's control word (53-bit
 * precision): every intermediate is a double, and only a store to a float rounds to float (see util/w3o.h for
 * the FCOM idioms). Floats that the original only moves as dwords are passed as raw bits (uint32_t).
 * Segment: the 0x38-byte ray records of unsorted_1009/w3o.h (start +0, end +0xc, vector +0x18, unit vector
 * +0x24, length +0x30).
 */
#ifndef HG_SIM_UNSORTED_1002_W7L_H
#define HG_SIM_UNSORTED_1002_W7L_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#define __stdcall
#endif

/* The C library sprintf of the DLL (0x100fc780). */
#define W7L_SPRINTF_1002 SIM_FN(int(__cdecl *)(char *, const char *, ...), 0x100fc780)
/* Patched by src/hooks/hooks_sim.c (TapSystemError): (code, format, ...). */
#define W7L_CALL_100bc280 SIM_FN(void(__cdecl *)(int, const char *, ...), 0x100bc280)
/* Not converted: (first, second, -1). */
#define W7L_CALL_1001f790 SIM_FN(void(__cdecl *)(uint32_t, uint32_t, int), 0x1001f790)
/* Not converted: (record). */
#define W7L_CALL_100b45b4 SIM_FN(void(__cdecl *)(void *), 0x100b45b4)
/* Not converted: no arguments, returns a count. */
#define W7L_CALL_100ab1f2 SIM_FN(uint32_t(__cdecl *)(void), 0x100ab1f2)
/* Not converted: (object). */
#define W7L_CALL_10029b1f SIM_FN(void(__cdecl *)(void *), 0x10029b1f)
/* Not converted: (start, direction, low, high, &near, &far) on float bits; non-zero means no overlap. */
#define W7L_CALL_1009cfd4 SIM_FN(int(__cdecl *)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t *, uint32_t *), 0x1009cfd4)
/* Not converted: (node, segment, &hit segment, &distance (in/out), &a, &b, &c); returns the hit or 0. */
#define W7L_CALL_1002dfbb SIM_FN(uint32_t(__cdecl *)(uint32_t, void *, void *, uint32_t *, uint32_t *, uint32_t *, \
                                                     uint32_t *), 0x1002dfbb)
/* Not converted: (face, points, x, y, z as float bits); non-zero when the point is inside. */
#define W7L_CALL_1002e91e SIM_FN(int(__cdecl *)(void *, void *, uint32_t, uint32_t, uint32_t), 0x1002e91e)
/* Not converted: (pointer). */
#define W7L_CALL_1003218b SIM_FN(uint32_t(__cdecl *)(uint32_t), 0x1003218b)
/* HeapFree on the game's heap (Hg_HeapFree, src/sim/crt/hg_winheap.c). */
/*
 * a in 0..2 or 4, b in 0..9: picks the 0x18-byte entries b of the tables PTR[0x101336b0 + 4 * n] for the
 * numbers n at 0x10133700 and 0x10133708 (a == 0) or at 0x101336f8 + 8 * a (second one 0); for b == 0 the entry
 * is the toggle at the dword after n, which flips. Calls 0x1001f790(first, second, -1) when one is set.
 */
void __cdecl Sim_100200d1(int a, int b);

/*
 * For the 0x50-byte record `index` at 0x109e9ba0: forgets [0x1017e678] when it points to an entry with bit 0x200
 * in word +2 and word +0x24 == index. Unless the record's word has bit 4: counts the result of
 * Sim_10052c02(index) (0, 1, 2) in the words 0x101653f1/ed/ef when the record [0x10134898] (>= 0) is the
 * current one [0x1017b8c8], and in 0x1016540a/06/08 when both are in the same group (+8); then Sim_1007cbb4
 * when the word has bit 0x40, and 0x100b45b4(record).
 */
void __cdecl Sim_1002597b(int index);

/*
 * Starts up to `limit` (0 means 0x100) of the idle entries (+0x18 == 0, kind +0x24 == 11, object +0) of the
 * 0x100 entries of 0x2c bytes at 0x109a5a40 that get a slot from Sim_10028b90(object, 1): +0x18 = 1,
 * +0x14 = [0x10147e14], Sim_100a9d47(object, a, b, c), Sim_10028c6f(slot), Sim_100aa39f and Sim_100aa799.
 */
void __cdecl Sim_10025c8c(uint32_t a, uint32_t b, uint32_t c, int limit);

/*
 * Fills the table of (pointer, size) pairs at 0x109f1480 from the pointers at 0x10134f80.. and sizes made of
 * the counts at 0x109f1448.. (times 0x2fc, Sim_10072ff2(), 0x100ab1f2(), Sim_100cc96c(), 0x2c, 0x14, 8, 4);
 * returns the table.
 */
void *__cdecl Sim_10027654(void);

/*
 * Frees a node of the hash table [0x1015dc30] (0x3f1 buckets by id word +0 plus the four signed bytes of the
 * name +4, chained at +8): word +2 = 0, Sim_10027cce(node), unlink, HeapFree and [0x109f13e0] - 1. A node that
 * is not in its bucket is reported (TapSystemError 0x20, and the text through Sim_1007df6c when [0x1015f1e8])
 * and Sim_10027d77 runs when it is [0x109f13f0]; it is not freed.
 */
void __cdecl Sim_10027fb8(void *node);

/*
 * Releases an object: unless its +0x90 text contains "WRF" or "WBZ" (then Sim_100aa406, Sim_100aa799 and
 * Sim_10038d65 run), 0x10029b1f(obj) and done(obj) when given. Then Sim_1002990f of Sim_1002997e(obj) when
 * that is not -1.
 */
void __cdecl Sim_10028fb0(void *obj, void(__cdecl *done)(void *));

/*
 * Starts the fading out of an object that has a pointer +0x8c (not for the kind [0x101352e4] at its +0x90):
 * in a free entry of the 0x80 entries of 0x14 bytes at 0x109f09e0 with a slot from Sim_10028b90(obj, 2) it
 * stores (1, obj, [0x1015dc78], 16.0, done) and calls Sim_100aa5e8(obj, 0x300), Sim_100aa4f0(obj, 0x50),
 * Sim_100aa541(obj, entry) and Sim_10028c6f(slot). Without a free entry or slot, or when (word +2 of +0x8c &
 * 0xf0) is 0x70 or (flag and its word +0x26 == 0) (then Sim_10009408(+0x8c, 0x50) first), it releases the object at once
 * (Sim_100aa8cb and Sim_10028fb0, or only Sim_10028fb0 when there is no slot).
 */
void __cdecl Sim_10028d9f(void *obj, void(__cdecl *done)(void *), int flag);

/*
 * Per frame for the player object: plays the pending sound [0x1015dd14] (+ Sim_1009c929(5)) once with
 * Sim_10061b78 and counts it in [0x1015dd24] (more than 4: Sim_1006863d(obj)); with [0x1015ef0c] set it starts
 * the "AFXHDMLG" / "AFXHDMLD" effects (Sim_1000ebfb) once when the word obj+0x560 gets bit 0x80 / 0x40 under the
 * conditions of the code; [0x1015dd18] = obj+0x4f0.
 */
void __cdecl Sim_1002c76d(void *obj);

/*
 * Clips the segment against the box obj+0x98 (Sim_1002fa3f(obj) first) with 0x1009cfd4 on the three axes;
 * returns 0 when it misses. Otherwise moves the segment end to the entry distance (Sim_1009cd36) and stores
 * the normal of the entry face (+-1.0 on the axis of entry) in 0x1015dd40..48; a negative entry distance is
 * kept in [0x1015dd3c] and replaced by 0. Returns 1.
 */
int __cdecl Sim_1002cf23(void *obj, void *seg);

/* 1 when the segment hits one of the faces (Sim_1002e790) of the model obj+0x30, else 0. */
int __cdecl Sim_1002d5e0(void *obj, void *seg);

/*
 * Nearest hit of the segment among the objects of the cell lists (Sim_100881dd of its start, table
 * 0x1036f2a0, nodes chained at +0x18) through 0x1002dfbb; objects with bit 0x100 in word +2 are skipped when
 * their word +0x24 is id or id < 0. *out = the hit node; on a hit the segment is cut (Sim_1009cfbd) and the
 * three dwords of the hit go to 0x1015dd58..60; returns 1, else 0. A segment not longer than f[0x101112c4]
 * uses Sim_1002dc60 instead. 0 when [0x1017c8cc] is null.
 */
int __cdecl Sim_1002dda9(void *seg, uint32_t *out, int id);

/* As Sim_1002dda9 without an id: skips every object with bit 0x100 in word +2 and those whose dword +0x34 is 6. */
int __cdecl Sim_1002e362(void *seg, uint32_t *out);

/*
 * Collision of the segment with an object by kind (dword +0x34, table of 0xc-byte entries at 0x10138bc0): the
 * segment test of the kind (+4) when it has one, else four steps of a quarter (f[0x101112d0]) along the
 * segment with the point test (+0) moving the segment end to the first point inside (normal 0x1015dd40.. =
 * 0, returns 1; 0 when none is inside), else Sim_1002e6de(obj, seg, t) and 1. 1 for a segment not longer than
 * f[0x101112c4].
 */
int __cdecl Sim_1002e569(void *obj, void *seg, float t);

/*
 * Segment against one face (normal at +0x18, first point index the byte at face + face[+4] into the 0x58-byte
 * points): when the segment runs against the normal and the hit distance is within [f[0x101112d4], seg+0x30]
 * and 0x1002e91e accepts the point, moves the segment end there (Sim_1009ceab), stores the normal at
 * 0x1015dd40.. and returns 1; else 0.
 */
int __cdecl Sim_1002e790(void *face, void *points, void *seg);

/* 0 for a null object or without +0x98; else by kind +0x34: 0 -> 0x78, 5 -> 0x1003218b(+0x98), others 0. */
uint32_t __cdecl Sim_1002ff6e(void *obj);

#endif
