/*
 * Round 10: game functions of HGSIM.DLL around 0x10030000..0x1003ffff, converted one at a time.
 * Calls of functions that are not converted yet go through their original address (SIM_FN).
 */
#ifndef HG_SIM_UNSORTED_1003_W10_H
#define HG_SIM_UNSORTED_1003_W10_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Segment seg (unsorted_1009/w3o.h) against the box (six floats: min/max x, y, z): slab by slab with
 * Sim_1009cfd4 (start and unit direction of the segment), keeping the latest entry and the earliest exit. 0 when
 * box is 0, a slab misses, the entry comes after the exit or after the segment's length (Sim_1009cbde);
 * otherwise cuts the segment at the entry (Sim_1009cd36) and returns 1. The first argument is not used. */
int __cdecl Sim_10034111(uint32_t unused, void *seg, const float *box);

/* Debug overlay: marks the objects of list (from +8, next at +8) on the screen. The name filter 0x1015e508 is
 * read from object.dat ("%s") the first time and when [0x10179cc0] is set (default "TRHP"). Objects of kind 0x50
 * (word +2 & 0xf0), or with bit 0x100 of +2 and without bit 0x800 of +0, that the filter function [0x1014cb40]
 * accepts (0) are projected (Sim_100d2669; with [0x1015e550] set, only those whose name +0xa4 or whose leader's
 * name matches, at the position +0x7c.. instead of +0x44..), boxed (0x100eb6e8, colour 15 for a name match, else
 * 7) and labelled with their name (font [0x1015e554], loaded once). */
void __cdecl Sim_100357db(uint8_t *list);

/* Quadtree cell q (bit 1: upper x half, bit 2: upper z half) of the box (+0/+4 x, +0x10/+0x14 z) over the faces of
 * mesh (word +6 faces of 0x28 bytes at mesh + dword +8): 0 when the cell is empty. The faces Sim_1003196f finds in the
 * cell are counted; up to 100 they go into a new leaf (Sim_1003184b(x, z bounds, y range, count) + their pointers at
 * +0x2c); above 100 the cell becomes a node whose four children (+0x1c..) are built recursively and whose y range is
 * theirs (cleared when the minimum is exactly [0x101112f8]). Returns the new record or 0. */
void *__cdecl Sim_10031447(void *box, uint32_t q, void *mesh);

/* Segment seg (unsorted_1009/w3o.h) against the collision grid: the grid's box (+0..+0x14) and 7 x 7 cells of 0x20
 * bytes (+0x24, rows of 0xe0, cell from (min - origin +0x18/+0x20 + 28000) / 8000) are tested against the
 * segment's bounding box; for every face of the matching cells (list +0x1c, face at node+4) a copy of the segment is
 * cut by Sim_1002e790(face, points+0x18, copy) and the shortest result kept. 0 when nothing is hit, otherwise seg
 * becomes the shortest cut and the result is 1. */
int __cdecl Sim_10030c87(const float *grid, uint8_t *points, void *seg);

/* Switches the model of the record index (0x109b9e20 + index * 0x7e) to the level's model (+0x50 + level * 4, name at
 * +8 + level * 0x10, kind word +0x74 + level * 2). Nothing (1) while +0x7c is set; the record pointer when the level is
 * already set. A missing model is loaded (0x100ba7d5, Sim_100e2a31) and attached to the record's node +0x60 (made from the
 * unit's node or the record +0x48's, Sim_100a9830 / Sim_100a9a1a by mode), the name copied to node+0x90; a level of kind
 * 0x50 already present keeps it (1). Then the model is shown (kind 0x70 or not), the previous level's state taken over
 * and hidden, the unit's node refreshed for flag 0x4000 (+0x1c bit 0x800) and +4 = level; 1. 0 when a load fails. */
uint32_t __cdecl Sim_1003aecb(int32_t index, int32_t level, uint32_t mode);

/* Network game start (game, when given, becomes [0x1015d004] and [0x1015ec84] = 1); without a network game only bit 1 of
 * [0x1017b94c] is set. Clears the per-player tables, allocates and clears the 17 score blocks of 0x100 bytes (the first
 * cleared for [0x1015ec8c] = 0x42 bytes; the original clears 0x1015ecb8 again instead of 0x1015ecc0) and 0x1015d008,
 * stopping at the first that fails, then the own player [0x1017b8c8] = Sim_1003db56(game+0x10), the lowest player id
 * 0x1015ecd8 of game+0x16 (count +0x12, 0xfa01 = none) and the host flags 0x1015ecd0 / 0x1015dc84. */
void __cdecl Sim_1003dc19(uint32_t game);

#endif
