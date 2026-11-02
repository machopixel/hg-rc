/*
 * Round 8, letter u: functions in 0x100278c0..0x1002e91e (debug log, resource cache, debris pieces, collision
 * walk). Helpers that are not converted (or that another round-8 list converts, or that a hook file patches) are
 * called through their original address with SIM_FN; prototypes below. Floating point follows the original x87
 * code with the game's control word (53-bit precision): every intermediate is a double and only a store to a
 * float rounds to float (see util/w3o.h for the FCOM idioms). Floats the original only moves are raw bits.
 */
#ifndef HG_SIM_UNSORTED_1002_W8U_H
#define HG_SIM_UNSORTED_1002_W8U_H

#include <stdint.h>
#include "../simmem.h"
#include "../libm/hg_math.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#define __stdcall
#endif

/* The C library of the DLL: sprintf 0x100fc780, fclose 0x100fd760, fabs 0x100fcb70 (double in, st(0) out). */
#define W8U_SPRINTF_1002 SIM_FN(int(__cdecl *)(char *, const char *, ...), 0x100fc780)
#define W8U_FCLOSE_1002 SIM_FN(int(__cdecl *)(void *), 0x100fd760)
#define W8U_FABS_1002 hg_fabs
/* Not converted: (name, extension pointer, 0); upper-cases both texts in place, returns a resource record or 0. */
#define W8U_CALL_100ba7d5 SIM_FN(uint8_t *(__cdecl *)(void *, uint32_t, int), 0x100ba7d5)
/* Patched by src/hooks/hooks_sim.c (TapSystemError): (code, format, ...). */
#define W8U_CALL_100bc280 SIM_FN(void(__cdecl *)(int, const char *, ...), 0x100bc280)
/* Not converted: (a, name, key (low 16 bits used), destination); -1 on failure. */
#define W8U_CALL_100a836e SIM_FN(int(__cdecl *)(uint32_t, void *, uint32_t, void *), 0x100a836e)
/* Not converted: (object). */
#define W8U_CALL_10029b1f SIM_FN(void(__cdecl *)(void *), 0x10029b1f)
/* Not converted: (face, points, a, b as float bits), like Sim_1002e9f5 / Sim_1002ed0b for the third plane. */
#define W8U_CALL_1002f021 SIM_FN(int(__cdecl *)(void *, void *, uint32_t, uint32_t), 0x1002f021)
/* HeapSize / HeapAlloc through the DLL's import slots. */
/* Appends text to "hgdebug.txt" (mode "a"; fclose is called even when the open failed) and prints it with
 * Mono_PrintText. */
void __cdecl Debug_LogText(const char *text);

/* Debug_LogText of "<Sim_10027acf(code)>, ". */
void __cdecl Debug_LogCodeText(uint32_t code);

/*
 * rec: word id, then an 8-character name at +2. Copies the name to out+6 (8 bytes, out[0xe] = 0), logs it when
 * [0x1015dc28] is set, sets out dword +0 = 1 and word +4 = id, and looks the name up with 0x100ba7d5(name,
 * [0x1014cefc], 0); when found, out +0xf/+0x13 = record +4/+8 and +0x17/+0x1b = the record, and returns out.
 * Not found: system error 0x2a ("%s ID %d") and 0.
 */
uint8_t *__cdecl Sim_100278c0(const uint8_t *rec, uint8_t *out);

/* Writes "dbugcch<n>.log" (n = [0x1015dc34]++): every node of the 0x3f1 hash chains at [0x1015dc30] (next +8),
 * then the purge list [0x109f13f0] (next +0xc), one line each with the id, 4-char type, lock and HeapSize. */
void __cdecl Cache_DumpLog(void);

/*
 * The cache's UseItem (from its own message): key < 0 gives 0; a cached node (Sim_10027f33) is touched
 * (Sim_10027cce) and its data (+0x14) returned. Otherwise (purging [0x109f13f0] with Sim_10027fb8 while the count
 * [0x109f13e0] is >= 1000 or HeapAlloc fails) loads the item into a new 0x14-byte-header block, hashes it into
 * the 0x3f1 chains and returns its data, or 0.
 */
void *__cdecl Cache_UseItem(uint32_t a, int key, const void *name);

/* Steps the 0x80 debris pieces (Sim_10029421) and retires the ones whose time ran out. */
void __cdecl Sim_10029067(void);

/* For an object with bit 0 of +0x94: rebuilds its matrix +0xc (and +0x4c) around its pivot object +0x8c and
 * recurses into the children (+4, next +8). */
void __cdecl Sim_100291e3(void *obj);

/* Moves debris piece `index` (0x30-byte entries at 0x109eeb00) one frame: gravity, ground bounce, position. */
void __cdecl Sim_10029421(int index);

/*
 * (node, segment, out segment, distance (in/out), out a, b, c): nearest hit of the segment against the node's
 * sphere test and its children (recursively); returns the hit node or 0.
 */
uint32_t __cdecl Sim_1002dfbb(void *node, const void *seg, void *outSeg, uint32_t *dist, uint32_t *outA,
                              uint32_t *outB, uint32_t *outC);

/* (face, points, x, y, z as float bits): picks the projection plane by the largest normal component. */
int __cdecl Sim_1002e91e(void *face, void *points, uint32_t x, uint32_t y, uint32_t z);

#endif
