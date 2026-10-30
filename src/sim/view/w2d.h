/*
 * Wave 2 additions to the view area: functions that call the converted
 * Rect_Interpolate, Mat3_Transpose and ObjList_Unlink. Same conventions as
 * view.h (plain C, original cdecl calling convention).
 */
#ifndef HG_SIM_VIEW_W2D_H
#define HG_SIM_VIEW_W2D_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Unlinks a node from its list (ObjList_Unlink), sets bit 0x800 of its flag
 * word and, when the dword 0x1017c8cc is non-zero, also unlinks it from the
 * second list made of the links at +0xc and +0x10. Does nothing for a null
 * node or one that already has bit 0x800.
 */
void __cdecl Sim_1008a37f(void *node);
/*
 * pair holds two pointers: a state block N and an animation block O.
 * Only when N+0 is 1 does it work: out = O+0xc is the rect it writes (also the
 * return value, 0 otherwise). With flip 0 the rects at O+4 / O+8 are the start
 * and end, with flip non-zero they are swapped. When N+4 is 0 the start rect is
 * copied and N+8 reset; otherwise N+8 grows by the dword at 0x1015dc80 and
 * t = (N+8 << 16) / O+0, signed, truncated toward zero. For t >= 0x10000 the end
 * rect is copied, N+8 reset and N+0 becomes 0, else out = Rect_Interpolate(start, end, t).
 * Always ends with N+4 = old N+0 and N+0 = the new state. The original raises a
 * divide error for O+0 == 0 or a quotient outside 32 bits; this version does not.
 */
int32_t *__cdecl Sim_1008f747(int flip, const uint32_t *pair);
/* dst+0x54 = transpose of the 3x3 of src (Mat3_Transpose); dst+0x78/+0x7c/+0x80 = src+0x24/+0x28/+0x2c. */
void __cdecl Sim_100d25eb(void *dst, const uint32_t *src);
/* The reverse: dst2's 3x3 = transpose of the one at src+0x54; dst2+0x24/+0x28/+0x2c = src+0x78/+0x7c/+0x80. */
void __cdecl Sim_100d262a(const void *src, uint32_t *dst2);

#endif
