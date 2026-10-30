/*
 * Round 10: game functions of HGSIM.DLL around 0x10000000..0x1000ffff, converted one at a time.
 * Calls of functions that are not converted yet go through their original address (SIM_FN).
 */
#ifndef HG_SIM_UNSORTED_1000_W10_H
#define HG_SIM_UNSORTED_1000_W10_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* -10000 when d > [0x101110e0] (50000.0f), else trunc([0x101110e4] * d * d / [0x101110e8]) (-2500 / 250000)
 * clamped to -10000..0. */
int32_t __cdecl Sim_1000f096(float d);

/* Sphere (centre x, y, z, radius r) against the segment seg (unsorted_1009/w3o.h), result in st(0):
 * 3.4e38 ([0x1011101c]) when r < 1e-7, the centre lies behind the start or the line misses the sphere;
 * 10.0 ([0x10111024]) when the start is inside the sphere; 0.0 when the entry distance is negative; otherwise
 * the entry distance t, or 3.4e38 when t is not below the segment's length (Sim_1009cbde). */
float __cdecl Sim_10009fe8(float x, float y, float z, float r, void *seg);

/* Fire decision of unit obj for the aim angle a: when its next time +0x1be has come ([0x1015dc78]), r =
 * Sim_1009c929(byte +0x1b6) sets the next time ([0x1015dc78] + 22 r); with d = a - obj+0x74 and the heading
 * w = fmod(obj+0x64 + (obj+0x28)->+0x1c + obj+0xec, 360) both inside (-10, 10) (and, when the target in word
 * +0x1aa is not the own unit, Sim_1009c929(3) == 0): Sim_1001403a(obj, target), and when r == 0 the weapon
 * check (Sim_1000bf73 when +0x1c2 is set, else Sim_1000bba7) and Sim_1000a6f0(obj, 1) decide; on success
 * byte +0x28 (or +0x26) of obj+0x54 is set and the result is 1. Then Sim_1000c172(obj) when both the flag of the
 * check and the result are set, and obj+0x54 gets +4 = Sim_1001448d(obj, a, r) * 11520 and +0 =
 * -Sim_100144bb(obj, obj+0xec, r) * 3840 (r = 0 when the result is 0). */
int __cdecl Sim_1000d34d(void *obj, float a);

/* Lead aim of unit at its target (unit+0xf0 for the own unit, else word +0x1aa; only a unit target, kind 0x200):
 * with the target's velocity v (its record +0x548..+0x550, nothing when all three are below 1e-7), the closing
 * speed s = |v| (signed by the direction) and the shot speed w of the current weapon (float +0x3c of its kind),
 * t = unit+0xd8 / (w - s) (2.0 when |s| equals w); when t is at least 1e-7, not above the weapon's range (int +0x5c)
 * and above [0x1015dc80]: the predicted position p = target + v * trunc(t) (written to out) and Sim_100cbc6b(p - unit,
 * &angle, &b, &c, d), *heading = angle - unit+0x68 wrapped into -180..180. */
void __cdecl Sim_1000c1ea(void *unit, float *heading, float *d, float *out);

/* Fire decision of unit obj for the aim angle a (the variant with the weapon checks of its record obj+0x28): when the
 * next time +0x1be has come, r = Sim_1009c929(byte +0x1b6) sets it ([0x1015dc78] + 11 r); the weapon is checked
 * (Sim_1000ceb7, then Sim_1000bf73 / Sim_1000bba7 or Sim_1000b9f1), the aim offsets d = a - obj+0x74 (halved for
 * kinds 1 and 9) and the heading w = fmod(obj+0x64 + record+0x1c + obj+0xec (+ obj+0x1c6), 360) go to
 * Sim_1000b01f with the limits of Sim_1000ae6e; on success with r == 0 it fires (Sim_1000a6f0, Sim_1000c6e3; bytes +0x28 /
 * +0x26 of obj+0x54) and the result is 1. obj+0x54 gets +4 / +0 from Sim_1001448d / Sim_100144bb, and the record's
 * +0x4dc the next time from the entry found. */
int __cdecl Sim_1000b587(void *obj, float a);

/* AI fire control of obj (record +0x28, controls +0x54) at the wanted heading: when its timer +0x1be has run out it is
 * set again (Sim_1009c929 of +0x1b6, times 11), the shot is chosen (Sim_1000ceb7, Sim_1000bf73 / Sim_1000bba7,
 * Sim_1000b9f1), the lead aim taken (Sim_1000c1ea) and, when Sim_1000b01f accepts the aim, the unit turns to the
 * target (Sim_1001403a) and on a 0 draw fires (Sim_1000a6f0 / Sim_1000c6e3: control bytes +0x26 / +0x28). Then the
 * steering controls +4 / +0 (Sim_1001448d / Sim_100144bb) and the next shot time record+0x4dc. Returns 1 when it fired. */
int __cdecl Sim_1000b070(uint8_t *obj, float heading);

#endif
