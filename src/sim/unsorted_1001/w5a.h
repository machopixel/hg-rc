/*
 * Round 5, letter a: functions of 0x1001042f..0x1001f3ac that call helpers which are not converted yet.
 * Those helpers are called through their address (SIM_FN) with the prototypes below.
 */
#ifndef HG_SIM_UNSORTED_1001_W5A_H
#define HG_SIM_UNSORTED_1001_W5A_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Not converted: (index, code). */
#define W5A_CALL_10016117 SIM_FN(int(__cdecl *)(int, uint32_t), 0x10016117)
/* Not converted: (object, key, -3, out word, out word). The outputs are 16-bit values. */
#define W5A_CALL_100135d4 SIM_FN(int(__cdecl *)(void *, uint32_t, uint32_t, int16_t *, int16_t *), 0x100135d4)
/* Not converted: (index, out, out, out) with dword outputs; returns 0 when the index is out of range. */
#define W5A_CALL_100529a2 SIM_FN(int(__cdecl *)(uint32_t, uint32_t *, uint32_t *, uint32_t *), 0x100529a2)
/* Not converted: no arguments. */

/*
 * The three functions below do the same with different constants. For an index below 8 whose record
 * (pointer table 0x109e8b40, found with Sim_10015c61) has the value 2 at (record + 0x28) + 0x4f0:
 * dword table 0x1012ff58 [index] = S, then 0x10016117(index, M), M being a code. The second argument is not used.
 */
/* S = 2, M = 0xd. */
void __cdecl Sim_1001042f(int index, int unused);
/* S = 2, M = 2. */
void __cdecl Sim_10010564(int index, int unused);
/* S = 4, M = 7. */
void __cdecl Sim_10010699(int index, int unused);

/*
 * Without any of the bits 0xf000 in key: key as an unsigned 16-bit value. Otherwise calls
 * 0x100135d4(obj, key, -3, &a, &b) (two 16-bit outputs) and returns a, zero extended, when the word at
 * obj + 0x1a6 is 2 and the dword at obj + 0x1dc is not zero, otherwise b, sign extended.
 */
int __cdecl Sim_10012a72(void *obj, int16_t key);
/*
 * After Sim_1001570f(obj): asks 0x100529a2(obj[4]) for three values and, when that succeeds, looks the
 * record up with Sim_100ca350(obj[4], a, b, c). When one is found (not -1): sets bit 0 in the word
 * at 0x109a8664 + 0x54 * index, stores obj[4] | 0x200 in the dword at 0x109a8648 + 0x54 * index and
 * puts index | 0x100 into the word obj + 0x1a8.
 */
void __cdecl Sim_10015a63(void *obj);
/*
 * Adds [0x1015dc80] to [0x1015d1c4]. At 0x16a (signed) or more: clears it and, when [0x1015d1bc] is 0,
 * bit 3 of [0x101321c0] is set, 0x1009e5d0() is not 0 and the track [0x101321a8] is not -1:
 * Cd_PlayTrack(track), Cd_UpdateStatus().
 * No CD (the music folder, DECISIONS.md "dropped features"): 1.2's WAV music poll (1.2 0x10015fbd) on platform/music.h: at the same
 * moments, with a track and bit 3, Music_Play(track) when Music_IsPlaying() is 0.
 */
void __cdecl Sim_1001f3ac(void);

#endif
