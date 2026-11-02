/*
 * Round 6, letter e: functions at 0x10035478..0x1003ce98 (driver-table calls, plugin probe, sound state set-up and
 * a few state switches). Helpers that are not converted yet are called through their original address (SIM_FN)
 * with the prototypes below; Windows functions through the DLL's import slots (SIM_API).
 */
#ifndef HG_SIM_UNSORTED_1003_W6E_H
#define HG_SIM_UNSORTED_1003_W6E_H

#include <stdint.h>
#include "../simmem.h"
#include "../util/access.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#define __stdcall
#endif

/* Not converted: takes no arguments (calls several state handlers chosen by flags at 0x1015d01c..). */
#define W6E_CALL_10007890 SIM_FN(void(__cdecl *)(void), 0x10007890)
/* Not converted: C library _strcmpi. */
#define W6E_CALL_100ff1e0 SIM_FN(int(__cdecl *)(const char *, const char *), 0x100ff1e0)
/* Not converted: (x, y, z); returns the length of the vector in st(0). */
#define W6E_CALL_10084b8c SIM_FN(double(__cdecl *)(float, float, float), 0x10084b8c)
/* Hooked by hooks_audio.c (SoundCreate): (address of the DirectSound object pointer, window). */
#define W6E_CALL_1003a0e4 SIM_FN(int(__cdecl *)(void *, uint32_t), 0x1003a0e4)

#ifndef HG_SIM_W6E_FLOAT_ACCESS
#define HG_SIM_W6E_FLOAT_ACCESS
/* A float of the original's data at a byte offset (may alias anything, like the U32 family in util/access.h). */
typedef float HG_MAY_ALIAS w6e_f32;
#define W6E_F(p, off) (*(w6e_f32 *)(BYTES(p) + (off)))
#define W6E_GF(addr) (*SIM_PTR(w6e_f32, addr))
static inline uint32_t W6e_Bits(float f)
{
    union {
        float f;
        uint32_t u;
    } v;
    v.f = f;
    return v.u;
}
static inline float W6e_Float(uint32_t u)
{
    union {
        float f;
        uint32_t u;
    } v;
    v.u = u;
    return v.f;
}
#endif

typedef void *(__stdcall *W6eLoadLibraryFn)(const char *name);
typedef void *(__stdcall *W6eGetProcAddressFn)(void *module, const char *name);
typedef int(__stdcall *W6eFreeLibraryFn)(void *module);
/*
 * Runs 0x10007890, 0x100d3491 and 0x100d33ec; then, when [0x109e3dac] is set, calls slot 0x20 of the
 * table at [0x1015e68c] with (rect left + 1, top + 1, right, bottom) of the rectangle at 0x109e3e04, copies the five
 * dwords at 0x1015e518 to 0x109e3e00 and clears the flag; otherwise calls slot 0x18 when [0x1017b920] is set.
 */
void __cdecl Sim_10035478(void);
/*
 * Loads the DLL `path`, asks its GetFuncDesc for the kind; for a "RASTER" plugin stores GetNumDevice(arg) in
 * [0x1015e56c] and, when that is not 0, copies the path to 0x1015e57c. Does nothing once [0x1015e56c] is set.
 */
void __cdecl Plugin_ProbeRaster(const char *path, uint32_t arg);
/*
 * id -1: returns 1 when an entry of the list at [0x109e3760] (next at +0x690) lies closer than 1000 to the position
 * (+0x70..0x78) of the object [*obj + 0x48]; otherwise returns 1 when an entry has +0x124 == id. Else 0.
 */
int __cdecl Sim_10038eea(void *obj, int id);
/*
 * Sets up the sound state: allocates the 0x374-byte block at [0x1015d0a4] from the game heap, zeroes it, stores the
 * voice count (at most 11) at +4, calls 0x1003a52b(0) and SoundCreate(0x109e369c, window), and resets the voice
 * slots. Returns 1, -1 when the block exists already, -3 when the allocation fails, -4 when no DirectSound object
 * was made (the block is freed again).
 */
int __cdecl Sound_Init(uint32_t voices, uint32_t window);
void __cdecl Sim_1003cbb3(uint32_t unused, int mode);
void __cdecl Sim_1003ce98(uint32_t a, uint32_t b, int value, uint32_t d, uint32_t e, void *obj);

#endif
