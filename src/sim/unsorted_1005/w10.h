/*
 * Round 10: game functions of HGSIM.DLL around 0x10050000..0x1005ffff, converted one at a time.
 * Calls of functions that are not converted yet go through their original address (SIM_FN).
 */
#ifndef HG_SIM_UNSORTED_1005_W10_H
#define HG_SIM_UNSORTED_1005_W10_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Frees (HeapFree on the heap 0x1017b8f8, flags 1) the block at rec+0x131 when it is set, then rec itself. */
void __cdecl Sim_1005b197(void *rec);

/* Shade level 0..15 of poly (with verts) for brightness b and distance d: the light position 0x109b16c4/c8/c0 is
 * set from the camera, moved or taken from 0x10143fc0.. by the mode words, the level [0x101668b4] gains
 * trunc([0x10143fbc] * Sim_100a6414(poly, verts) / 128) when positive, loses trunc(d / [0x1014cb2c]) >> 4 (not
 * below 0) and the light of every lamp of the list [0x101668d0] that Sim_1005897b finds (until 0x80), the light
 * position and 0x109afdcc are put back, and the level becomes level * (b >> 1) / 0x440 clamped to 0..15. */
int32_t __cdecl Sim_100591ff(const void *poly, const void *verts, int32_t b, float d);

/* Rebuilds a model record (0xd4 bytes, on the heap [0x1017b910] when set, else [0x1017b8f8]; the heap word is put
 * back to [0x1017b900] after each allocation) from the saved image buf of size bytes: 0 when buf is 0, the first
 * dword is not 0 or the second marker is not 1 or 2 (both freed then); the 0x88-byte head, its parts (+0x84 of
 * 0x438 bytes at +0x88, with their code +8 mapped by Sim_1004b8b0 into +0x10 and the links +0x54 / +0x78 turned from
 * indexes into pointers), the 0x3c bytes at +0x8c, the +0xb0 records of 0x6c bytes at +0xc8 and, when bytes are
 * left, a sized extra block (+0xcc size, +0xd0 data). */
void *__cdecl Sim_10059a34(const uint8_t *buf, uint32_t size);

/* Lights the [0x101c9ea8] vertices of list (not yet lit: byte +0x29 bit 2) of the polygon poly over verts: base level
 * [0x101668b4], plus the light of Sim_100a648c (scaled by [0x10143fbc]) when lighting is on, less the distance fade
 * ([0x1014cb2c]) or the fog (Sim 0x10058ac2, byte +0x2b); then the light sources of the list [0x101668d0] that touch the
 * polygon (Sim_1005897b) add theirs (all of it when the source sits on the polygon's first corner). The light vector
 * 0x109b16c0.. and [0x109afdcc] are put back at the end; the level +0x3c is scaled (* scale / 1024) and clamped to
 * (4 or 0)..0x1f and the vertex marked lit. */
void __cdecl Sim_10058c5e(const uint8_t *poly, const uint8_t *verts, uint8_t *const *list, int32_t scale);

#endif
