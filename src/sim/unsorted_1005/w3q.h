/*
 * Round 3 (letter q) functions of HGSIM.DLL around 0x10052000..0x10060000.
 * Names carry the original address because the code alone does not say what
 * the fields mean. Floating point is done as the original's x87 did it
 * (see util/w3q.h).
 */
#ifndef HG_SIM_UNSORTED_1005_W3Q_H
#define HG_SIM_UNSORTED_1005_W3Q_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Returns [0x10185bf4] - Render_Ftol(-((x - [0x10185bd4]) * f(p+0x58) / [0x10185be4] + f(p+0x70)) *
 * [0x10185be8] / f(p+0x64)), the dword at 0x10185bd4 being an integer and the rest floats.
 */
int __cdecl Sim_10052edc(int x, const void *p);
/*
 * Polygon record (word +2 = vertex count, dword +4 = offset of the byte list of vertex numbers,
 * see unsorted_1002/w3q.h) against the cube around a point: the floats at box+0x14, +0x18, +0x1c
 * are the centre and the float at +0x3c the half-size. Returns 1 as soon as one vertex (0x58-byte
 * vertices, x/y/z at +0xc/+0x10/+0x14) lies inside the cube (not above centre + size and
 * not below centre - size, per axis), else 0 (also for a null poly or verts).
 */
int __cdecl Sim_1005897b(const void *poly, const void *verts, const void *box);
/*
 * Multiplies the vector (*a, *b, *c) by the 3x3 matrix of nine floats at m (rows of three:
 * *a = m0 * x + m1 * y + m2 * z, *b = m3 * x + .., *c = m6 * x + ..) and stores the result back
 * through the three pointers (the products are summed in double and rounded to float on the store).
 */
void __cdecl Sim_1005fc4f(const float *m, float *a, float *b, float *c);

#endif
