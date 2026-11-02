/*
 * Round 8, letter x: functions at 0x100908b3..0x1009db70. The effect pool's set-up (0x100908b3), effects drawn
 * as lines, pixels and screen quads, the debris effect's start (0x10092f9d), the camera views of the cockpit
 * (0x10099c36 .. 0x1009bba2) with their compass needles, the HUD markers of units and objects
 * (0x1009a282 .. 0x1009aac9), the random tables of Sim_1009c929 (0x1009c860) and a node tree copy (0x1009db70).
 * Helpers that are not converted yet (or that another hook file patches) are called through their original
 * address with SIM_FN and the prototypes below.
 */
#ifndef HG_SIM_UNSORTED_1009_W8X_H
#define HG_SIM_UNSORTED_1009_W8X_H

#include <stdint.h>
#include "../simmem.h"
#include "../libm/hg_math.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Not converted (in the w8z list): projects the point (*x, *y, *z) in place, non-zero when it is on the screen. */
#define W8X9_CALL_100d2669 SIM_FN(int(__cdecl *)(float *, float *, float *), 0x100d2669)
/* Not converted: one pixel (pane, x, y, colour). */
#define W8X9_CALL_100e84f0 SIM_FN(int(__cdecl *)(void *, uint32_t, uint32_t, uint32_t), 0x100e84f0)
/* Not converted (in the w8w list): table lookups of one / two floats, result in st(0). */
#define W8X9_CALL_10084766 SIM_FN(double(__cdecl *)(float), 0x10084766)
#define W8X9_CALL_1008486e SIM_FN(double(__cdecl *)(float, float), 0x1008486e)
/* fmod and sqrt from OpenLibm (hg_fmod, hg_sqrt), in place of the DLL's C library. */
#define W8X9_CALL_100fcb30 hg_fmod
#define W8X9_CALL_100fca04 hg_sqrt
/* Not converted: (a, b, c, 0xf, 0), see Sim_100993a3. */
#define W8X9_CALL_10098f8c SIM_FN(int32_t(__cdecl *)(uint32_t, uint32_t, uint32_t, int32_t, int32_t), 0x10098f8c)
/* Not converted (in the w8z list): (0x11, 0xb5, 1), see Sim_100993a3. */
#define W8X9_CALL_100d3510 SIM_FN(int(__cdecl *)(int32_t, int32_t, int32_t), 0x100d3510)
/* Patched by hooks_view.c: the gears' level of detail. */
#define W8X9_CALL_1003b534 SIM_FN(void(__cdecl *)(void), 0x1003b534)
/* Not converted: the parts of a camera view drawn after the scene / at the end of the frame. */
#define W8X9_CALL_1009a137 SIM_FN(void(__cdecl *)(void *), 0x1009a137)
#define W8X9_CALL_1009b03d SIM_FN(void(__cdecl *)(void *), 0x1009b03d)
/* Not converted: (pane, [0x10138c78]), see Sim_10099ef3. */
#define W8X9_CALL_100eb070 SIM_FN(void(__cdecl *)(void *, uint32_t), 0x100eb070)
/* Not converted: (5) gives non-zero while that condition holds. */
#define W8X9_CALL_100766ca SIM_FN(int(__cdecl *)(int), 0x100766ca)
/* Not converted (in the w8v list): (pane, target, 0, 0), see Sim_1009bba2. */
#define W8X9_CALL_10048287 SIM_FN(int(__cdecl *)(void *, uint32_t, int, int), 0x10048287)
/* Not converted: upper-cases its two texts in place and looks the first up as a shape resource (0 when absent). */
#define W8X9_CALL_100ba7d5 SIM_FN(void *(__cdecl *)(void *, void *, int), 0x100ba7d5)
/* Patched by hooks_hud.c / not converted: draw a shape (pane, shape, number, x, y). */
#define W8X9_CALL_100f0205 SIM_FN(int(__cdecl *)(void *, void *, uint32_t, uint32_t, uint32_t), 0x100f0205)
#define W8X9_CALL_100e91dc SIM_FN(int(__cdecl *)(void *, void *, uint32_t, uint32_t, uint32_t), 0x100e91dc)
/* Patched by hooks_hud.c / not converted: draw a line (pane, x0, y0, x1, y1, mode, colour). */
#define W8X9_CALL_100ef727 SIM_FN(int(__cdecl *)(void *, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t), 0x100ef727)
#define W8X9_CALL_100e86a1 SIM_FN(int(__cdecl *)(void *, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t), 0x100e86a1)
/* Not converted (in the w8z list): (dx, dy, dz, four result pointers), see Sim_1009a3b3. */
#define W8X9_CALL_100cbc6b SIM_FN(void(__cdecl *)(float, float, float, void *, void *, void *, void *), 0x100cbc6b)

/*
 * Fills the effect pool: up to 1000 effect objects (Sim_10008f1c(3, 1, 0xc, &d), counted in [0x1017cc20]) with
 * three point records, a record +d made of word i | 0x8000 (floats +0x14 = +0x20 = 1.0), a scene node
 * (Sim_100a9830(0, 0xc)) linked both ways, filed with Sim_10089e91, and the pool entry i at 0x10217c20 (0x57e
 * bytes, 32 particle slots of 0x2a from +0x12) cleared with +8 = i, +0x552 = node; 0x10216b80[i] = i. Stops at
 * the first failed object; [0x10215ae8] = count - 1, then Sim_100905e8.
 */
void __cdecl Sim_100908b3(void);

/*
 * The effects below draw their particles (16 of 0x2a bytes from +0x12, positions relative to the node +0x552
 * through Sim_100a9cfb unless noted) after projecting them with 0x100d2669, with colour +0x566, through the
 * display driver slot 0x1014cb4c as (count, 36-byte vertices, flags); vertex fields the original leaves unset
 * stay unset. They return 0 for a null object, else 1.
 * Sim_10091223: 8 jagged lines, each from particle i to a point of it moved by 0..99 (Sim_1009c929) - [0x1012b93c]
 * per axis and on to particle i + 1 (two 2-vertex lines, flags colour | 0xa000).
 */
int __cdecl Sim_10091223(uint8_t *obj);
/*
 * Quads of size trunc(+0x56a * 2000 / depth) (at least 1); with size below 1 and not in the high-colour mode
 * ([0x1017b940] = 0) a pixel (0x100e84f0 on 0x109e3e00) instead. The node position is projected once first.
 */
int __cdecl Sim_10092bbb(uint8_t *obj);
/* As Sim_10092bbb without the first projection, each particle's own colour +0x1e and no depth in the vertices. */
int __cdecl Sim_10093e5b(uint8_t *obj);
/* Quads of size 1.0 for every particle, or every second one when the node's projected depth is above [0x1012b990]. */
int __cdecl Sim_10096163(uint8_t *obj);
/* 10 particles: pixels (not in the high-colour mode) or quads of size [0x1012b970]. */
int __cdecl Sim_10096869(uint8_t *obj);
/*
 * Starts a debris effect at (x, y, z) when the pool has room ([0x1017cc24] + 1 < [0x1017cc20], k non-zero) and
 * the point is within the camera's range +0x40: an effect from Sim_10090c7d with speed |v| at its block +0x50,
 * its 16 particles scattered around the origin (offsets 0..9 - [0x1012b948], 0..29 - [0x1012b954]) moving at
 * k * 80 + 0..k * 80 - 1 in a random direction (sin / cos table 0x10084766), lifetime k * 181.
 */
void __cdecl Sim_10092f9d(float x, float y, float z, float vx, float vy, float vz, int32_t k);

/* 0x10098f8c(a, b, c, 0xf, 0); above 60 also 0x100d3510(0x11, 0xb5, 1). Returns the first answer. */
int32_t __cdecl Sim_100993a3(uint32_t a, uint32_t b, uint32_t c);
/* Sim_100999fc, then Sim_1009bba2 when the word +6 of [0x109e88e0] is set, else Sim_10099c59. */
void __cdecl Sim_10099c36(void);
/*
 * Draws the camera view of mode [0x1015d1ac] (view record 0x1014c4c8[mode]: pane +0, slot +8, scale +0x18, info
 * +0x74, end callback +0x7c): copies the pane's 5 dwords to 0x10185400 + slot * 0x14, puts the camera at the
 * player's node position ([0x1015da24] = 0) or the camera's, with heading from the mode, renders (Sim_1008fb20
 * .. Sim_1008fd5a; mode 4 sets the display routines 0x1014cb40 / 48 / 4c and renders the scene through the
 * level-of-detail update) and draws the compass needles (Sim_1009ace7) and the view's parts.
 */
void __cdecl Sim_10099c59(void);
/* As Sim_10099c59 for mode 4 into the given pane (mode restored after; left at 4 when the view is missing). */
void __cdecl Sim_10099ef3(void *pane);
/* The two needles of the compass (view +0x88 gives the end point of an angle) around the pane's centre. */
void __cdecl Sim_1009ace7(void *view, float angle);
void __cdecl Sim_1009ae96(void *pane, void *view, float angle);
/* The timed switching between the views of the record +0x14 of the current view (state [0x1017cc50]). */
void __cdecl Sim_1009ba57(void);
/* Draws the current view (Sim_10099c59) and, now and then for a view with a target, the target's picture. */
void __cdecl Sim_1009bba2(void);

/*
 * A HUD marker: the point (x, y, z) projected (Sim_10090099) and checked by the pane object's +0x80; when seen
 * and (b = 0 or the integer at 0x1014be6d + min(b, 4) * 0x1d is at least f / [0x1012b9c0]) the shape `name`
 * drawn there (a = 1 sets [0x1017cc5c]).
 */
void __cdecl Sim_1009a282(void *obj, uint32_t x, uint32_t y, uint32_t z, char *name, int32_t a, int32_t b, uint32_t f);
/* The markers of all other units (team picked by Sim_10052be7) and of the objects 0x109e9ba0 (Sim_10052c02). */
void __cdecl Sim_1009a3b3(void *obj);
/* The marker of the player's target (+0xdc .. +0xe4, target word +0xf0) and of +0x104 .. +0x10c (+0x118). */
void __cdecl Sim_1009a6f3(void *obj);
/* The markers of the entries 0x109a8640 (0x54 bytes, [0x1017e5a8] of them) that belong to [0x1013bb40]. */
void __cdecl Sim_1009aac9(void *obj);

/*
 * Seeds the C library random generator (Sim_10100390), fills 0x101cffc0[0..126] with Crt_Rand and
 * 0x101cfce0[0..126] with trunc((sum of 30 Crt_Rand * [0x1012b9e8] * sqrt(90) / [0x1012b9f0] - sqrt(90)) *
 * [0x1012b9f8]).
 */
void __cdecl Sim_1009c860(uint32_t seed);
/*
 * Copies a node tree (first +4, next +8) into new 0x28-byte records (HeapAlloc of [0x1017b8f8]): +8 the node,
 * +0xc its +0, +0 / +4 the copies of its +4 / +8, +0x10.. its position (Sim_100a9d21), +0x1c.. Sim_100a9ccf.
 * Returns the copy, 0 for a null node or a failed allocation.
 */
void *__cdecl Sim_1009db70(void *node);

#endif
