/*
 * Round 6, letter f, functions at 0x1008xxxx: the scene-cell lists (moving objects between the cells of the
 * 9 x 9 grid at 0x1017ba28 and freeing them), the screen size of a display driver, small rectangle helpers
 * and the camera override that 0x1008fb20 installs and 0x1008fd5a takes back.
 * Callees that are not converted (or are patched by another hook file) are called through their original
 * address with SIM_FN.
 */
#ifndef HG_SIM_UNSORTED_1008_W6F_H
#define HG_SIM_UNSORTED_1008_W6F_H

#include <stdint.h>
#include "../simmem.h"
#include "../libm/hg_math.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Not converted: the C library's sqrt (a double on the stack, the result in st(0)) and _strcmpi. */
#define W6F_SQRT hg_sqrt
#define W6F_STRCMPI SIM_FN(int(__cdecl *)(const char *, const char *), 0x100ff1e0)
/* Not converted: a dword of entry `index` of a resource table (offset table at +8, 8 bytes per entry). */
#define W6F_CALL_100ed743 SIM_FN(int32_t(__cdecl *)(void *, int32_t), 0x100ed743)
/* Not converted: fills a rectangle of a bitmap (bitmap, left, top, right, bottom, colour). */
#define W6F_CALL_100e90a3 SIM_FN(int32_t(__cdecl *)(void *, int32_t, int32_t, int32_t, int32_t, uint32_t), 0x100e90a3)
/* Patched by another hook file: always called through the original address. */
#define W6F_CALL_100d2306 SIM_FN(void(__cdecl *)(void *), 0x100d2306)
#define W6F_CALL_100d1f90 SIM_FN(void(__cdecl *)(void *), 0x100d1f90)

/*
 * sqrt(x * x + y * y + z * z) in doubles. Like Sim_10084b57 the original returns the double in st(0) (the
 * float copy it stores in a local is never read), so this returns the double.
 */
double __cdecl Sim_10084b8c(float x, float y, float z);

/*
 * Sets the screen size [0x1017b918] x [0x1017b91c] from the name of the display driver: "VESA480.DLL"
 * 640 x 480, "VESA768.DLL" 1024 x 768, anything else ("MCGA.DLL" included) 320 x 200. The names are compared
 * ignoring case.
 */
void __cdecl Display_SizeFromDriverName(const char *name);

/*
 * For every node of the tree `node` (first child at +4, next sibling at +8) that has an object at +0x8c:
 * takes the object out of its cell and links it into the list of the cell at 0x1017b9b0, and sets its cell
 * row/column (+0x8c/+0x90) to 9999. Does nothing while [0x1017c8c4] is 0.
 */
void __cdecl Sim_10088986(void *node);

/*
 * Moves the object of `node` (+0x8c) into the cell (row, col) of the grid at 0x1017ba28 (0x18c bytes per
 * row, 0x2c per cell) and stores row/col in the object (+0x8c/+0x90), then does the same for the children
 * whose object is not flagged 0x1000 with kind (word +2 & 0xf0) 0x70. Does nothing while [0x1017c8c4] is 0.
 */
void __cdecl Sim_10088b24(void *node, int32_t row, int32_t col);

/*
 * Frees the objects of the cell lists: with [0x1017c8c4] set, every cell of the 9 x 9 grid and the list at
 * 0x1017b9b0; otherwise the list at 0x1017b988 when that cell is the current one ([0x1017c8cc]); then, when it
 * is, the lists at 0x1017b9d8, 0x1017c818 and 0x1017ba00. Each list is emptied from its head (+8) with
 * Sim_1008a16d and Sim_10009369 until a head stays in place.
 */
void __cdecl Sim_1008a5e3(void);

/*
 * Scales `rect` around its centre (Rect_ScaleAroundCenter) into `out` by the size of entry `index` of the
 * table `table` (Sim_100ed743: width in the high word, height in the low word) divided by the rectangle's
 * size, as 16.16 factors. Returns out.
 */
int32_t *__cdecl Sim_1008d2d0(const int32_t *rect, int32_t *out, void *table, int32_t index);

/*
 * Every 90 ticks steps the value [0x1014b9b0] by [0x1017cc14] (that step turns 1 at 0xf0 and changes sign at
 * 0xf7), toggles the inset [0x1017cc0c] between 1 and 2 (starting from its remainder by 2) and fills the
 * rectangle inset..(+0xc, +0x10)-inset of `bitmap` with the value. Returns what the fill returned.
 */
int32_t __cdecl Sim_1008e02b(void *bitmap);

/*
 * The point on the border of a rectangle of half size ((+0xc - +4 + 1) >> 1, (+0x10 - +8 + 1) >> 1) in the
 * direction of `pt` (see Sim_1008e489), written to out. Returns out.
 */
int32_t *__cdecl Sim_1008e266(const int32_t *rect, const int32_t *pt, int32_t *out);

/*
 * Installs a camera override: saves the camera ([0x1014cae0], 0xe0 bytes), the driver block 0x1014cae8
 * (0x68 bytes), [0x1017e650] and [0x1017d14c]; places the camera at pos (x, y, z, 3 angles), selects view
 * preset `preset`, sets the projection numbers at 0x1036f1c0.. from `scale` and `depth`, puts two routines
 * into the driver slots 0x1014cb40/0x1014cb44 and rebuilds the camera.
 */
void __cdecl Sim_1008fb20(const uint32_t *pos, int32_t preset, int32_t scale, int32_t depth);

/* Takes back what Sim_1008fb20 saved and rebuilds the camera (view preset 0). */
void __cdecl Sim_1008fd5a(void);

#endif
