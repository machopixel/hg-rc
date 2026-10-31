/*
 * Round 32: 1.2's particle effects (rounds C-F of the 1.2 port; DECISIONS.md, "Round 32"). Every function here is a
 * 1.2 function with no 1.0 counterpart, named Sim12_<its 1.2 address>; every 1.2 global it adds lives here as
 * g_w32_<its 1.2 address> with 1.2's layout (the checker maps each to 1.2's address one to one).
 *
 * Particles exist only in 1.2's 3D configuration: here with the Vulkan renderer (g_w31Tex) in 3D mode, as the
 * .TEX textures (round 31); option 3 and noconv stay 1.0.
 */
#ifndef HG_SIM_FX_W32_H
#define HG_SIM_FX_W32_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl
#endif

#include "../render/w31.h"

/*
 * The effect pool (1.0 0x10217c20, 1000 records): 1.2 keeps it but holds particle systems in it, records of 0x406
 * bytes (1.0: 0x57e) with the engine node / update / draw / argument dwords at +0x332 / +0x336 / +0x33a / +0x366
 * (1.0: +0x552 / +0x556 / +0x55a / +0x57a). 1.2's records fit in 1.0's area, so the pool keeps its addresses; the
 * layout follows the configuration: 1.2's with the Vulkan renderer in 3D mode (as the .TEX textures), else 1.0's.
 */
#define W32_PARTICLES() W31_TexMode()
#define W32_REC_SIZE (W32_PARTICLES() ? 0x406u : 0x57eu)
#define W32_REC_OBJ (W32_PARTICLES() ? 0x332u : 0x552u)
#define W32_REC_UPDATE (W32_PARTICLES() ? 0x336u : 0x556u)
#define W32_REC_DRAW (W32_PARTICLES() ? 0x33au : 0x55au)
#define W32_REC_ARG (W32_PARTICLES() ? 0x366u : 0x57au)
/* 1.2 0x1014b58c: set by the draw dispatcher, (Sim_100448ff() == 1) */
extern int32_t g_w32_1014b58c;

/*
 * 1.0 0x10090d15 / 1.2 0x10079cec: the effects switch: -1 returns it ([0x1014b9b8]); 0 with effects in the pool
 * first takes every active record out (bit 0 of +0x10 cleared, its engine node released with Sim_100aa351, the
 * record freed with Sim_10090513) and clears the active count; then stores value in [0x1014b9b8] and prints it
 * into 0x10217b40 with the format at 0x1014b9bc. Returns value.
 */
int32_t __cdecl Sim_10090d15(int32_t value);

/*
 * A particle type (1.2 0x1011e700, 22 of 0x1e bytes): +0 the 14-byte texture record it animates (-1 none),
 * +4 its number of textures, +8 red, +9 green, +0xa blue (PartSpec.ini, 0..255), +0xb unused, +0xc intensity,
 * +0xd size, +0xe its PartSpec.ini section name, +0x12 lock, +0x16 order, +0x1a loop.
 */
#define W32_TYPES 0x16
#define W32_TYPE_SIZE 0x1e
#pragma pack(push, 1)
typedef struct W32Type {
    int32_t group;
    uint32_t count;
    uint8_t red, green, blue, unused, intensity, size;
    const char *section;
    int32_t lock, order;
    uint32_t loop;
} W32Type;
#pragma pack(pop)
extern W32Type g_w32_1011e700[W32_TYPES];
#define W32_TYPE(i) ((uint8_t *)&g_w32_1011e700[i])

/* 1.2 0x1014b5a0: an empty text (the settings reader's default). */
extern char g_w32_1014b5a0[4];

/*
 * 1.2 0x10078e34: the settings of particle type `type` from <current folder>\PartSpec.ini: Red, Green, Blue,
 * Intensity, Size (bytes), Lock, Order, Loop (dwords), each with its current value as default, then the names
 * Texture00.. (max at most 0x20; each its first 8 characters into names + 9 * i). Returns the number of names
 * before the first empty one.
 */
int32_t __cdecl Sim12_10078e34(uint8_t *type, char *names, int32_t max);
/*
 * 1.2 0x100790be: the particle types' textures. Without the 3D mode every type is reset (group -1, no textures,
 * colour / intensity 0xf, size 1). Otherwise each type without a texture record gets the next free one of
 * 0x100..0x1ff (word +8 = -2), its PartSpec.ini textures registered (Sim_10006ba7, current group) and the record
 * bound to the group (Sim_10006db8); a type with none is reset.
 */
void __cdecl Sim12_100790be(void);

#ifdef __GNUC__
typedef float __attribute__((may_alias)) w32fa;
#else
typedef float w32fa;
#endif
#define W32_F(p, off) (*(w32fa *)((uint8_t *)(p) + (off)))

/*
 * Round 32: 1.2's particle squares carry no depth (their corners keep z 1 and 1/z 1 from the template); 1.2's
 * renderer had no depth buffer and drew in the game's order. Around each square the sim sets this state of our own
 * (renderer SetState, slot 0x109e3d40) so the Vulkan renderer draws it the same way: no depth test or write.
 */
#define W32_STATE_PAINTER 0x48470001u
#define W32_PAINTER(on) ((int(__cdecl *)(uint32_t, uint32_t))(uintptr_t)G32(0x109e3d40))(W32_STATE_PAINTER, (on))

/* ---- what the families share (w32_family.c) ---- */
/* 1.2 0x1011e9d0: 24 size factors (float bits) picked at random per particle. */
extern const uint32_t g_w32_1011e9d0[24];
/*
 * A new particle system (1.2's spawn prologue): none when the active count + 1 reaches the created count; else the
 * pool record (Sim_10090c7d(update, draw, arg, force)) with +0x10 = 1, the type at +0x402, every particle's frame
 * +0x30 = -1, +0x3fe = 0 and the start frame +0x3fa by the type's Order (1: 0, 2: random, else -1; -1 for a type
 * without textures).
 */
uint8_t *W32_NewSystem(uint32_t update, uint32_t draw, uint32_t arg, int force, int32_t type);
/*
 * The polygon flags for particle i (1.2's draw callbacks): `plain` for a type without a texture record; else its
 * group | 0x7000 after advancing the frame (Lock keeps the particle's own frame; else the system's +0x3fa steps
 * and at the end wraps (Loop 1), stays (Loop 2) or stops the system (bit 1 of +0x10, returns 0)); a frame still
 * -1 is random; it is stored in the particle's +0x30.
 */
uint32_t W32_ParticleFlags(uint8_t *rec, int32_t i, uint32_t plain);
/*
 * A particle's size on screen (1.2's draw callbacks): in 3D mode size * the type's Size * a random factor (at least
 * 1), times the system's scale +0x3fe when 1.2's flag g_w32_1014b58c is set and the scale is not 0, else times the
 * camera's +0x18, truncated; below 1 it is half the type's Size.
 */
int32_t W32_ParticleSize(uint8_t *rec, int32_t size);
/* The 3D part of W32_ParticleSize alone (in 2D mode `size` unchanged). */
int32_t W32_ParticleScale(uint8_t *rec, int32_t size);
/* The four corners' template at +0x36a (1.2's spawns): the type's colour and intensity, fog byte 0xff, texture
   coordinates (0,1) (1,1) (1,0) (0,0), z and 1/z 1. */
void W32_SetTemplate(uint8_t *rec);

/* 1.2 0x1011e998: the corners' texture coordinates (8 pairs u, v as float bits). */
extern const uint32_t g_w32_1011e998[16];
/*
 * 1.2 0x10078be0: qsort comparison of two screen points (16 bytes: x, y, depth, visible): 0 when either is not
 * visible, else 1 when b's depth is greater than a's, -1 otherwise (unordered included).
 */
int __cdecl Sim12_10078be0(const void *a, const void *b);
/*
 * 1.2 0x10078c29: draws a particle system's `count` particles as squares. points: count screen points (16 bytes:
 * x, y, depth, visible dword), sorted far to near first (the C library qsort, Sim12_10078be0) when flags has a
 * texture record (flags & 0x1ff). For each visible one: the four corners at rec + 0x36a (0x24 bytes each) at x -+
 * half and y +- half, half = the particle's size (rec + i * 0x32 + 0x38, int) / 2, their texture coordinates from
 * g_w32_1011e998 starting at a random corner (Sim_1009c929(4)); with flags > 0x1ff the record's frame set to the
 * particle's (+0x30 word, Sim_1000716a); then the polygon routine [0x1014cb4c](4, corners, flags).
 */
void __cdecl Sim12_10078c29(uint8_t *rec, uint8_t *points, uint32_t flags, int32_t count);

/* ---- E: the tracer explosion (type 12; w32_tracer.c) ---- */
/* 1.2 0x10083dde (the system's update): for 22 frames (and until stopped) moves the node by the velocity +0x34e,
   colour +0x346 = 0x1f, returns 1; then releases the node, clears bits 0..2 of +0x10, returns 0. */
int __cdecl Sim12_10083dde(uint8_t *rec);
/* 1.2 0x10083e8a (its draw): the 8 particles at the node + their offsets, projected; the visible ones drawn as
   squares (Sim12_10078c29), a pixel each in 2D mode. Returns 0 when the frames ran out, else 1. */
int __cdecl Sim12_10083e8a(uint8_t *rec);
/* 1.2 0x10084362: a tracer explosion at (x, y, z) moving by (dx, dy, dz) / 8 per frame, if it is within the
   camera's reach (+0x40), on screen and in line of sight; its scale is [0x1015eef4] / distance / 4. */
void __cdecl Sim12_10084362(float x, float y, float z, float dx, float dy, float dz);

/*
 * What 1.2's explosion spawns share (w32_explode.c): nothing beyond the camera's reach (+0x40), off screen or out of
 * line of sight; else a system of `type` (W32_NewSystem(update, draw, 0, 0, type)) with scale [0x1015eef4] /
 * distance / 4, the corner template, the node at (x, y, z) with speed |d * 100| (speed100) or |d|, velocity d / vdiv
 * (+0x34e), the particles' step d / sdiv in step[3]; the active count raised, the start frame +0x33e noted. Returns
 * the record or 0.
 */
/* The distance of (x, y, z) from the camera when it is within reach, on screen and in line of sight, else 0. */
float W32_InSight(float x, float y, float z);
uint8_t *W32_Explosion(float x, float y, float z, float dx, float dy, float dz, int32_t type, uint32_t update,
                       uint32_t draw, int speed100, double vdiv, double sdiv, float *step);
/* ---- E: the missile explosion (type 10; replaces 1.0's Sim_10095e0a) ---- */
/* 1.2 0x10081afb (update): for 110 frames moves the node by the velocity, y plus a random 0..9, colour from the fade
   table 0x101d09c0 by age; then releases the node, returns 0. */
int __cdecl Sim12_10081afb(uint8_t *rec);
/* 1.2 0x10081bdc (draw): as the tracer's without the 2D pixels; a node nearer than projected depth 20000 also
   projects each particle one tenth of the velocity on (the result is not used). */
int __cdecl Sim12_10081bdc(uint8_t *rec);
/* 1.2 0x10082556: the missile explosion, its 8 particles along the velocity with random offsets (x, z -5..4, y
   -25..24). */
void __cdecl Sim12_10082556(float x, float y, float z, float dx, float dy, float dz);

/* ---- E: the grenade explosion (type 8; replaces 1.0's Sim_10094fcb) ---- */
/* 1.2 0x1007ffb3 (update): for 362 frames as the missile's (fade table 0x10215af0) and every particle jittered by
   -30..29 on each axis. */
int __cdecl Sim12_1007ffb3(uint8_t *rec);
/* 1.2 0x1008011d (draw): the missile's far way. */
int __cdecl Sim12_1008011d(uint8_t *rec);
/* 1.2 0x100805dc: the grenade explosion, its 8 particles along the velocity with offsets -25..24. */
void __cdecl Sim12_100805dc(float x, float y, float z, float dx, float dy, float dz);

/* ---- E: the grenade's bounce dust (types 17..20 by the ground kind; new in 1.2, from its grenade shot) ---- */
/* 1.2 0x10084a87 (update): for 90 frames moves the node (y plus a random 0..9), colour and size from the tables
   0x10215bc0 / 0x10215bb0 by age. */
int __cdecl Sim12_10084a87(uint8_t *rec);
/* 1.2 0x10084b83 (draw): every particle moved by its own velocity (+0x1e), drawn with size +0x34a * 2000 / depth (scaled
   in 3D mode; below 1 a pixel in 2D mode, half the type's Size in 3D); none below size 1. */
int __cdecl Sim12_10084b83(uint8_t *rec);
/* 1.2 0x1008514d: in 3D mode, dust at (x, y, z) when in sight: type by the ground kind (0xf..0x1b: 20, 0x25 / 0x2d:
   19, 0x21..0x23 / 0x28: 18, else 17), drifting back (-d / 5, y -10), 8 particles near the centre with random
   velocities around it. */
void __cdecl Sim12_1008514d(float x, float y, float z, float dx, float dy, float dz, uint32_t unused, int32_t kind);

/* ---- the smoke (type 7; replaces 1.0's Sim_100949e8) and gun (type 6) families, one update and draw ---- */
/* 1.2 0x1007e99d (update): for 543 frames moves the node (y plus 0..9), colour / size from 0x10215bd0 /
   0x10217b20 by age, every particle jittered by -10..9 on each axis. */
int __cdecl Sim12_1007e99d(uint8_t *rec);
/* 1.2 0x1007eb46 (draw): size +0x34a * 2000 / depth; in 2D mode a pixel, or pixel pairs over the size; in 3D the
   first corner coloured from the colour table by the flags (past its 256 entries 1.2 reads its scene-object table;
   the same bytes are read from ours). */
int __cdecl Sim12_1007eb46(uint8_t *rec);
/* 1.2 0x1007f8a9 (smoke) / 0x1007f19f (gun): the system along the velocity (node d / 256, particles d / 8). */
void __cdecl Sim12_1007f8a9(float x, float y, float z, float dx, float dy, float dz);
void __cdecl Sim12_1007f19f(float x, float y, float z, float dx, float dy, float dz);

/* ---- the skate dust (type 5; replaces 1.0's Sim_100941c3) ---- */
/* 1.2 0x1007dbe7 (update): for 181 frames moves the node (y plus 0..9), size from 0x10217b20 by age, each
   particle's frame from its row (unless the system is locked) and jittered (x, z -50..49, y -10..9). */
int __cdecl Sim12_1007dbe7(uint8_t *rec);
/* 1.2 0x1007dd89 (draw): as the dust's, without moving the particles; a type without textures draws with the
   particle's frame as its flags. */
int __cdecl Sim12_1007dd89(uint8_t *rec);
/* 1.2 0x1007e25c: the skate dust along the velocity (node d / 256, particles d / 8), each particle on one of the two
   frame rows at 0x10215b20. */
void __cdecl Sim12_1007e25c(float x, float y, float z, float dx, float dy, float dz);

/* ---- the shockwave (type 4; replaces 1.0's Sim_10092f9d) ---- */
/* 1.2 0x1007bb48 (update): until the end frame +0x342 moves the node (y plus 0..9), size from 0x10215bb0 by age,
   colour 0xfe, every particle by its own velocity. */
int __cdecl Sim12_1007bb48(uint8_t *rec);
/* 1.2 0x1007bca5 (draw): as the skate's, with the system's colour as the flags of a type without textures. */
int __cdecl Sim12_1007bca5(uint8_t *rec);
/* 1.2 0x1007c1a1: for strength k > 0, a ring at (x, y, z) when in sight: lasts k * 181 frames, 8 particles near the
   centre flying out horizontally at a random angle with speed k * 80 + 0..k * 80 - 1. */
void __cdecl Sim12_1007c1a1(float x, float y, float z, float dx, float dy, float dz, int32_t strength);

/* ---- the sparks / puffs (types 13..16; replace 1.0's Sim_100939a3, with the shot's ground kind) ---- */
/* 1.2 0x1007c996 / 0x1007ca8b (updates in 2D mode, kinds 0 / 1): the dust's, colour from 0x1036f050 / 0x10215b18 by
   age, size 1. 0x1007cb80 (kind 2) and 0x1007cc7c (3D mode) are the dust's update, 0x1007cd78 its draw (byte for byte
   the same code in 1.2). */
int __cdecl Sim12_1007c996(uint8_t *rec);
int __cdecl Sim12_1007ca8b(uint8_t *rec);
int __cdecl Sim12_1007cb80(uint8_t *rec);
int __cdecl Sim12_1007cc7c(uint8_t *rec);
int __cdecl Sim12_1007cd78(uint8_t *rec);
/* 1.2 0x1007d342: sparks at (x, y, z) when in sight: in 2D mode by `kind` (0..2; none for others), in 3D the type by
   the ground kind (0xf..0x1b: 14, 0x25 / 0x2d: 16, 0x21..0x23 / 0x28: 15, else 13); then as the dust. */
void __cdecl Sim12_1007d342(float x, float y, float z, float dx, float dy, float dz, int32_t kind, int32_t ground);

/* ---- F: the railgun (type 0), accelerator (2) and laser (3) beams (w32_beam.c) ---- */
/* 1.2 0x10086750: the four corners' texture coordinates from a random start (as the particle draw's). */
void __cdecl Sim12_10086750(uint8_t *v);
/* 1.2 0x10086810: a quad: the texture coordinates, the corners (x0, y0) .. (x3, y3), the frame (flags > 0x1ff), the
   polygon routine. */
void __cdecl Sim12_10086810(uint8_t *v, float x3, float y3, float x2, float y2, float x1, float y1, float x0, float y0,
                            uint32_t unused, uint32_t frame, uint32_t flags);
/* 1.2 0x10079f7d / 0x1007a092: the railgun's update (follows the shot record +0x366 for +0x342 frames, then the
   impact 0x100246b3) and draw (nothing). */
int __cdecl Sim12_10079f7d(uint8_t *rec);
int __cdecl Sim12_1007a092(uint8_t *rec);
/* 1.2 0x1007a26c / 0x1007aec2: the accelerator's and laser's updates (copy the shot record's two ends, the node at
   the first / second; the laser stops its sound at the end). */
int __cdecl Sim12_1007a26c(uint8_t *rec);
int __cdecl Sim12_1007aec2(uint8_t *rec);
/* 1.2 0x1007a379 (= 0x1007afdc): the beam between the two ends, projected: two quads (5 wide, then 2..3), across
   or up and down by the larger extent. */
int __cdecl Sim12_1007a379(uint8_t *rec);
int __cdecl Sim12_1007afdc(uint8_t *rec);
/* 1.2 0x1007a0a6 / 0x1007acf3 / 0x1007b956: the railgun, accelerator and laser systems (the laser also starts its
   sound "AFXWLLLC"). */
void __cdecl Sim12_1007a0a6(uint32_t arg, int unused, uint32_t param);
void __cdecl Sim12_1007acf3(uint32_t arg, int unused, uint32_t param, uint32_t param2);
void __cdecl Sim12_1007b956(uint32_t arg, const uint8_t *shot, uint32_t param);

#endif
