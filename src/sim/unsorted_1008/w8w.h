/*
 * Round 8, letter w: functions in 0x100840dc..0x10089bad (lookup tables of sine and arctangent, the normal of a
 * triangle, a palette fade, the radii of a node tree, the debug grid markers, a small file reader).
 * Floating point follows the original x87 code with the game's control word (53-bit precision): every
 * intermediate is a double and only a store to a float rounds to float. Truncation to int is the runtime's _ftol
 * (Render_Ftol, render/w2n.h). The C library fabs (0x100fcb70) and sqrt (0x100fca04) of the DLL are called
 * through their original address; prototypes of the helpers that are not converted are below.
 */
#ifndef HG_SIM_UNSORTED_1008_W8W_H
#define HG_SIM_UNSORTED_1008_W8W_H

#include <stdint.h>
#include "../simmem.h"
#include "../libm/hg_math.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* fabs and sqrt from OpenLibm (hg_fabs, hg_sqrt), in place of the DLL's C library. */
#define W8W8_FABS hg_fabs
#define W8W8_SQRT hg_sqrt
/* The C library fread (0x100fe830) and fclose (0x100fd760) of the DLL. */
#define W8W8_FREAD SIM_FN(uint32_t(__cdecl *)(void *, uint32_t, uint32_t, void *), 0x100fe830)
#define W8W8_FCLOSE SIM_FN(int(__cdecl *)(void *), 0x100fd760)
/* The node-tree walks (Sim_10089066 / Sim_1008950e, round 19), called through their addresses: they fill three out
 * values (floats) for the node. */
#define W8W8_CALL_10089066 SIM_FN(int(__cdecl *)(void *, float *, float *, float *), 0x10089066)
#define W8W8_CALL_1008950e SIM_FN(int(__cdecl *)(void *, float *, float *, float *), 0x1008950e)

/*
 * Table sine: the angle a (float) is reduced with the floats at 0x1012b870..0x1012b888 and looked up in the
 * float table at 0x103719a0 (index truncated with _ftol). |a| below [0x1012b870] (or NaN) gives the float at
 * 0x1012b874. Returned in st(0).
 */
float __cdecl Sim_10084766(float a);

/*
 * Table arctangent of (x, y) in the units of the constants at 0x1012b870..0x1012b894 (45.0 for |x| == |y|),
 * looked up in the float table at 0x103721c0; the special cases for a near-zero x or y return the floats at
 * 0x1012b88c, 0x1012b87c, 0x1012b874, 0x1012b890 and 0x1012b880. Returned in st(0).
 */
float __cdecl Sim_1008486e(float x, float y);

/*
 * Normal of the triangle (a, b, c), each point given as three floats: *nx, *ny, *nz = the cross product
 * -((b - a) x (c - b)) divided by its length; returns the length (as a float) truncated with _ftol.
 */
int32_t __cdecl Sim_10084bcf(float ax, float ay, float az, float bx, float by, float bz, float cx, float cy, float cz,
                             float *nx, float *ny, float *nz);

/*
 * Fades from the palette at 0x1015e6a0 (copied to 0x109b8c00) to the 256 RGB triples of `target` in steps / 2
 * steps (steps arithmetically shifted right by one): each step writes current + truncated (k * delta) into
 * `target` and calls Gdi_SetPalette(0, 256, target) and Sim_10083c45. Then [[0x1015e698]] = [0x1015e9a0].
 * Returns -1 for a null target, else 0.
 */
int32_t __cdecl Sim_100840dc(uint8_t *target, int32_t steps);

/*
 * Reads the file `name` ("rb"): 0x66 bytes into rec; when the dword rec+4 is non-zero, a heap block of
 * rec+4 * 0x1e bytes (HeapAlloc of heap [0x1017b8f8], flags 1) goes to rec+0x10 and rec+4 records of 0x1e bytes
 * are read into it. Returns 1 when that second part was read, else 0.
 */
int32_t __cdecl Sim_10086e27(const char *name, uint8_t *rec);

/*
 * Radius of the node tree under obj (stored at obj+0x88 and returned in st(0)): the float at 0x1012b8ec for a
 * null obj; without children (block +0x28 or its first child null) +0x88 = +0x50. Otherwise the largest of
 * (distance of +0x44.. from the centre +0x7c.. plus +0x50, or 0 when the word +0 has 0x1000) and, for every child
 * node whose object (+0x8c) has a radius above the float at 0x1012b8f0, its distance plus its radius. The
 * radius is scaled by the float at 0x1012b8f4 when the byte +0xa0 is 'W'. Word +0 bit 0x1000 makes the leaf
 * case return the float at 0x1012b8ec.
 */
float __cdecl Sim_10088e37(uint8_t *obj);

/* As Sim_10088e37 for the second radius: centre +0x60.., result +0x6c, flag 0x800, no 'W' scaling. */
float __cdecl Sim_10089325(uint8_t *obj);

/*
 * For a non-null obj: unless bit 2 of obj+0x94 is set, 0x10089066(obj, out values) and Sim_10088e37(obj), then
 * bit 2 is set; unless bit 1 is set, 0x1008950e(obj, out values) and Sim_10089325(obj), then bit 1 is set.
 */
void __cdecl Sim_100897c7(uint8_t *obj);

/*
 * When [0x1017c8c4] and [0x1017c8c0] are set: draws Sim_10089907 markers over the 9 x 9 grid around the camera
 * ([0x1014cae0]), at the ground height (Sim_1002d961) or at the position of the cell record at 0x1017ba50, then
 * for the cell under the camera and at the camera itself.
 */
void __cdecl Sim_10089bad(void);

#endif
