/*
 * Round 8, letter z: functions of HGSIM.DLL at 0x100d2669..0x100daa08.
 *
 * The x87 arithmetic of the originals runs with 53-bit precision (control word 0x27f); it is done here in double
 * with every float store kept where the original rounds, so the results are the same bit for bit.
 *
 * Callees that are not converted (or patched elsewhere) are called through their original address (SIM_FN):
 *   0x100ba7d5 (name, extension, out) -> resource      0x100bb2b0 is the converted Sim_100bb2b0
 *   0x100f01ea (index) -> value                         0x10062386 ()
 *   0x100eda16 (record, record, step, value) -> value   0x100eb070 (record, value)
 *   0x100e85cb (pane, x, y) -> colour index             0x100e86a1 / 0x100ef727 (pane, x0, y0, x1, y1, 0, colour)
 *   0x100d965e (x, y, z, vx, vy, vz, owner)             0x100b9203 (name, extension)
 *   0x100e4579 / 0x100e4622 / 0x100e45e7 / 0x100e469b   the timers (patched by hooks_core.c)
 * and through pointers: the routines at [0x1015e688] + 0x14 / 0x18 / 0x1c and [0x1015e68c] + 0x18, the
 * renderer plugin slot 0x109e3d44.
 */
#ifndef HG_SIM_UNSORTED_100D_W8Z_H
#define HG_SIM_UNSORTED_100D_W8Z_H

#include <stdint.h>
#include "../simmem.h"
#include "../unsorted_100f/w8z.h"
#include "../crt/hg_winheap.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#define __stdcall
#endif

typedef uint32_t(__cdecl *W8zResource3Fn)(const char *name, const char *ext, void *out);
typedef uint32_t(__cdecl *W8zIndexFn)(uint32_t index);
typedef void(__cdecl *W8zVoidFn)(void);
typedef uint32_t(__cdecl *W8zStepFn)(void *a, void *b, int32_t step, uint32_t value);
typedef void(__cdecl *W8zFinishFn)(void *a, uint32_t value);
typedef int(__cdecl *W8zPixelFn)(void *pane, int32_t x, int32_t y);
typedef int(__cdecl *W8zLineFn)(void *pane, int32_t x0, int32_t y0, int32_t x1, int32_t y1, int mode, uint32_t colour);
typedef void(__cdecl *W8zSpawnFn)(float x, float y, float z, float vx, float vy, float vz, void *owner);
typedef int(__cdecl *W8zNamePairFn)(const char *name, const char *ext);
typedef int(__cdecl *W8zTimerAllocFn)(unsigned which);
typedef int(__cdecl *W8zTimerFn)(unsigned timer);
typedef void(__cdecl *W8zTimerVoidFn)(unsigned timer);
typedef void(__cdecl *W8zDrv4Fn)(uint32_t a, uint32_t b, void *data, uint32_t d);
typedef void(__cdecl *W8zDrv1Fn)(void *data);
typedef void(__cdecl *W8zDrv2Fn)(void *data, uint32_t b);
typedef void(__cdecl *W8zSlotFn)(uint32_t a, void *table);

#define W8Z_CALL_100ba7d5_D SIM_FN(W8zResource3Fn, 0x100ba7d5)
#define W8Z_CALL_100f01ea SIM_FN(W8zIndexFn, 0x100f01ea)
#define W8Z_CALL_10062386 SIM_FN(W8zVoidFn, 0x10062386)
#define W8Z_CALL_100eda16 SIM_FN(W8zStepFn, 0x100eda16)
#define W8Z_CALL_100eb070 SIM_FN(W8zFinishFn, 0x100eb070)
#define W8Z_CALL_100e85cb SIM_FN(W8zPixelFn, 0x100e85cb)
#define W8Z_CALL_100e86a1 SIM_FN(W8zLineFn, 0x100e86a1)
#define W8Z_CALL_100ef727 SIM_FN(W8zLineFn, 0x100ef727)
#define W8Z_CALL_100d965e SIM_FN(W8zSpawnFn, 0x100d965e)
#define W8Z_CALL_100b9203 SIM_FN(W8zNamePairFn, 0x100b9203)
#define W8Z_TIMER_ALLOC SIM_FN(W8zTimerAllocFn, 0x100e4579)
#define W8Z_TIMER_RESET SIM_FN(W8zTimerVoidFn, 0x100e4622)
#define W8Z_TIMER_ELAPSED SIM_FN(W8zTimerFn, 0x100e45e7)
#define W8Z_TIMER_FREE SIM_FN(W8zTimerVoidFn, 0x100e469b)
#define W8Z_DRIVER(off) (U32(G32(0x1015e688), (off)))

/* ---- w8z_project.c: points to the screen ---- */
/*
 * Moves the point (*x, *y, *z) to the view: subtracts the floats 0x10185c48 / 44 / 4c and takes the dot products
 * with the rows 0x10185bfc.. 0x10185c1c (float results). *z gets the depth; a depth not above [0x10185c50] is
 * mirrored (negative) or set to 1.0 (|depth| below [0x1012bcc4]) and gives 0. *x, *y become the screen point
 * trunc(x / depth) + [0x10185bd4], [0x10185be0] - [0x10185bf0] - (trunc(y / depth) + [0x10185bf4]) as floats;
 * returns 1 when it is inside [0x10185bd0]..[0x10185bec] x [0x10185bf0]..[0x10185be0], else 0.
 */
int __cdecl Sim_100d2669(float *x, float *y, float *z);
/*
 * For the two points at obj+0x12 and obj+0x3c (three floats each) moved by the position of the object
 * [obj+0x552]: when the point is on screen, draws a line from it along the vector obj+0x56e / [0x1012bd6c] with a
 * colour next to the one 0x100e85cb finds under it (through the patched 0x100ef727 while [0x1017b940] is set,
 * else 0x100e86a1). 0 for no object, else 1.
 */
int __cdecl Sim_100d8e43(void *obj);
/* The same for the first point only, vector and colour table from the record [obj+0x57a] (colour by depth). */
int __cdecl Sim_100d943f(void *obj);

/* ---- w8z_effects.c ---- */
/*
 * Refreshes the two dwords +0xbc / +0xac of the object with Sim_10047af2 around a test of the segment
 * Sim_100d6f16 gives: when Sim_1002dda9 finds something with flag 0x100 or 0x200 in its word +2, +0xc4 gets the
 * segment's length (Sim_1009cbde, truncated; 2000 when not above [0x1012bd38]) and, for the current unit, the
 * find goes to [0x1017e678]. +0xb0 = +0xc4. Returns the find (or 0).
 */
uint32_t __cdecl Sim_100d6d6a(void *obj);
/*
 * For the nine emitters at 0x10180a00 (0x80c bytes each, in use when +0 is not 0): adds [0x1015dc80] times the
 * rate +0xf8 to the fraction +0xf4 and spawns its whole part (Sim_100d9064 for [0x1015245c] = 1, 0x100d965e for 2)
 * at +0xc..+0x14 plus the camera [0x1014cae0] plus a random spread of +0xe4..+0xec.
 */
void __cdecl Sim_100d82f6(void);

/* ---- w8z_palette.c: palettes (names of 16 bytes at 0x10185620, extension [0x1014ced4]) ---- */
/*
 * While [0x1017b940] is set: loads palette 0 (0x100ba7d5), hands it to the driver ([0x1015e688] + 0x14), stores
 * the 256 colours (bytes through Sim_1004b804, times 4; colour 255 is 0x808080) at 0x10185760 and the matching
 * words (0x100f01ea of the index) at 0x101659e0, then gives the table to the plugin slot 0x109e3d44 and calls
 * 0x10062386. The argument is not used.
 */
void __cdecl Sim_100d3000(void *unused);
/* Loads palette `index`, runs Sim_100d3000, gives it to the driver ([0x1015e688] + 0x18) and releases it. */
void __cdecl Sim_100d3427(int32_t index);
/*
 * Starts a fade (Sim_1008c6f5 on the record 0x1017e620) from the current palette (or palette [0x10152420]) to
 * palette `index` over a number of steps worked out from `time` and [0x1015dc80]; mode 1 halves the time, mode 2
 * makes a one-step record. Returns 1 when the fade was set up.
 */
int __cdecl Sim_100d3510(int32_t index, int32_t time, int mode);
/*
 * Loads the palette 0x10185720 and runs Sim_100d3000; flag 0: hands it to the driver (+0x1c with 60, then
 * +0x18). Otherwise (while [0x1017b940] is 0) copies it through a heap block and the records 0x109e3e00 /
 * 0x109e3e20 with 0x100eda16 once per timer tick until the timer reaches 0x10f. Sets 0x10152420 and
 * 0x10152424 to 0x10.
 */
void __cdecl Sim_100d3877(int flag);

/* ---- w8z_lists.c ---- */
/*
 * Loads the name with extension "MLT" (0x100ba7d5); for a resource with 0 at +0 adds its [+0x28] records of
 * 0x60 bytes (from +0x2c) as heap nodes (0x38 bytes: copy pointer +0x34, first dword +0x10, name +0x14) to the
 * list 0x1017e6d0 (Sim_100da4f7; rejected nodes are freed) and the count 0x1017e6c4. Returns 1 when loaded.
 */
int __cdecl Sim_100da805(const char *name);
/* The same for "TLT" resources (+0 must be 1, records of 0x48 bytes, the dword and name at +0x20), list
 * 0x1017e740, count 0x1017e6c8. */
int __cdecl Sim_100daa08(const char *name);

#endif
