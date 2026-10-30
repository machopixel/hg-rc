/*
 * Round 8, letter t: functions in 0x10007890..0x1000ee23. The mono-screen debug displays (frame rate, position,
 * memory, version line and their dispatcher), the line-of-sight checks around 0x1000a6f0 and the sound-slot
 * functions that use the table [0x1015d0a4].
 * Helpers that are not converted (or that another round-8 list converts) are called through their original
 * address with SIM_FN; their prototypes are below.
 * Floating point follows the original x87 code with the game's control word (53-bit precision): every
 * intermediate is a double and only a store to a float rounds to float (see util/w3o.h for the FCOM idioms).
 * A truncation to int is the runtime's _ftol (0x100fcb08), written as Render_Ftol (render/w2n.h).
 */
#ifndef HG_SIM_UNSORTED_1000_W8T_H
#define HG_SIM_UNSORTED_1000_W8T_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* The C library sprintf of the DLL (0x100fc780). */
#define W8T_SPRINTF_1000 SIM_FN(int(__cdecl *)(char *, const char *, ...), 0x100fc780)
/* Not converted: draws a text with a background (font name, ..., text, x, y) (0x1007e29a). */
#define W8T_CALL_1007e29a SIM_FN(void(__cdecl *)(const char *, uint32_t, const char *, uint32_t, uint32_t), 0x1007e29a)
/* The C library fmod (x, y) of the DLL on doubles; the result comes back in st(0) (0x100fcb30). */
#define W8T_FMOD_1000 SIM_FN(double(__cdecl *)(double, double), 0x100fcb30)
/* Not converted: from a float vector (x, y, z) four floats through the pointers (0x100cbc6b). */
#define W8T_CALL_100cbc6b_1000 SIM_FN(void(__cdecl *)(float, float, float, float *, float *, float *, float *), 0x100cbc6b)
/* Not converted: table lookup of one float, returns a float value in st(0) (0x10084766). */
#define W8T_CALL_10084766_1000 SIM_FN(double(__cdecl *)(float), 0x10084766)
/* Not converted: (obj, &float, &float, &float) (0x1000c1ea). */
#define W8T_CALL_1000c1ea SIM_FN(void(__cdecl *)(void *, float *, float *, float *), 0x1000c1ea)
/* Not converted: takes a sound slot (7 arguments, the third read as 16 bits), returns the slot or a negative code (0x1000e0d9). */
#define W8T_CALL_1000e0d9 SIM_FN(int32_t(__cdecl *)(int32_t, int32_t, uint32_t, int32_t, int32_t, uint32_t, uint32_t), 0x1000e0d9)
/* Not converted: volume of a distance given as a float (0x1000f096). */
#define W8T_CALL_1000f096 SIM_FN(int32_t(__cdecl *)(float), 0x1000f096)
/* Not converted: (0, key, name, 0), returns a handle (0x100282d4). */
#define W8T_CALL_100282d4 SIM_FN(uint32_t(__cdecl *)(uint32_t, int32_t, uint32_t, uint32_t), 0x100282d4)

/*
 * The mono-screen debug dispatcher: Sim_1000799a when [0x1015d01c], Mono_TickSpinner when [0x1012dc9c], then one
 * of each pair: Mono_ShowFrameRate ([0x1015d02c]) or Sim_10007c45 ([0x1015d028]), Sim_10007c9e ([0x1015d034]) or
 * Sim_10007ed5 ([0x1015d03c]), Mono_ShowPosition([0x1014cae0]) ([0x1015d040]) or Sim_10008123 ([0x1015d048]),
 * Mono_ShowMemory ([0x1012dc98]) or Sim_100082be ([0x1015d050]), Cache_ShowItemCount ([0x1015d054]) or
 * Cache_ClearItemCount ([0x1015d058]). [0x1015d064] = 1 when [0x1015d068]; when [0x1015d060] and the tick count
 * [0x1015dc78] is past [0x1015d05c]: [0x1015d05c] = ticks + 0xb5 and Sim_1002850e.
 */
void __cdecl Sim_10007890(void);

/*
 * When [0x1015d030]: counts frames in [0x1015d07c] and sums the frame ticks [0x1015dc80] in [0x1015d078]; after
 * ten frames (with a positive sum) the rate frames * 0x712 / sum goes to [0x1015d070] (whole) and [0x1015d074]
 * (tenths) and is printed "Framerate .." on the mono screen; both counters restart. With a positive rate it also
 * draws the "F:.. P:.. T:.. V:.." line through 0x1007e29a. Sets [0x1015d028] = 1.
 */
void __cdecl Mono_ShowFrameRate(void);

/*
 * When [0x1015d044] or the mono screen [0x10a3ade8] is on: prints "txyz" (floats pos+0, +4, +8) and "rxyz" (+0x10,
 * +0xc, +0x14) on rows 1 and 2 of the mono screen; with [0x1015d044] draws all six through 0x1007e29a. Sets
 * [0x1015d048] = 1.
 */
void __cdecl Mono_ShowPosition(const float *pos);

/* Prints "Mem " (once, [0x1015d050] = 1) and the number 0 on row 0 of the mono screen; draws it with [0x1015d04c]. */
void __cdecl Mono_ShowMemory(void);

/*
 * Formats "%s  Res:%dx%d  Mem:%ld" (name, the two dwords at [0x109e3da4], mem) for Sim_1007df6c(text, 0, 0x4f3,
 * 0x50); with the mono screen on also writes "WarThink / Heavy Gear %s %ld" (name, mem) as its top line.
 */
void __cdecl Mono_ShowVersion(const char *name, uint32_t mem);

/*
 * Whether obj can see its target: a = fmod(obj[0x68] + obj[0x74] - obj[0xe8] + f[0x10111030], 360) (floats);
 * with check set and f[0x10111038] < a < f[0x10111034] returns 0. Builds the segment of obj (Sim_100d6f16, cut to
 * obj[0xd8] by Sim_1009cd36) and looks for a hit (Sim_1002dda9, else Sim_100c2851 with id obj[4]): no hit gives
 * 1; a hit node with bit 0x100 (or 0x200) in word +2 and bit 0x200 (or 0x400) in obj[0xf0] gives whether its word
 * +0x24 equals the low byte of obj[0xf0]; otherwise 0.
 */
int __cdecl Sim_1000a6f0(void *obj, int check);

/*
 * Walks from (x, y, z) towards the point obj[0x58..0x60] in steps of f[0x1011103c] (direction and distance from
 * 0x100cbc6b and the table 0x10084766) and returns 0 as soon as the ground height Sim_1002d961 of a step is not
 * within f[0x1011103c] of 0, else 1.
 */
int __cdecl Sim_1000a866(void *obj, float x, float y, float z);

/*
 * A shot decision of obj against the angle a (see the code: 0x1000c1ea, fmod to 360, the limits at 0x1011109c,
 * 0x101110b0, 0x1011104c, 0x101110b4, the timer obj[0x1be] and Sim_1009c929); on success sets byte +0x28 (or
 * +0x26) of obj[0x54] and returns 1. Always stores the floats obj[0x54]+4 and obj[0x54]+0 from Sim_1001448d and
 * Sim_100144bb.
 */
int __cdecl Sim_1000d0e2(void *obj, float a);

/*
 * Starts a sound in the slot table [0x1015d0a4] (needs [0x109e369c] and a non-zero first dword, else -2):
 * v = trunc(a4 - (a5 - 0xffff) * ((a4 - 10000) / f[0x101110d0])), below -6000 gives 0, above 0 becomes 0; a9 keeps
 * bits 8..15 and gets 0x32 or the word of table 0x109f1520 for a2 (a2 < 1 with a3 == 0 gives -6); 0x1000e0d9(a2,
 * a3, a9, -1, a8, a10, a11) picks the slot (a negative value is returned as is). In the table the slot gets: +0x1d4
 * the tick count [0x1015dc78] (plus a0 when a0 > [0x1015dc80]), +0x334 a8 (or -1), +0x254 a6, +0x214 a1, +0x294
 * v, +0x54 = 1. Returns the slot.
 */
int32_t __cdecl Sim_1000e737(int32_t a0, uint32_t a1, int32_t a2, int32_t a3, int32_t a4, int32_t a5, uint32_t a6,
                             int32_t a7, int32_t a8, uint32_t a9, uint32_t a10, uint32_t a11);

/*
 * Plays the named sound at the point (x, y, z) when it is close enough (0x100cbc6b, the limit [0x1015ebb4], the
 * volume 0x1000f096 and Sim_1000ed25). Returns the truncated distance.
 */
int32_t __cdecl Sim_1000eb12(float x, float y, float z, const char *name, int quieter, uint32_t a5, uint32_t a6);

/*
 * Updates the positional sound obj: stops it (Sim_1000f029) when out of range, takes a free slot 0xb..0xf of the
 * table [0x1015d0a4] and a handle (0x100282d4) when it has none, and marks the slot active.
 */
void __cdecl Sim_1000ee23(void *obj);

#endif
