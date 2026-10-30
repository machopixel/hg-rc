/*
 * Round 3 (letter q) functions of the renderer-facing glue: registering a
 * texture file under a name, drawing flat polygons through the display
 * driver callback at 0x1014cb4c, and taking a projected vertex from the pool.
 * Names carry the original address where the code alone does not say what
 * the fields mean. See also w2m.h and w2n.h.
 */
#ifndef HG_SIM_RENDER_W3Q_H
#define HG_SIM_RENDER_W3Q_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Enters the cached file `name` into the name table (groups of 32 records of 0x18 bytes at
 * 0x10a51ac0, see Render_GetTextureImage): `group` is -1 for "the current group" (dword
 * 0x1015cfe0, first advanced when 0x1015cfe4 is set, which clears it). The name is copied
 * (0x20 bytes at most are used for the work copy), with "WX"/"wx" changed to "WG" when dword
 * 0x1014cb38 is set; the file is loaded through the cache loader (name, [0x1014ceb8], 0) unless
 * [0x1015ec58] is set (then the image pointer stored is NULL). When [0x1017b954] is set, the
 * image's two 16-bit words (width, height) are multiplied and added to [0x1015cff4]. The record
 * goes in the first free slot of the group; sets [0x1015cfe8] to 1 when the group already had a
 * name in slot 0, else 0; stores the image pointer at +0x14 and `value` (16 bits) at +0x12.
 * Returns 0, or -1 for a null name, a group outside 0..0x1ff or a full group. When
 * [0x1012da30] is -2 the tables are first reset (Sim_10007020).
 */
int __cdecl Sim_10006ba7(const char *name, int group, int value);

/*
 * Draw a quadrilateral / a triangle of flat colour through the display driver callback at
 * 0x1014cb4c: a vertex array of 36-byte plugin vertices (x, y as floats converted from the
 * integer arguments, the colour argument at +0x10, 0xff000000 at +0x18; the other fields are
 * not set by the original and are zero here) and the call callback(4 or 3, vertices, color).
 * Both return the callback's result.
 */
int __cdecl Sim_100a291d(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3, uint32_t color);
int __cdecl Sim_100a29da(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color);

/*
 * Takes the next vertex from the projected-vertex pool (Sim_100a88ff) and fills it from the
 * object: obj+0x2c is set to it, vertex +0/+4/+8 = obj+0x20/+0x24/+0x28, +0xc/+0x10/+0x14 =
 * obj+0/+4/+8, +0x20/+0x24 = the two dwords at `screen` (or obj+0x18/+0x1c when screen is
 * NULL), +0x30/+0x34/+0x38 = obj+0x40/+0x44/+0x48, +0x2c = obj+0x10. With a NULL `screen` an
 * existing vertex (obj+0x2c non-zero) is returned unchanged. Returns the vertex.
 * (The original also computes three float differences from the camera position at 0x10185c48
 * and never uses them.)
 */
void *__cdecl Sim_100a5c10(void *obj, const uint32_t *screen);
/* Sim_100a5c10(obj, NULL). */
void *__cdecl Sim_100a5d25(void *obj);

#endif
