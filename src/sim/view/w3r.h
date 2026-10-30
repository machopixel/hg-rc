/*
 * Round 3 (letter r): 3x4 matrix maths and the camera viewport set-up/visibility
 * test. Floats are stored as float but every sum and product was evaluated in
 * double by the x87 code, and is here (see util/w3r.h).
 *
 * A "3x4 matrix" is 12 floats: the 3x3 part in row order (m[0..8]; row i is
 * m[3i..3i+2]), then the translation m[9..11]. A point p maps to
 * (row0 . p + m[9], row1 . p + m[10], row2 . p + m[11]).
 */
#ifndef HG_SIM_VIEW_W3R_H
#define HG_SIM_VIEW_W3R_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Transforms the point (*x, *y, *z) in place by the 3x4 matrix m. */
void __cdecl Mat3x4_TransformPoint(const float *m, float *x, float *y, float *z);
/*
 * Replaces column `axis` (0, 1 or 2: elements axis, axis+3, axis+6) of the 3x3 part by the
 * cross product of the other two columns taken in cyclic order (axis+1 x axis+2). Does
 * nothing for any other axis.
 */
void __cdecl Mat3_CrossColumn(float *m, int axis);
/* out = a * b for the 3x3 parts (row-major, out[3i+j] = sum a[3i+k] * b[3k+j]); the translation is not touched. */
void __cdecl Mat3_Multiply(const float *a, const float *b, float *out);
/* out = a applied after b: 3x3 part a*b, translation a(b's translation). Copies the 12 floats to out at the end. */
void __cdecl Mat3x4_Multiply(const float *a, const float *b, float *out);
/* out = inverse of m, valid when its 3x3 part is a rotation: the 3x3 part is transposed, the translation is -R^T t. */
void __cdecl Mat3x4_InvertRigid(const float *m, float *out);
/* Transforms the three floats at p by the matrix of record [0x101507f8] of the 0x7c-byte table at 0x101a11c8, unless that index is -1. */
void __cdecl Sim_100b403d(float *p);

/*
 * Fills the derived fields of a camera record from its viewport rectangle
 * (+0x2c left, +0x30 right, +0x34 top, +0x38 bottom): centre +0x8c/+0x90, half
 * size +0x84/+0x88 (at least 1), and the scale fields +0xa8, +0xac, +0xb0, +0xb8,
 * +0x3c, +0x94, +0x98 from the field-of-view float at +0x18 (limited to 0.5..16.0),
 * the aspect float at +0x44 and the trigonometry-style tables at 0x1036f860 and
 * 0x10370d20.
 * NOTE: src/hooks/hooks_view.c patches this address (CameraViewport); this C is
 * verified but is not in a conv list.
 */
void __cdecl Sim_100d2306(uint8_t *cam);
/*
 * Visibility test of a sphere (centre +0x44,+0x48,+0x4c, radius +0x50) against the
 * current camera [0x1014cae0] and the view-plane values at 0x10185c20..0x10185c50.
 * Returns 0 when it is visible, 1 for a record whose word at +2 has (w & 0xf0) == 0xa0
 * while [0x1015ef0c] is set, and 4, 5, 6 or 7 for the first test it fails.
 * Stores the distance along the view axis in the float at 0x101c7aa0.
 */
int __cdecl Sim_100d280d(const uint8_t *obj);

#endif
