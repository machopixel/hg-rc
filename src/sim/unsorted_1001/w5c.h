/*
 * Round 5, letter c: functions at 0x1001555c..0x1001ffc8 that call helpers which are not converted yet.
 * Those helpers are called through their original address (SIM_FN) with the prototypes below.
 */
#ifndef HG_SIM_UNSORTED_1001_W5C_H
#define HG_SIM_UNSORTED_1001_W5C_H

#include <stdint.h>
#include "../simmem.h"
#include "../libm/hg_math.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Not converted: (object, word, word, flag); both words are read as 16-bit values. */
#define SIM_CALL_10015630 SIM_FN(void(__cdecl *)(void *, int16_t, int16_t, int), 0x10015630)
/* Not converted: (object, 1, 1). */
#define SIM_CALL_100cbdb8 SIM_FN(void(__cdecl *)(void *, int, int), 0x100cbdb8)
/* Not converted: x87 fmod(x, y) of the C library (the name "fmod" is in its error structure at 0x101593f0). */
#define SIM_FMOD hg_fmod
/* Not converted: returns the record of an id (used by Sim_1001bb1f; record +4 is a state, +0 the loaded data). */
#define SIM_CALL_1001b902 SIM_FN(uint8_t *(__cdecl *)(int), 0x1001b902)
#define SIM_CALL_1001cedf SIM_FN(void(__cdecl *)(int), 0x1001cedf)
/* Not converted: (name pointer, dword). */
#define SIM_CALL_100bb2b0 SIM_FN(int(__cdecl *)(const void *, const void *), 0x100bb2b0)
/* Not converted: (count, pointer). */
#define SIM_CALL_100bc280 SIM_FN(int(__cdecl *)(int, const void *), 0x100bc280)

/* With nothing stored (count at +0x198 is 0) calls 0x10015630 with the two words, otherwise stores them as the dword at obj+0x180+4*count. */
void __cdecl Sim_1001555c(void *obj, int16_t a, int16_t b);
/* Sets obj+0xf0 to the sign-extended word, calls 0x100cbdb8, and sets the word at +0x1a8 from +0x118 (when bit 8 of the argument is set) or +0xf0. */
void __cdecl Sim_100159cf(void *obj, int16_t value);
/*
 * The floating-point part of the original keeps its intermediates in x87 registers at the 53-bit precision the Windows loader
 * sets, so every intermediate is a double here and only the stored results are floats.
 *
 * r = fmod(Sim_10015266(obj), 360.0); stores a float in the record at obj+0x54, at +0xc: 819.2 when r > 45.0, -819.2 when r < -45.0
 * (or r is NaN), else r / 45.0 * 819.2 (the constants are the floats at 0x10111140, 0x10111154 and 0x10111158). Returns the value
 * of Sim_10015266 (a float, in st(0) as in the original).
 */
float __cdecl Sim_100144e9(const void *obj);
/* Loads `name` into the record of `id` when that record is not in state 1 yet. */
void __cdecl Sim_1001bb1f(const char *name, int id, int reload);
/* Frees all 0x80 records of 0x320 bytes at 0x10a1eaa0 that are in use. */
void __cdecl Sim_1001dcff(void);
/* Clears the dwords at +0 and +0x67, frees +0x24 and +0x28; returns the old dword at +0x67. */
uint32_t __cdecl Sim_1001ffc8(void *obj);

#endif
