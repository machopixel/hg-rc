/*
 * Round 5, letter a: functions of 0x100bf9a1..0x100c6eae that call helpers which are not converted yet.
 * Those helpers are called through their address (SIM_FN) with the prototypes below.
 */
#ifndef HG_SIM_UNSORTED_100C_W5A_H
#define HG_SIM_UNSORTED_100C_W5A_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Not converted: (object, 16-bit value); it only reads the low word of the second argument, the original
 * leaves the upper half of a register there.
 */
#define W5A_CALL_1001403a SIM_FN(int(__cdecl *)(void *, uint32_t), 0x1001403a)
/* Not converted: one dword. */
#define W5A_CALL_100c4d63 SIM_FN(int(__cdecl *)(void *), 0x100c4d63)
/* Not converted: (a, b, c, 28000.0f as a dword, out dword pointer); returns a count. */
#define W5A_CALL_10033eb3 SIM_FN(int(__cdecl *)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t *), 0x10033eb3)
/* Not converted: the C library's fmod(x, y). */
#define W5A_FMOD SIM_FN(double(__cdecl *)(double, double), 0x100fcb30)
/* Not converted: the C library's case-insensitive string compare. */
#define W5A_STRICMP SIM_FN(int(__cdecl *)(const char *, const char *), 0x100ff1e0)

/*
 * Calls Sim_100c274a(obj, 0), then: word +0x1fa = word +0x1f8, word +0x1f8 = 0xffff, dwords +0x200, +0x204,
 * +0x218, +0x21c = 0, word +0x1a8 = word +0x1aa and 0x1001403a(obj, word +0x1aa).
 */
void __cdecl Sim_100bf9a1(void *obj);
/*
 * Sim_100c476c(obj) == 0: dword obj + 0x1fc = 0x16, returns 1. Otherwise 0x100c4d63(obj) != 0: +0x1fc =
 * 0x17, returns -1. Otherwise +0x1fc = -1, returns 0.
 */
int __cdecl Sim_100c1853(void *obj);
/*
 * Sets the dword obj + 0xf0 to key with 0x1001403a, then computes fmod(Sim_10015266(obj) + 360.0, 360.0),
 * restores the old dword with 0x1001403a and returns the float (in st(0)).
 */
float __cdecl Sim_100c34b7(void *obj, int16_t key);
/*
 * -1 for a null obj. Otherwise calls 0x10033eb3(obj[0x58], obj[0x5c], obj[0x60], 28000.0f bits, &out) and
 * returns 1 when it returned a non-zero count and wrote a non-zero out (out starts as 0), else 0.
 */
int __cdecl Sim_100c4da3(const void *obj);
/*
 * Index of the first of the obj[0x4b4] (signed count) 12-byte entries, string pointer at +4 of each, whose
 * string equals name when compared without regard to case; -1 if there is none.
 */
int __cdecl Entry_FindByName(const void *obj, const char *name);

#endif
