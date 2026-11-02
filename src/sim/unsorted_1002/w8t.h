/*
 * Round 8, letter t: functions in 0x10020256..0x100273a1 (sound requests by kind, the per-tick updates of the
 * effect tables 0x1096fc80 / 0x109a3f80 / 0x109a5a40, the aiming of the mounted objects, damage of the 0x50-byte
 * records at 0x109e9ba0 and the loading of a named resource set). Helpers that are not converted (or that another
 * round-8 list converts) are called through their original address with SIM_FN; their prototypes are below.
 * Floating point as in unsorted_1000/w8t.h.
 */
#ifndef HG_SIM_UNSORTED_1002_W8T_H
#define HG_SIM_UNSORTED_1002_W8T_H

#include <stdint.h>
#include "../simmem.h"
#include "../libm/hg_math.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Not converted: per-entry updates of the table 0x1096fc80 (kinds 0 and 3) and 0x109a3f80 (by index). */
#define W8T_CALL_100232d7 SIM_FN(void(__cdecl *)(int), 0x100232d7)
#define W8T_CALL_100225c7 SIM_FN(void(__cdecl *)(int), 0x100225c7)
#define W8T_CALL_10021e8c SIM_FN(void(__cdecl *)(int), 0x10021e8c)
/* Not converted: table lookup of one float, returns a float value in st(0) (0x10084766). */
#define W8T_CALL_10084766_1002 SIM_FN(double(__cdecl *)(float), 0x10084766)
/* Not converted: table lookup of two floats, returns a float value in st(0) (0x1008486e). */
#define W8T_CALL_1008486e_1002 SIM_FN(double(__cdecl *)(float, float), 0x1008486e)
/* The C library fmod (x, y) of the DLL on doubles; the result in st(0) (0x100fcb30). */
#define W8T_FMOD_1002 hg_fmod
/* A C library function of the DLL on a double, the result in st(0) (0x100fcc74). */
#define W8T_CALL_100fcc74 hg_asin
/* Not converted: (0x12, time, 2) (0x100d3510). */
#define W8T_CALL_100d3510 SIM_FN(void(__cdecl *)(uint32_t, uint32_t, uint32_t), 0x100d3510)
/* Not converted here (it reads fields of its record argument that the original leaves uninitialised): the
 * effect damage of 0x100255de (7 dwords). */
#define W8T_CALL_100255de SIM_FN(void(__cdecl *)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t), 0x100255de)
/* Not converted: looks up the resource record `key` (word number, 12-character name) and fills out (0x100278c0). */
#define W8T_CALL_100278c0 SIM_FN(void *(__cdecl *)(void *, void *), 0x100278c0)
/* Not converted: (tag) (0x10027b69). */
#define W8T_CALL_10027b69 SIM_FN(void(__cdecl *)(uint32_t), 0x10027b69)
/* Patched by another hook file: the error report (code, argument) (0x100bc280). */
#define W8T_CALL_100bc280 SIM_FN(void(__cdecl *)(uint32_t, uint32_t), 0x100bc280)
/* The jump stub of Sim_100273a1 (0x10001a37) that it hands to Sim_100dafa8 / Sim_100db259 as the visitor. */
#define W8T_FN_10001a37 0x10001a37u

/* Queues the sound of kind (1..12, see the switch: Sim_100200d1 or Sim_1001f790 of the tables 0x10133728 and
 * 0x10133770); nothing for index < 0, kind -1 or kind >= 13. */
void __cdecl Sim_10020256(int32_t kind, int32_t index);

/* Advances the timers +0x30 (up) and +0x34 (down) by the frame ticks [0x1015dc80] of the active entries of the
 * 0x156-byte tables 0x1096fc80 (0x200, calling 0x100232d7 / 0x100225c7 by kind) and, unless [0x1017b92c],
 * 0x109a3f80 (0x14, calling 0x10021e8c). Clears the dword [0x1017694c] points to. */
void __cdecl Sim_10021d65(void);

/* Adds the turn of obj (lookup 0x10084766 of the angle obj[0x38], scaled by the rotated vector of Sim_10084c95)
 * to *a, *b and *c and advances the angle by the frame ticks (fmod 360). */
void __cdecl Sim_1002411e(void *obj, float *a, float *b, float *c);

/* Aims obj at its target (+0x40, kind +0x44: 0x200 object of the pointer table, 0x400 record of 0x109e9ba0)
 * from the point (x, y, z): see the code. */
void __cdecl Sim_100241ed(void *obj, float x, float y, float z, float *p, float *q, float *r);

/* Per-tick update of the 0x100 entries of 0x2c bytes at 0x109a5a40 (see the code). */
void __cdecl Sim_1002528f(void);

/* Damage dmg of the hit node to its record of 0x109e9ba0 (word +0x24); at 0 the record is destroyed with
 * Sim_10024616 (kind 0x24, 0x16 or 0x32) and Sim_1002597b. */
void __cdecl Sim_10025aee(uint32_t source, const void *hit, float dmg, uint32_t x, uint32_t y, uint32_t z);

/* Loads the resource set named name (a leading digit gives its number, Crt_AToI) through 0x100278c0 and
 * Sim_100273a1; returns Sim_10027654() when that succeeds, else 0. */
uint32_t __cdecl Sim_100272cb(const char *name);

/* Reads the records of the resource set src (Sim_100279f1) after the two header checks (errors 0x48, 0x49,
 * 0x3e through 0x100bc280) and hands each to its loader by tag; returns 1 when all succeed. */
int __cdecl Sim_100273a1(void *src);

#endif
