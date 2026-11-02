/*
 * Round 7, letter l: functions in 0x1001150d..0x1001ee7c. Helpers that are not converted (or that another
 * round-7 list converts) are called through their original address with SIM_FN; prototypes below.
 * Floating point follows the original x87 code with the game's control word (53-bit precision): every
 * intermediate is a double, and only a store to a float rounds to float (see util/w3o.h for the FCOM idioms).
 * Floats that the original only moves as dwords are passed as raw bits (uint32_t).
 */
#ifndef HG_SIM_UNSORTED_1001_W7L_H
#define HG_SIM_UNSORTED_1001_W7L_H

#include <stdint.h>
#include "../simmem.h"
#include "../libm/hg_math.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#define __stdcall
#endif

/* sprintf of the DLL's C library (0x100fc780); fabs from OpenLibm (hg_fabs, in place of 0x100fcb70). */
#define W7L_SPRINTF_1001 SIM_FN(int(__cdecl *)(char *, const char *, ...), 0x100fc780)
#define W7L_FABS_1001 hg_fabs
/* Not converted: (object). */
#define W7L_CALL_1001457f SIM_FN(void(__cdecl *)(void *), 0x1001457f)
/* Not converted: (dword); only the low 16 bits of the result are used. */
#define W7L_CALL_1007cbcb SIM_FN(uint32_t(__cdecl *)(uint32_t), 0x1007cbcb)
/* Not converted: (x, y, z, limit as float bits, &found); the result and *found are tested. */
#define W7L_CALL_10033eb3 SIM_FN(int(__cdecl *)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t *), 0x10033eb3)
/* Not converted: (record, pointer). */
#define W7L_CALL_100c398d SIM_FN(void(__cdecl *)(void *, uint32_t), 0x100c398d)
/* Not converted: (key a, key b, fraction, out): writes the four floats of out from the two 16-byte keys. */
#define W7L_CALL_100608ef SIM_FN(void(__cdecl *)(const void *, const void *, float, float *), 0x100608ef)
/* Not converted: (dword); returns a pointer to a slot that holds an object pointer, or null. */
#define W7L_CALL_1001b902 SIM_FN(uint32_t *(__cdecl *)(uint32_t), 0x1001b902)
/* The routines 0x1001ee7c calls through a pointer (jump stubs of 0x1000e93e, 0x1001fe29 and 0x1009ebeb). */
#define W7L_FN_100029eb 0x100029ebu
#define W7L_FN_10001dfc 0x10001dfcu
#define W7L_FN_10003102 0x10003102u
/* HeapAlloc on the game's heap (Hg_HeapAlloc, src/sim/crt/hg_winheap.c). */
/*
 * Resets an object: 0x1001457f(obj); words +0x1a8, +0x1aa and dword +0x1ae = 0; word +0x1a6 = 0xc when its
 * dword +0x14 is 2, or 3 with +0xc equal to [0x1017b8c8], else 0; Sim_10016a6b(obj, slot, -1, 0) for slots
 * 0..2; the dword at the pointer +0x170 = 0; +0x198, +0x1a0 (words), +0x1be, +0x1c6 = 0; the three dwords
 * from +0x20c = -1; +0x208 and +0x218..+0x228 = 0; then Sim_10017852(obj) and Sim_100119a7(obj).
 */
void __cdecl Sim_1001150d(void *obj);

/*
 * Moves the float *value: q = *value / f[0x10111100] / f[0x10111118]; w = *value - the float +8 of the
 * pointer obj+0x54 times q; when |obj+0x5c - w| > f[0x1011111c] the result is obj+0x5c minus that step with
 * the sign of the difference (0 below f[0x10111120]); *value = w. Then obj+0x64 is moved by the step
 * f[0x10111128] (with the sign of its distance) towards sign(difference) * f[0x10111124] unless equal.
 */
void __cdecl Sim_100125d9(void *obj, float *value);

/*
 * Prints the record list on the mono screen (when [0x10a3ade8] is set): a title line at row 3, and for each
 * record PTRTAB(i), i < [0x1015dc64], a line (format 0x10131158) at row +4 of its dword +4 (when below 25),
 * marked '>' for the record [0x1015ef14], attribute 7, or 0xf when bit 4 of +0x4f0 of its block +0x28 is clear.
 */
void __cdecl Sim_10012970(void);

/*
 * Chooses a record index from the selector `sel` and `key` (see the code for the cases 0x2100, 0x2101,
 * 0x2200..0x2202, 0x4200, 0x4201 that search PTRTAB with Sim_10052c60/Sim_10052be7/Sim_10052aaf); a found
 * index without bits 8..11 gets those of `sel`. -1 when nothing fits. Only the low 16 bits of eax are a
 * result (every caller reads ax).
 */
int16_t __cdecl Sim_10015018(void *obj, int16_t sel, int16_t key);

/*
 * 0x1000 when 0x10033eb3(the point obj+0x58..0x60, 30000.0, &found) fails or finds nothing; else the word
 * obj+0x1aa when its state word +0x1a6 is 2 or 3 and it has bit 0x200; else the word +0xf0 of `a` when that
 * dword is not -1, has bit 0x200 and Sim_10052be7(its low byte) is set; else (index | 0x200) of the record
 * with Sim_10052be7 set that gives the smallest float obj+0xd8 after Sim_1001403a(obj, index | 0x200), with
 * the 0x28 bytes at obj+0xd4 saved and put back around the search. Only ax is a result (the caller reads ax).
 */
int16_t __cdecl Sim_10015f5b(void *a, void *obj);

/*
 * For obj with dword +0 == 1: every 0x20 ticks (0x40 while |the float +8 of obj+0x54| < f[0x10111120]) of
 * [0x1015dc78] (next time in obj+0x200) adds +-5.0 to the float +4 of obj+0x54 (sign by obj+0x218) and flips
 * obj+0x218 when its absolute value is above f[0x10111140].
 */
void __cdecl Sim_100178f4(void *obj);

/*
 * When c+8 is 1: for each used entry of the 0x200 entries of 0x156 bytes at 0x1096fc80 of kind 3, 9, 10 or 12
 * whose Sim_10052be7(+0x50) differs from Sim_10052be7(a+4), and whose point (Sim_100a9cfb of +0x3c) is not
 * farther than f[0x1011118c] from a+0x58.. (Sim_10084a94) while the dot product of its vector +0x18.. with
 * the offset is above f[0x10111190]: c+8 = 2, c+0x2c = b+0x2c, c+0x10/0x18/0x1c = 0 and 0x100c398d(c, a+0x28).
 * Returns 1.
 */
int __cdecl Sim_100196d5(void *a, void *b, void *c);

/*
 * Two-mode handler of a text "key;n,n,...". Mode 0: allocates a 0x50-byte record (HeapAlloc, flags 1) into the
 * slot Sim_100cc8ba(Sim_100cc6e0()), with up to 16 numbers (strtol << 4) from +0x10 (count +0xc), +4 = 0,
 * +8 = -1, and +0 = Sim_100b399c of Sim_100b38c5(Sim_100db770(atoi(text))); 0 when that fails. Mode 1:
 * with the record of the slot, +4 = the first dword of +0, +8 from Sim_100094df when -1, then
 * Sim_100096b2(+4, i, number[(n / div % count + i) % count]) for i < +8. Returns 1 (0 on the failures).
 */
int __cdecl Sim_10019da0(int mode, char *text, int n, int div);

/*
 * Key-frame lookup: keys = (count, then 0x14-byte keys of dword time + 16 bytes). t is taken modulo period;
 * a key at exactly t is copied to out (returns 0); otherwise 0x100608ef(previous key, next key, fraction, out)
 * with the keys wrapping around the period. Returns 1 when out is null or count <= 0, else 0.
 */
int __cdecl Sim_1001bb9d(int t, int period, const void *keys, float *out);

/*
 * With the object in the slot that 0x1001b902(a) returns: finds the three entries (0xc bytes from +0x1c,
 * count dword +0xc of +0x14) whose key (dword +4 of their first pointer) is [0x1015d190], [0x1015d194] and
 * [0x1015d19c]; when all have the same number of 0x14-byte keys, every key of the third becomes the
 * Quat_Multiply of the keys of the first two with x, y and z negated.
 */
void __cdecl Sim_1001cedf(uint32_t a);

/*
 * Sets the option `index` (0..4) to value: 0 -> [0x101321b4] (and calls 0x100029eb when it changed),
 * 1 -> [0x101321b8] with bit 2 of [0x101321c0] (0x10001dfc when changed), 2 -> [0x101321bc] (when changed:
 * 0x10003102 if Sim_1009eb24(), Sim_1001f26e if it was 0; always Sim_1009e441), 3 -> [0x101321b0]. Every
 * index except 2 first calls Sim_1001f335.
 * No CD (the music folder): index 2 is 1.2's WAV-map version (1.2 0x1001550d) on platform/music.h:
 * when changed, 0x10003102 (the volume) always and Sim_1001f26e if it was 0; then, when no music plays,
 * Sim_1001f26e (with track 2 when [0x101321a8] is -1), else Sim_1001f367 when paused.
 */
void __cdecl Sim_1001ee7c(int index, uint32_t value);

#endif
