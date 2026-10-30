/*
 * Wave 3S, original 0x10008eac..0x10009c55: a float getter and the transform of
 * a model's vertex blocks by a 3x4 matrix. Plain C, original cdecl convention.
 *
 * The matrix m is 12 floats: three rows of three (m[0..8]) and a translation
 * (m[9..11]). The model block has a 16-bit count of vertices at +4, a 16-bit
 * count of normals at +6, the offset of the normals at +8, and 0x58-byte
 * vertices from +0x18; a 0x28-byte normal has its input vector at +0xc and its
 * output vector at +0x18.
 */
#ifndef HG_SIM_UNSORTED_1000_W3S_H
#define HG_SIM_UNSORTED_1000_W3S_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* The float that the pointer at obj+0x30 points to, or the constant at 0x10111018 when that pointer is NULL. */
float __cdecl Sim_10008eac(const void *obj);
/*
 * Transforms the position (+0, +4, +8) of each vertex of the block into (+0xc, +0x10, +0x14)
 * with rotation and translation, and the direction (+0xc, +0x10, +0x14) of each normal into
 * (+0x18, +0x1c, +0x20) with the rotation only.
 */
void __cdecl Sim_100098bb(void *block, const float *m);
/*
 * Clears bit 9 of the word at obj+0, then transforms the three vectors at +0x38, +0x54 and +0x70 by
 * the matrix: the three components of each vector (rows 0..2, translation m[9..11]) go to +0x44,
 * +0x60 and +0x7c. Adds 1 to the dword at obj+0x9c.
 */
void __cdecl Sim_10009a53(void *obj, const float *m);
/* Sim_10009a53(obj, m), then Sim_100098bb(child, m) for each child of the chain that starts at obj+0x2c (next at +0xc), setting child+0x10 = obj+0x9c. */
void __cdecl Sim_10009c55(void *obj, const float *m);

#endif
