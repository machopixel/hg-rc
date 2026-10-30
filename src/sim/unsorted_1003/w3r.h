/*
 * Round 3 (letter r), original 0x10030228..0x1003cc73: bounding-box/centre maths over the
 * 0x58-byte vertex records of a model, a comparison function, and small state updates.
 * Floats are evaluated in double and stored as float, like the x87 original (util/w3r.h).
 *
 * A vertex record is 0x58 bytes; the records of "obj" start at obj+0x18, their position is
 * the three floats at +0xc, +0x10, +0x14. A "part" lists the vertices it uses: word at +2 =
 * count, dword at +4 = offset (from the part) of the byte indices.
 */
#ifndef HG_SIM_UNSORTED_1003_W3R_H
#define HG_SIM_UNSORTED_1003_W3R_H

#include <stddef.h>
#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Bounding box and centre of the vertices listed by `part`: centre (*cx, *cy, *cz), minimum
 * and maximum of x, y and z as raw float dwords. Does nothing when any pointer is NULL. A
 * minimum is replaced unless the new value is >= it (so a NaN replaces), a maximum only when
 * the new value is > it. Start values are +-3.3999e38 (0x7f7fc99e, 0xff7fc99e).
 */
void __cdecl Sim_10030228(uint8_t *obj, uint8_t *part, float *cx, float *cy, float *cz, uint32_t *minx, uint32_t *maxx,
                          uint32_t *miny, uint32_t *maxy, uint32_t *minz, uint32_t *maxz);
/*
 * Average position of the vertex records of the model block at [[obj+0x18]+0x30] (count: signed
 * word at +4, records from +0x18) into obj+0x20..+0x28, plus the floats at obj+4..+0xc. Returns
 * 1, or 0 when obj, obj+0x18 or that block is NULL.
 */
int __cdecl Sim_10033744(uint8_t *obj);
/* Compares the floats at +4 of the two records (qsort style): -1 when a < b or unordered, 1 when a > b, else 0. */
int __cdecl Sim_10033e6f(const uint8_t *a, const uint8_t *b);

/* Calls the function at +0x14 of the record [0x1015e68c] and the one at +0x10 of the record [0x1015e688] (each only when the record exists), then sets [0x1013a2c8] = 1. */
void __cdecl Sim_10037c6c(void);
/* Rebuilds four multiples (5, 7, 0x11, 0x13) of [0x1013abc8] / 0x18 at 0x1015eba0.. and a few related state values. */
void __cdecl Sim_1003c970(void);
/* Multiplies the float at +0x4ec of every gear's data record (table 0x109e8b40, count [0x1015dc64], pointer at +0x28) by the float at 0x101114fc. */
void __cdecl Sim_1003cc73(void);

#endif
