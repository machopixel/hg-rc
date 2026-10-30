/*
 * Interpreted functions: the original takes a value from the C runtime's x87 sin / cos / asin / acos and keeps
 * computing with it on the x87 stack, which a C double cannot reproduce bit for bit. These versions call the
 * platform math library instead; everything around the math is exact. See DECISIONS.md at the repository root.
 */
#ifndef HG_SIM_UTIL_W10_INTERP_H
#define HG_SIM_UTIL_W10_INTERP_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Angles in degrees of the 3x3 rotation matrix m (row-major floats): pitch = asin(m[5]) when |m[5]| <= 0.1, else
 * acos of the horizontal length Sim_10084b57(m[2], m[8]) signed by m[5]; heading = Sim_1008486e(m[2], m[8]) and
 * roll = Sim_1008486e(m[3], m[4]). Straight up or down (length <= [0x1012b528]) gives heading -+90, pitch 0 and
 * roll Sim_1008486e(m[1], m[0]). */
void __cdecl Sim_10060681(const float *m, float *pitch, float *heading, float *roll);

/* Aims node along (dx, dy, dz): rotation (pitch -asin(dy) in degrees, heading Sim_1008486e(dx, dz)) through
 * Sim_100aa0da, position from Sim_100d709d(obj, .., q), moved by a random -100..99 (Sim_1009c929(200)) along the
 * normalised side and up vectors when spread is set, placed with Sim_100a9d47; unless flag 0x40 of weapon+8 the
 * direction scaled by the speed (node+0x8c)+0x50 * [0x1012bcf4] goes to Sim_100a9d7c. */
void __cdecl Sim_100d71b9(void *obj, uint8_t *node, float dx, float dy, float dz, int spread, const uint8_t *weapon,
                          void *q);

/* Builds the ballistic table 0x10a3a600 (0xa8-byte rows, count in 0x1015d0a0) for the 0x2e weapon kinds (0x10145508,
 * 0xd6 bytes) with flag 0x40 of +0x86: per row the kind, 20 / (max - min range) and 20 ranges from min (+0x4c) to max
 * (+0x50), each with its launch angle 0.5 * asin(g * range / speed^2) in degrees (45 when out of reach); g starts
 * at [0x1013abc0] and is multiplied by each listed kind's +0x38, as the original does. */
void __cdecl Sim_1000cbaa(void);

/* Quaternion slerp out = a..b at t (four floats each; asserts the three pointers): spherical with
 * sin((1 - t) w) / sin w and sin(t w) / sin w, w = acos(a . b), linear when 1 - a . b <= 1e-5; for nearly opposite
 * ones (1 + a . b <= 1e-5) out becomes the perpendicular (-a1, a0, -a3, a2) and its first three parts the blend
 * sin((1 - t) pi/2) a + sin(t pi/2) out. */
void __cdecl Sim_100608ef(const float *a, const float *b, float t, float *out);

/* Rotation matrix m (3x3 floats, then the translation tx, ty, tz at +0x24) from the angles ax, ay, az in degrees:
 * a single axis (the others below [0x1012b510]) fills an identity (0x1006018d) with that axis' cos / sin; else
 * the general product by order flags & 3 (0..2, completed by 0x1005fcdc; 3 leaves the 3x3 alone), with flag 4
 * negating the sines and transposing the result (0x1006007d). */
void __cdecl Sim_10060220(float *m, float ax, float ay, float az, uint32_t tx, uint32_t ty, uint32_t tz,
                          uint32_t flags);

/* A unit following a path (the task records "key;how,turn,name" of the world files). Mode 0 makes the record
 * (HeapAlloc, 0x88 bytes, kept at Sim_100cc8ba(Sim_100cc6e0())): how "loop" 0 / "repeat" 1 / else 2 (+0x18), turn
 * "rotate" (+0x14), the path called name (0x10a00040, 0x744 bytes each, [0x1015d17c] of them) at +8; mode -1 only
 * restarts it at time. With a path both restart the six interpolators (+0x1c.., Sim_10047b76 / Sim_10047c92), sum
 * the path's length +0x10 and find the unit +0 by the text's key (0 when there is none). Mode 1 places the unit's
 * node at the point of the path for time (loop / repeat / stop at the end): position by Sim_10047bcd, the heading
 * from the path (and Sim_1008486e of the segment when turning), the pitch less asin(rise / [0x101111d8]) in degrees
 * when turning, and the roll, each wrapped to -180..180 and smoothed by Sim_10047cf2, then Sim_100aa799. */
int __cdecl Sim_1001a706(int mode, char *text, int32_t time);

#endif
