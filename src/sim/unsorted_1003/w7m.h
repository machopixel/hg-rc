/*
 * Round 7, letter m: functions at 0x10031acd..0x1003ca04: the collision-tree queries, the table of collision
 * records at 0x109e4cc0, object state set-up, a renderer plugin probe, the window mode switch and the DirectSound
 * error names. Helpers that are not converted yet are called through their original address (SIM_FN) with the
 * prototypes below; Windows functions through the DLL's import slots (SIM_API).
 */
#ifndef HG_SIM_UNSORTED_1003_W7M_H
#define HG_SIM_UNSORTED_1003_W7M_H

#include <stdint.h>
#include "../simmem.h"
#include "../util/access.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#define __stdcall
#endif

/* Not converted: takes the object (the first dword of the state record). */
#define W7M_CALL_10011093 SIM_FN(void(__cdecl *)(uint32_t), 0x10011093)
#define W7M_CALL_10011681 SIM_FN(void(__cdecl *)(uint32_t), 0x10011681)
/* Not converted: an angle (float) to its sine from the table at 0x103719a0 (st(0)). */
#define W7M_CALL_10084766 SIM_FN(double(__cdecl *)(float), 0x10084766)
/* Not converted: the C library qsort (base, count, width, compare). */
#define W7M_QSORT SIM_FN(void(__cdecl *)(void *, uint32_t, uint32_t, void *), 0x100fdcc0)
/* Not converted: clips the segment seg to the box of six floats (x, y, z minimum / maximum pairs); returns 1 on a
 * hit (the first argument is not used). */
#define W7M_CALL_10034111 SIM_FN(int(__cdecl *)(uint32_t, void *, const void *), 0x10034111)
/* Not converted: takes no arguments. */
#define W7M_CALL_1003be30 SIM_FN(void(__cdecl *)(void), 0x1003be30)
/* Not converted: (state record, -1, 1); called again after a placement when another entry is near. */
#define W7M_CALL_10038913 SIM_FN(int(__cdecl *)(uint32_t *, int32_t, int), 0x10038913)
/* Not converted: (code); returns a dword (0 when there is no entry for the code). */
#define W7M_CALL_100766ca SIM_FN(uint32_t(__cdecl *)(uint32_t), 0x100766ca)

#ifndef HG_SIM_W7M_FLOAT_ACCESS
#define HG_SIM_W7M_FLOAT_ACCESS
/* A float of the original's data at a byte offset (may alias anything, like the U32 family in util/access.h). */
typedef float HG_MAY_ALIAS w7m_f32;
#define W7M_F(p, off) (*(w7m_f32 *)(BYTES(p) + (off)))
#define W7M_GF(addr) (*SIM_PTR(w7m_f32, addr))
static inline float W7m_Float(uint32_t bits)
{
    union {
        uint32_t u;
        float f;
    } v;
    v.u = bits;
    return v.f;
}
static inline uint32_t W7m_Bits(float f)
{
    union {
        float f;
        uint32_t u;
    } v;
    v.f = f;
    return v.u;
}
#endif

/*
 * Collision tree query at (x, y, z). A node has the bounds x min/max at +0/+4, y min/max at +8/+0xc, z min/max at
 * +0x10/+0x14, a polygon count at +0x18 and either four children at +0x1c..+0x28 (count 0) or the polygon
 * pointers from +0x2c. Returns 0 outside the node's x/z bounds (or for NULL), 2 above it or when nothing is hit,
 * 3 below it or when a polygon (with a positive float at +0x1c) that contains (x, z) is found (Sim_1002e9f5)
 * and Sim_1002d8be reports a hit.
 */
int __cdecl Sim_10031acd(const void *node, const void *mesh, float x, float y, float z);
/*
 * Like Sim_10031acd without the y tests: returns 0 outside the x/z bounds, else 1, after Sim_1002d8be wrote the
 * height of the first polygon that contains (x, z) to *out (or 0 when there is none).
 */
int __cdecl Sim_10031f95(const void *node, const void *mesh, float x, float y, float z, float *out);
/*
 * Casts a segment from the centre of obj (+0x20) mirrored at vec to vec, both at the centre's height, against the
 * box at obj+0x2c (0x10034111); on a hit moves the end back by `back` along it and stores its x, ground height
 * (Sim_1002d961) and z in out. Returns 1 on a hit, else 0.
 */
int __cdecl Sim_1003387d(const void *obj, const uint32_t *vec, float back, uint32_t *out);
/* Fills the 0x44-byte collision record rec from obj (kind 1 or 2 from the type bits, bounds via Sim_1002fba5). */
int __cdecl Sim_100339e8(uint32_t *rec, const void *obj);
/* Rebuilds the collision record table at 0x109e4cc0 (count, then records) from the object lists. Returns 1, or 0
 * when a record could not be filled or the table (200 records) is full. */
int __cdecl Sim_10033aeb(void);
/*
 * Collects the records of the table at 0x109e4cc0 that are in use and closer than radius to (x, y, z), sorted by
 * distance (qsort), into the index list at 0x109e3e40; *list = that list. Returns their count (0 when the table
 * is empty or list is NULL).
 */
int __cdecl Sim_10033eb3(float x, float y, float z, float radius, uint32_t *list);
void __cdecl Sim_10036cc0(void *obj);
void __cdecl Sim_10036f25(uint32_t *state);
void __cdecl Sim_1003705a(uint32_t *state);
/*
 * Finds an entry of the list at [0x109e3760] (its successor first when skip is set): with key -1 the first one
 * nearer than 1000.0 to the position of obj, else the first whose +0x124 equals key; then *data = entry + 0xc,
 * *a = entry[2] and *b = entry[0] (*b is 0 when there is none).
 */
void __cdecl Sim_10038dd3(uint32_t *data, uint32_t *a, uint32_t *b, uint32_t *obj, int32_t key, int skip);
/*
 * Takes the "WRF" (else "WBZ") node of the state record's object tree (0 when there is none, when +0x490 is 0 or
 * when id is not -1 and differs from the id +0x118 of the record selected by +0x5fb); with place set the node from
 * Sim_100ab151(tree, 10) is dropped to the ground height (Sim_1002d961). Adds the list entry (Sim_10038fac), sends
 * the network message when this is the local player, and returns 1.
 */
int __cdecl Sim_100386d5(uint32_t *obj, int32_t id, int place);
/* The name of a DirectSound error code ("DSERR_..."), or "Unknown HRESULT". */
const char *__cdecl Sound_ErrorName(int32_t hr);
/*
 * Loads the renderer plugin DLL name (unless [0x1015e56c] is set): when its GetFuncDesc reports the kind "RASTER"
 * and its CheckFunc(0) (if any) is not 0, adds an entry (description, file name without directory, id) to the
 * 0x124-byte table list at [0x1015e574]; frees the library again.
 */
void __cdecl Sim_100361d8(char *name, uint8_t *list);
/* Switches the window style and placement for mode rec[1] (2 = windowed) and the cursor; sets [0x1015e9bc]. */
void __cdecl Sim_100381d0(const uint32_t *rec);
/* [0x1015eb68] = GetModuleHandleA(name), then 0x1003be30(); returns the handle. */
uint32_t __cdecl Sim_1003c5a0(const char *name);
void __cdecl Sim_1003ca04(void);

#endif
