/*
 * Round 22: the renderer group of HGSIM.DLL (DECISIONS.md, "Round 22").
 */
#ifndef HG_SIM_RENDER_W22_H
#define HG_SIM_RENDER_W22_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Draws entry idx (0 .. [0x1017d1c8] - 1) of the 0x34-byte draw-item table 0x101c57e0, by its kind +4:
 * - kind 1 (with [0x1014d1d8] set): the object +8, skipped when it is the player's ([0x1017b8c8] at +4) and
 *   [0x1015ef0c] is set. An object of type 1 or 9 (+0) whose position +0x58..+0x60, projected by Sim_100d2669, lies
 *   beyond the depth [0x1012bab0] (or any type 9), and whose model +0x48 has a sprite +0x8c, is drawn as that sprite
 *   (Sim_100ad5b7 with the entry's +0x14..+0x1c and the sprite's half size +0x50); otherwise the model is drawn with
 *   Sim_100ad87d(entry, model);
 * - kinds 2 and 6: drawn by 0x100ac9ab(entry) when 0x100585c4(-1) answers non-zero.
 */
void __cdecl Sim_100ad96e(int32_t idx);

/*
 * Hands the queued polygon rec to the display: its +0 (signed word) vertex pointers from +0x10 are copied into the
 * plugin vertex array 0x101c9ec0 (0x24 bytes each: x / y as floats of the vertex's integer screen +0x18 / +0x1c, z
 * +8, light +0x3c at +0x10, u / v +0x20 / +0x24 at +0x1c / +0x20, the byte +0x2b in the top of +0x18). Then:
 * - one vertex and [0x1014cb00] set: a point through the graphics library 0x100e84f0 (unless the colour word +2 is
 *   0x1000);
 * - two vertices and [0x1014cafc] set: a line through 0x100ef727 (accelerated, the colour through the table
 *   0x101659e0) or 0x100e86a1;
 * - otherwise the driver's polygon slot [0x1014cb4c](count, vertices, flags), by the display kind [0x1014cb1c]:
 *   0 also draws 0x1009fada when bit 0 of +0xc is set, 1 draws the plain kinds twice (flags 0xf0, then flags with
 *   0xa000), 4 skips the colour 0x10, others add 0xa000 to the flags.
 * The library functions are called at their original addresses (plain cdecl; DECISIONS.md round 22).
 */
void __cdecl Sim_100a539e(void *rec);

/*
 * The spark ring of a polygon: the bounding box of its count vertices (0x24 bytes each, x / y floats) gives a centre
 * (each axis: _ftol of max + min, halved). Two of its vertices, from [0x1017d110] modulo count, each give four
 * sparks: white or blue (0x1009c929(100) < 50), +0x10 = 0xff, +0x18 = 0xff000000, z / rhw / u / v from the vertex,
 * and the position the centre plus 0x1009c929(2) for the first, the vertex plus 0x1009c929(5) for the others.
 * The eight go to the renderer [0x109e3d30] as lines (mode 0x10, surface 0x109e3dc0); [0x1017d110] then counts
 * up. (The original's line strip without the renderer is gone, round 42.)
 *
 * Deviation (DECISIONS.md round 22): the texture record handed with the lines is {0, 0x40}; the original never writes
 * its second dword (0x40 in every watched call), and the vertex alpha bytes +0x17, which it never writes either, are 0.
 * Neither renderer reads them for mode 0x10.
 */
void __cdecl Sim_1009fada(int32_t count, void *verts);

/*
 * The display driver's polygon routine (the slot [0x1014cb4c]): count plugin vertices (0x24 bytes) drawn by the kind
 * flags & 0xf000:
 * - 0 (flat colour flags & 0xff) and 0x1000 (Gouraud, colour +0x10 of each vertex plus flags & 0xf0): the colour
 *   bytes +0x14..+0x16 from the palette 0x10185760 scaled by [0x10143fdc..fe4] >> 8, +0x10 0xff (and +0x18
 *   0xff000000 for kind 0), then the renderer [0x109e3d30] in mode 0xd (the software side is gone, round 42);
 * - 0x4000: fog in +0x10 ((u32)+0x10 / [0x1012ba14] * [0x1012ba18] + [0x1012ba20], truncated; 0xff minus that unless
 *   [0x1017d108]), then mode 0xd;
 * - 0xa000: lines, accelerated with [0x1017b948] (mode 0x10) or through 0x100a2b8c round the polygon;
 * - 0x2000 / 0x7000 / 0xb000..0xe000: 0x10006744(colour, count, vertices, 1 for 0xb000.., else 0) with [0x1014caf4];
 *   0x5000 / 0x6000: 0x10006609 (0x6000 with [0x1014cb34] set to 1 around it); 0x3000: 0x10098f30(flags, count,
 *   vertices, -1); 0x8000 / 0x9000: 0x10090f17 / 0x100ad96e (flags & 0xfff).
 *
 * Deviations (DECISIONS.md round 22): the record handed to the renderer as its texture argument has 0 where the
 * original never writes it (neither renderer reads that argument in modes 0xd and 0x10); the fourth of the seven
 * arguments the original pushes to 0x10006609 is its never-written local [ebp-4], 0 here (0x10006609 reads three).
 */
void __cdecl Sim_1009ef50(int32_t count, void *verts, uint32_t flags);

/*
 * The star of an item (the 0x34-byte draw-item entry: position +0x14..+0x1c, size +0x2c, each truncated): four
 * 4-vertex fans of 0x58-byte clip vertices drawn by 0x100ac738, each from the centre (u 24.0) to three points at the
 * size s or s + s / 2 (u 16.0) in the x / z plane. [0x101c9ea8] is cleared first.
 * The original never writes the clip vertices' +0..+8 and +0x31..+0x57; they are 0 here (DECISIONS.md round 22).
 */
void __cdecl Sim_100ac9ab(void *item);

/*
 * The sky band (with the sky texture name [0x1017d0b0] set): four 0x58-byte clip vertices around the camera
 * [0x1014cae0], from its heading +0xc (wrapped into [0x1012ba28] .. [0x1012ba30]) and pitch ([0x1012ba2c] - +0x10)
 * through Sim_10084766, at the radius [0x1014c9c8], the top [0x1012ba38] above and the bottom [0x1012ba3c] below the
 * camera height +4; u / v from the position past the cell [0x1014c9e8] boundary below the nearest corner plus the
 * scroll [0x1017d104] (advanced, as [0x1014c9f4], by [0x1015dc80] * [0x1014c9f8] / [0x1012ba40] and reset at
 * [0x1012ba44]); then the camera position is added, the band clipped by Render_ClipPolygon with [0x1014cb34] cleared
 * and drawn by 0x100a0339 with the sky texture (loaded once by 0x100ba7d5 into [0x1017d100]).
 * The original never writes the clip vertices' +0..+8 (nothing reads them): 0 here (DECISIONS.md round 22).
 */
void __cdecl Sim_100a0880(void);

/*
 * The view background: the horizon test of the four screen corners ([0x10185bd0] / [0x10185bec] left / right,
 * [0x10185bf0] / [0x10185be0] top / bottom) by 0x10052e60 picks one of 16 cases; the sky ([0x10138c74], with
 * [0x1014cb04]) and the ground ([0x10138c78] with [0x1014cb08], else 0) are drawn as quads / triangles / pentagons
 * (0x100a291d / 0x100a29da / 0x100a2823) split where the horizon crosses the edges (0x10052f3a / 0x10052edc). With
 * [0x1014cb0c] some cases also give a haze band (its width and depth from the view matrix through 0x1005fbb0),
 * drawn with [0x1014caec] (unless [0x1017d0ac]) through the renderer in mode 2 with the haze texture (loaded once
 * into [0x1017d0fc]) or the driver's polygon slot [0x1014cb4c]. With the sky flag +0x38 of [0x1015d1b8] it then
 * draws the sky band (Sim_100a0880).
 * The original never writes the haze vertices' alpha bytes: 0 here (DECISIONS.md round 22; unread in modes 2 / 0xd).
 */
void __cdecl Sim_100a0def(void *view);

/*
 * The 16 sparks of an object's particle burst (points +0x12 + i * 0x2a, 0x2a bytes apart, plus the base 0x100a9cfb
 * gives for +0x552): each projected by Sim_100d2669 and, when visible, drawn at the size +0x56a * 2000 / depth:
 * without the renderer plugin as points through the graphics library 0x100e84f0 (colour 0xf0), with it as a quad of
 * four plugin vertices through the driver's polygon slot [0x1014cb4c] (colour 0xf0). Returns 0 for a null object,
 * else 1.
 * Deviation (DECISIONS.md round 22): z (+8) of the four vertices, which the original never writes (stack leftovers),
 * is 0.
 */
int __cdecl Sim_100945b8(void *obj);

/*
 * The sparks of a fountain (0x1009353e: each of the 16 points +0x12.. first moved by its velocity +0x1e..; the size
 * +0x56a * 2000 / depth, at least 1 with the renderer plugin) and of a cloud (0x1009596a: size 1; when the base point
 * is not beyond the depth [0x1012b980] a second point offset by +0x56e.. / [0x1012b958] is projected too, unused),
 * colour +0x566, drawn as points (0x1009353e without the plugin) or as quads through the driver slot [0x1014cb4c].
 * Both first project the base point with x and y in one local, as the original. Return 0 for a null object, else 1.
 * Deviation (DECISIONS.md round 22): z (+8) of the four vertices, never written by the original, is 0.
 */
int __cdecl Sim_1009353e(void *obj);
int __cdecl Sim_1009596a(void *obj);

/*
 * A beam (0x100918e6 and 0x1009221d, which differ only in instruction order): the two points +0x12.. and +0x1e..
 * projected by Sim_100d2669, drawn through the driver slot [0x1014cb4c] as a quad thickened by [0x1012b948] in y
 * (wider than high), x (higher than wide) or both (diagonal), colour +0x566 / 2, then a second quad between the
 * offsets [0x1012b94c] and [0x1012b950], colour +0x566. Returns 0 for a null object, else 1.
 * Deviation (DECISIONS.md round 22): z (+8) of the four vertices, never written by the original, is 0.
 */
int __cdecl Sim_100918e6(void *obj);
int __cdecl Sim_1009221d(void *obj);

#endif
