/*
 * Round 8, letter w: functions in 0x10062386..0x1006d911 (cockpit display pieces drawn from resources, a node
 * tree's bounding box, the contact step of a moving object, and some state handlers).
 * Floating point follows the original x87 code with the game's control word (53-bit precision): every
 * intermediate is a double and only a store to a float rounds to float. Truncation to int is the runtime's _ftol
 * (Render_Ftol, render/w2n.h). Helpers that are not converted (or that another round-8 list converts) are called
 * through their original address with SIM_FN; prototypes below.
 */
#ifndef HG_SIM_UNSORTED_1006_W8W_H
#define HG_SIM_UNSORTED_1006_W8W_H

#include <stdint.h>
#include "../simmem.h"
#include "../libm/hg_math.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* The C library fabs of the DLL (0x100fcb70), on a double; the result comes back in st(0). */
#define W8W6_FABS hg_fabs
/* Resource loader: upper-cases name and kind in place and returns the loaded data or 0 (0x100ba7d5). */
#define W8W6_CALL_100ba7d5 SIM_FN(uint32_t(__cdecl *)(char *, uint32_t, uint32_t), 0x100ba7d5)
/* Not converted (0x100f0205 is patched by hooks_*.c): the two blitters of a loaded image to a surface. */
#define W8W6_CALL_100f0205 SIM_FN(void(__cdecl *)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t), 0x100f0205)
#define W8W6_CALL_100e91dc SIM_FN(void(__cdecl *)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t), 0x100e91dc)
#define W8W6_CALL_100e96c6 SIM_FN(void(__cdecl *)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t), 0x100e96c6)
/* Not converted: palette selection for the two blitters above (0x100f01ad, 0x100e96a7). */
#define W8W6_CALL_100f01ad SIM_FN(void(__cdecl *)(uint32_t), 0x100f01ad)
#define W8W6_CALL_100e96a7 SIM_FN(void(__cdecl *)(uint32_t), 0x100e96a7)
/* Not converted: one 16-bit value per index (0x100f01ea). */
#define W8W6_CALL_100f01ea SIM_FN(uint32_t(__cdecl *)(int32_t), 0x100f01ea)
/* In another round-8 list: projects the point (*x, *y, *z) in place, non-zero when it is on the screen. */
#define W8W6_CALL_100d2669 SIM_FN(int(__cdecl *)(float *, float *, float *), 0x100d2669)
/* Not converted: three angles (two floats and a vector of three) for the object (0x1000c1ea). */
#define W8W6_CALL_1000c1ea SIM_FN(void(__cdecl *)(uint32_t, float *, float *, float *), 0x1000c1ea)
/* Not converted: fill the 0x38-byte ray record for the object; a value in st(0); move along the ray. */
#define W8W6_CALL_100d6f16 SIM_FN(void(__cdecl *)(uint32_t, void *), 0x100d6f16)
#define W8W6_CALL_100d6e99 SIM_FN(double(__cdecl *)(uint32_t), 0x100d6e99)
#define W8W6_CALL_1009cd36 SIM_FN(void(__cdecl *)(void *, float), 0x1009cd36)
/* Not converted: the image size and offset words of a resource (0x100f1f52, 0x100f1f86). */
#define W8W6_CALL_100f1f52 SIM_FN(uint32_t(__cdecl *)(uint32_t, uint32_t), 0x100f1f52)
#define W8W6_CALL_100f1f86 SIM_FN(uint32_t(__cdecl *)(uint32_t, uint32_t), 0x100f1f86)
/* Not converted: draw to a surface described by a record (pointer to {bits, width, height}, rectangle). */
#define W8W6_CALL_100e9cd1                                                                                          \
    SIM_FN(void(__cdecl *)(void *, uint32_t, uint32_t, int32_t, int32_t, uint32_t, uint32_t, uint32_t, uint32_t, \
                           uint32_t),                                                                               \
           0x100e9cd1)
#define W8W6_CALL_100ea9f5 SIM_FN(void(__cdecl *)(void *, uint32_t, uint32_t, uint32_t, uint32_t), 0x100ea9f5)
/* Not converted: state handlers of an object record (0x1006b52f, 0x1006a12d, 0x1006bf33, 0x1006ad98). */
#define W8W6_CALL_1006b52f SIM_FN(void(__cdecl *)(uint32_t, void *, int32_t, int32_t, uint32_t), 0x1006b52f)
#define W8W6_CALL_1006a12d SIM_FN(void(__cdecl *)(uint32_t, void *), 0x1006a12d)
#define W8W6_CALL_1006bf33 SIM_FN(void(__cdecl *)(uint32_t, int32_t, void *), 0x1006bf33)
#define W8W6_CALL_1006ad98 SIM_FN(void(__cdecl *)(uint32_t, void *, int32_t, uint32_t), 0x1006ad98)
/* Not converted: a test on the object (0x100c2ae5). */
#define W8W6_CALL_100c2ae5 SIM_FN(int(__cdecl *)(uint32_t), 0x100c2ae5)

/*
 * When [0x1017692c] is set: the 8 x 4 words at 0x10176960.. (0x200 bytes apart) get 0x100f01ea of the indices
 * 0..31 in the rotated order of the code.
 */
void __cdecl Sim_10062386(void);

/*
 * For the record obj (+0x84 array of 0x128-byte entries indexed by +0x4fc): when the dword +0xf0 of obj[0] has
 * 0x200 in bits 8..11 and not 0x1000, and the entry's +8 is 1, the angles 0x1000c1ea(obj[0]) are projected with
 * 0x100d2669; the screen position is not used further.
 */
void __cdecl Sim_10062abd(uint8_t *obj);

/*
 * Draws the pieces of a display (resources 0x10144ffc.. 0x1014503c) with Sim_1006535f / Sim_100652e8 on the surface
 * +0xec of [0x109e8940] at the position [0x10144ed0], [0x10144ed4] according to the floats a (replaced by b unless
 * the dword +0xf0 of obj[0] has 0x100) and c; nothing when that dword has no bits 8..11 or has 0x1000 (unless
 * obj+0x50c is 2, which skips the first two pieces only).
 */
void __cdecl Sim_10063f00(uint8_t *obj, uint32_t unused, uint32_t unused2, float a, float b, float c);

/*
 * For a non-null image: clears the
 * 0xfe01-byte image, sets its first byte to 0xf, draws into it as a 0xb2 x 0xb2 surface with 0x100e9cd1 and
 * 0x100ea9f5 (palette 0x10166910) and blits it with 0x100f0205 to (x - 0x59, y - 0x59) on dest.
 */
void __cdecl Sim_1006419e(uint32_t dest, uint32_t res, uint32_t a3, int32_t x, int32_t y, uint8_t *image, uint32_t a7,
                          uint32_t a8, uint32_t a9, uint32_t a10);

/*
 * Moves a 0x38-byte ray record of the object *pobj (0x100d6f16) by 0x100d6e99 (0x1009cd36), projects its end point
 * (0x100d2669) and stores the truncated screen x, y in *outX, *outY. Returns the projection's result.
 */
int __cdecl Sim_10064824(uint32_t *pobj, int32_t *outX, int32_t *outY);

/*
 * When the dword +0xbcb of block +0x28 of the current record PTRTAB([0x1017b8c8]) has bit 4 and the point
 * 0x1017693c.. projects (0x100d2669): Sim_1006519d(truncated x, truncated y, [0x109e85c0]).
 */
void __cdecl Sim_10065131(void);

/*
 * Loads the resource `name` (0x100ba7d5 with the kind [0x1014cec0]) and, when it loaded, blits it to the surface
 * 0x109e3e00 at (x, y) (0x100f0205) and releases it (Sim_100bb2b0).
 * Returns 0 when it did not load, else the release's result.
 */
uint32_t __cdecl Sim_1006519d(uint32_t x, uint32_t y, char *name);

/* As Sim_1006519d, with the palettes `palette` and then 0x101659e0 around the blit, and only when [0x1017692c]. */
uint32_t __cdecl Sim_10065216(uint32_t x, uint32_t y, char *name, uint32_t palette);

/* As Sim_1006519d with the surface `dest`. */
uint32_t __cdecl Sim_100652e8(uint32_t x, uint32_t y, char *name, uint32_t dest);

/*
 * Moves the point (*x, *y, *z) out of the first object of the cell lists 0x1036f2a0 (after Sim_100881dd(*x, *z))
 * that it is inside of (Sim_10067356 with the radius obj+0x53c, nodes of type 6 without flag 0x100 in word +2):
 * for a hit whose word +2 has 0x50 in bits 4..7 only an effect is placed (when the length of obj+0x548.. exceeds
 * the float at 0x1012b600) and 0 is returned; otherwise the push direction goes to the floats 0x1015dd58..60, the
 * point is set on the hit's surface, the velocity obj+0x548.. is turned along the direction and *hit = the hit,
 * returning 1. *hit is cleared for every list.
 */
int __cdecl Sim_1006754e(uint8_t *obj, float *x, float *y, float *z, uint32_t *hit);

/*
 * For the record s (+0 object): steps through the entries of the table at 0x109a8650 (0x54 bytes) with
 * Sim_100cbdb8 while s+0x50c is 1, marks them, plays the sound 0x101452d4 for a newly marked entry, then
 * (unless the state was ended) runs 0x100c2ae5 / Sim_100cb8ea / Sim_100144e9 and keeps the dword +0x218 of the
 * object in step with the +8 of its block +0x54.
 */
void __cdecl Sim_10069c90(uint8_t *s);

/*
 * When [0x10179ce0] is set: the first slot (of 17, 0x3c bytes at s+0x88) whose id words (+0xc, more than 10000)
 * match a part (0x24 bytes at s+0x8c, count s+0x51c) with a live entry (+0x20 > 0) gets 0x1006b52f(a, s, slot + 1,
 * index, 0); otherwise 0x1006a12d(a, s).
 */
void __cdecl Sim_10069fd2(uint32_t a, uint8_t *s);

/*
 * Event `code` (with the 16-bit value a) for the record s: removes the object's child (Sim_100386d5) for codes 2
 * and 3 (3 also Sim_1006ab0c and s+0xc45 = 1), passes codes 9, 10, 11, 13..16 to 0x1006bf33, and for an object of
 * type 9 sets flags and plays the sound "AFXINFD" + one of three digits (Sim_1000ed52) at its position relative
 * to the camera; other types go to 0x1006ad98.
 */
void __cdecl Sim_1006abf6(uint32_t a, uint8_t *s, int32_t code);

/*
 * Observation only (the damage log, src/hooks/hooks_damagelog.c; null otherwise): Sim_1006abf6 calls the observer
 * first with its arguments and g_simDestroyShot, the shot of the hit that destroys the part, which Sim_1006c161 sets
 * around its call (null for every other caller). Neither changes what the game does.
 */
extern void(__cdecl *g_simDestroyObserver)(uint32_t killer, const uint8_t *s, int32_t code, const uint8_t *shot);
extern const uint8_t *g_simDestroyShot;

/*
 * Bounding box of a node and (recursively) its first child (+4) and next sibling (+8): Sim_1002fd4c of the
 * node's object (+0x8c) moved by the node's offset (+0x30..+0x38) and grown by the boxes of the child and the
 * sibling. Writes the six floats and returns 1; returns 0 when an argument is null.
 */
int __cdecl Sim_1006d5d0(uint8_t *node, float *minX, float *maxX, float *minY, float *maxY, float *minZ, float *maxZ);

/*
 * Box of the object's node tree (block +0x48) at the origin, widened by the float at 0x1012b6a0 into obj+0x284..
 * +0x298, then the node is put back at the object's position (+0x58..) and angles (+0x64..).
 */
void __cdecl Sim_1006d911(uint8_t *obj);

#endif
