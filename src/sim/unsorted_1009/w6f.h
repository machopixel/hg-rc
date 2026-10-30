/*
 * Round 6, letter f, functions at 0x1009xxxx: effect objects of the pool of Sim_10090c7d (0x57e bytes each),
 * their per-tick updates, and a few state switches.
 *
 * Effect object fields as these functions use them: +0xc state, word +0x10 flags (bit 0 = alive),
 * +0x12 16 particles of 0x2a bytes (floats +0, +4, +8 position, +0xc, +0x10, +0x14 speed, word +0x18 = 1,
 * dword +0x1a), +0x552 the scene node (its +0x8c block gets the position and size), +0x55e start tick
 * (from [0x1015dc78]), +0x562 lifetime in ticks, +0x566 and +0x56a values the update picks from byte tables,
 * floats +0x56e, +0x572, +0x576 the speed, +0x57a the argument of the constructor.
 *
 * Callees that are not converted or are patched by another hook file are called through their original
 * address with SIM_FN.
 */
#ifndef HG_SIM_UNSORTED_1009_W6F_H
#define HG_SIM_UNSORTED_1009_W6F_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Patched by another hook file (100988fc, 100989a1) or not converted (100b5bcb): (unused, value) setters. */
#define W6F_CALL_100988fc SIM_FN(void(__cdecl *)(int32_t, uint32_t), 0x100988fc)
#define W6F_CALL_100989a1 SIM_FN(void(__cdecl *)(int32_t, uint32_t), 0x100989a1)
#define W6F_CALL_100b5bcb SIM_FN(void(__cdecl *)(int32_t, uint32_t), 0x100b5bcb)
/* Not converted: works with the 5 dwords that 0x1009b8d8 puts in place around the call. */
#define W6F_CALL_10099c59 SIM_FN(void(__cdecl *)(void), 0x10099c59)

/*
 * Effect updates. While the effect's age ([0x1015dc78] - +0x55e) is not above its limit they move the node
 * (+0x552) and return 1; after that they clear bit 0 of +0x10, call Sim_100aa351(node) and return 0.
 */
/* Limit +0x562: copies the position of the object +0x57a (Sim_100a9cfb) to the node. */
int32_t __cdecl Sim_1009117d(uint8_t *obj);
/* Limit +0x562: copies 6 dwords from the record +0x57a to +0x12.., the node position from +0x12/+0x16/+0x1a. */
int32_t __cdecl Sim_100917ed(uint8_t *obj);
/* As Sim_100917ed with the position from +0x1e/+0x22/+0x26, and Sim_1000ecb0("AFXWLLLC") at the end. */
int32_t __cdecl Sim_10092117(uint8_t *obj);
/*
 * Limit 90: moves the node by (+0x56e, +0x572 + random 0..9, +0x576), +0x566 = byte table 0x1036f050
 * [trunc(age / 90 * 3)], +0x56a = 1.
 */
int32_t __cdecl Sim_10093295(uint8_t *obj);
/* As Sim_10093295 with the byte table 0x10215b18. */
int32_t __cdecl Sim_10093376(uint8_t *obj);
/* As Sim_10093295 with +0x566 from the byte table 0x10215bc0 and +0x56a from 0x10215bb0 (same index). */
int32_t __cdecl Sim_10093457(uint8_t *obj);
/* Limit 22: as Sim_10093295, +0x566 = byte table 0x101d09c0[trunc(age / 22 * 15)], +0x56a unchanged. */
int32_t __cdecl Sim_1009589c(uint8_t *obj);
/* Limit 45: as Sim_1009589c with the byte table 0x10215b10[trunc(age / 45 * 7)]. */
int32_t __cdecl Sim_10096095(uint8_t *obj);
/* Limit 45: moves the node by (+0x56e, +0x572, +0x576), +0x566 = 0x1f. */
int32_t __cdecl Sim_100967cc(uint8_t *obj);

/*
 * Effect constructor: unless the effect count [0x1017cc24] + 1 reaches [0x1017cc20], takes an object of the
 * pool with two code addresses of the DLL and `arg`, lifetime rec[+0x34] - rec[+0x30], +0x566 = param, resets
 * the node block, counts the effect, starts the sound "AFXWLLLC" and sets the state 0xd.
 */
void __cdecl Sim_10092956(uint32_t arg, const uint8_t *rec, uint32_t param);

/*
 * Particle effect constructors at (x, y, z) with speed (sx, sy, sz): nothing happens when the point is
 * farther from the camera ([0x1014cae0] +0/+4/+8) than its range (+0x40, distance by Sim_10084a94) or when
 * the pool or the effect count is exhausted. The node block gets the position and the size |speed|.
 */
/* type 0, 1, 2 picks the code addresses; 16 particles scattered by random 0..9 - 5 and speed random 0..39 - 20 around (speed / -5, -10, speed / -5). */
void __cdecl Sim_100939a3(float x, float y, float z, float sx, float sy, float sz, int32_t type);
/* Particles along speed / 16 steps, each with one of two 64-byte records at 0x10215b20 (random). */
void __cdecl Sim_100941c3(float x, float y, float z, float sx, float sy, float sz);
/* Particles along speed / 16 steps. */
void __cdecl Sim_100949e8(float x, float y, float z, float sx, float sy, float sz);
/* Particles along speed / 16 steps, each scattered by random 0..49 - 25. */
void __cdecl Sim_10094fcb(float x, float y, float z, float sx, float sy, float sz);
/* Speed (sx / 16, 5, sz / 16); particles along speed / 16 steps scattered by 0..9 - 5, 0..49 - 25, 0..9 - 5. */
void __cdecl Sim_10095616(float x, float y, float z, float sx, float sy, float sz);
/* Speed / 8; particles along speed / 16 steps scattered by 0..9 - 5, 0..49 - 25, 0..9 - 5. */
void __cdecl Sim_10095e0a(float x, float y, float z, float sx, float sy, float sz);
/* Speed / 2; particles along speed / 16 steps scattered by 0..3 - 2, 0..49 - 25, 0..3 - 2; state 2. */
void __cdecl Sim_10096541(float x, float y, float z, float sx, float sy, float sz);

/*
 * When the record [0x1015d1b8] exists, hands its values +0x14, +0x18, +0x1c, +0x24, +0x28, +0x2c and +0x30
 * to their seven setters (first argument 1). Returns 1. Its two arguments are not used.
 */
int32_t __cdecl Sim_10098721(int32_t unused1, int32_t unused2);

/*
 * Switches the mode [0x1015d1ac] to the wanted one [0x1015d1cc] (the previous one goes to [0x1015d098]),
 * saving/restoring the values 0x1014bef0.. and the driver slot 0x1014cb3c around modes 3, 4 and 5.
 * Returns 1 when the modes differed.
 */
int32_t __cdecl Sim_100999fc(void);

/*
 * Looks up the 5-dword block of `pair` (Sim_1008f747; when it finds none and `apply` is set, the block
 * [[pair + 4] + 8], or [[pair + 4] + 4] when flip is set). With a block: saves the 5 dwords view[0] points
 * to into view[1]'s buffer, puts the block's there for the call of 0x10099c59, puts the saved ones back and,
 * with `apply` set, copies the block to 0x109e3e00 and sets [0x109e3dac] = 1. Returns 1 when Sim_1008f747
 * found the block, else 0.
 */
int32_t __cdecl Sim_1009b8d8(int32_t flip, uint32_t *view, const uint32_t *pair, int32_t apply);

#endif
