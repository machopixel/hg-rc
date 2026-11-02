/*
 * Round 7, letter o, functions at 0x1009xxxx: more updates of the effect objects of the pool of Sim_10090c7d
 * (layout in w6f.h: +0x10 flags, 16 particles of 0x2a bytes from +0x12, +0x552 scene node, +0x55e start tick,
 * +0x562 lifetime, +0x566/+0x56a values from byte tables, +0x56e/+0x572/+0x576 drift, +0x57a source), the
 * spawner of one kind of them, two view-mode switches, the CD status poll, a slab test, an upper-case copy and
 * the drive-letter query.
 *
 * Floating point: computed in double and stored to float where the original stores a float (the x87 runs with
 * a 53-bit mantissa, see util/w3o.h); truncations go through Render_Ftol. The results are bit-exact.
 * Callees that are not converted (or patched by another hook file) are called through their original address.
 */
#ifndef HG_SIM_UNSORTED_1009_W7O_H
#define HG_SIM_UNSORTED_1009_W7O_H

#include <stdint.h>
#include "../simmem.h"
#include "../libm/hg_math.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Not converted: twelve-argument helper the expired effect of Sim_10090f71 calls with (-2, 0, 0x49, x, y, z, 0...). */
#define W7O_CALL_100246b3 \
    SIM_FN(void(__cdecl *)(int32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, \
                           uint32_t, uint32_t, uint32_t), 0x100246b3)
/* Not converted: a float function of one float (C library 0x100fcb70 inside), result in st(0). */
#define W7O_CALL_10084766 SIM_FN(double(__cdecl *)(float), 0x10084766)
/* Not converted: C library function of a double, result in st(0). */
#define W7O_CALL_100fcb70 hg_fabs
/* Not converted: called by Sim_1009b99b when Sim_1009b8d8 finds nothing. */
#define W7O_CALL_10099c36 SIM_FN(void(__cdecl *)(void), 0x10099c36)
/* Not converted: C library character conversion used by Str_CopyUpper (to upper case). */
#define W7O_CALL_100ff280 SIM_FN(int(__cdecl *)(int), 0x100ff280)
/* Not converted: C library sprintf, called with a buffer and a format only. */
#define W7O_CALL_100fc780 SIM_FN(int(__cdecl *)(char *, const char *), 0x100fc780)

/*
 * Effect updates. While the age ([0x1015dc78] - +0x55e) is not above the limit they move the node and return 1;
 * after that they clear bit 0 of +0x10, call Sim_100aa351(node) and return 0.
 */
/* Limit +0x562: copies the position (+0xc..+0x14) of the object +0x57a to +0x12/+0x16/+0x1a and moves the node to
   (+0x12, +0x22, +0x26). When expired it first calls 0x100246b3(-2, 0, 0x49, +0x12, +0x16, +0x1a, 0 x6). */
int32_t __cdecl Sim_10090f71(uint8_t *obj);
/* Limit +0x562: drift, +0x56a = byte [0x10215bb0 + trunc(age / limit * [0x1012b954])], +0x566 = 0xfe, every
   particle position += its speed. */
int32_t __cdecl Sim_10092a71(uint8_t *obj);
/* Limit 0xb5: drift, i = trunc(age / [0x1012b960] * [0x1012b954]), +0x56a = byte [0x10217b20 + i], particle
   +0x1e = dword i of the table at particle +0x1a, position += random (-50..49, -10..9, -50..49). */
int32_t __cdecl Sim_10093ce7(uint8_t *obj);
/* Limit 0x21f: drift, i = trunc(age / [0x1012b96c] * [0x1012b954]), +0x566 = byte [0x10215bd0 + i],
   +0x56a = byte [0x10217b20 + i], every particle position += random -10..9 per axis. */
int32_t __cdecl Sim_10094423(uint8_t *obj);
/* Limit 0x16a: drift, +0x566 = byte [0x10215af0 + trunc(age / [0x1012b974] * [0x1012b954])], every particle
   position += random -30..29 per axis. */
int32_t __cdecl Sim_10094c2a(uint8_t *obj);
/* Limit 0xb5: drift, +0x566 = byte [0x10217b90 + trunc(age / [0x1012b960] * [0x1012b954])], +0x56a = 1, every
   particle position = position + node position (Sim_100a9cfb) + random (-3n..3n-1, n = +0x566). */
int32_t __cdecl Sim_1009524d(uint8_t *obj);

/*
 * Unless [0x1017cc24] + 1 >= [0x1017cc20] or size is 0: when the position of `src` (Sim_100a9cfb) is within the
 * camera range ([0x1014cae0] +0x40, length by Sim_10084a94) of the camera, takes an effect from the pool
 * (Sim_10090c7d(0x100032c9, 0x1000356c, 0, 0)), places its node at the position, counts it, sets lifetime
 * size * 0xb5 and gives the 16 particles a random direction (0x10084766 of a random 0..359) times size * 30.
 */
void __cdecl Sim_10091597(void *src, int32_t size);

/*
 * View-mode switches (mode [0x1015d1ac], requested mode [0x1015d1cc]): while the mode is at most 2 they let
 * Sim_100999fc switch, and if it is still at most 2 remember [0x1015d1c8] in [0x1015d104] (when the mode is not
 * 0), set [0x1015d1c8] to 1 (Sim_1009b99b) or 3 (Sim_1009bc94) and apply the entry of the table 0x1014c4c8 for
 * the mode through Sim_1008f630 / Sim_1009b8d8. In mode 4 they only call Sim_100999fc when the request is at
 * most 2. Sim_1009b99b calls 0x10099c36 when Sim_1009b8d8 finds nothing.
 */
void __cdecl Sim_1009b99b(void);
void __cdecl Sim_1009bc94(void);

/*
 * Copies the text `src` to `dst`, letters ([0x1015965c] > 1: Crt_IsCType(c, 0x103), else the class table at
 * [0x10159450]) converted to upper case by 0x100ff280. Returns the address of the copied terminating zero.
 */
char *__cdecl Str_CopyUpper(char *dst, const char *src);

/*
 * Slab test of one axis: p position, d direction, [lo, hi] the slab. When |d| (0x100fcb70) is below the float
 * at 0x1012ba08: returns 1 when p is outside the slab, else stores -/+3.4e38. Otherwise t = (lo - p) / d and
 * (hi - p) / d. Stores the smaller at *outNear and the larger at *outFar (raw float bits) and returns 0.
 */
int __cdecl Sim_1009cfd4(float p, float d, float lo, float hi, uint32_t *outNear, uint32_t *outFar);

/* 0x10099510 (CD drive search): round 28, src/sim/platform/w28.h. */

#endif
