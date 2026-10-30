/*
 * Round 17: the kill credit 0x10067ec6 and the unit-against-unit collision damage 0x1006788c
 * (DECISIONS.md, "Round 17").
 */
#ifndef HG_SIM_UNSORTED_1006_W17_H
#define HG_SIM_UNSORTED_1006_W17_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * The collision damage shot of record a (the damage block of the unit at +0) against b (0 for a world object): the
 * killer is -2 without b; with b, a kind-9 unit (unit +0) is handed to Sim_1006863d and the killer is b's id (unit +4)
 * when only a is kind 9, -2 when only b is; when neither or both are kind 9 the killer is `unsetKiller`. Then, by the
 * collision normal [0x1015dd58..60]: with [0x1015dd5c] below [0x1012b62c] (or NaN) part 1 is hit, above [0x1012b630]
 * parts 5 and 6 with the shot's factor +0x74 divided by [0x1012b604], else the angle of the normal
 * (Sim_1008486e) less the unit's +0x68 and +0x74, put into one turn, picks part 2, 3 or 4. Each hit is
 * Sim_1006c161(killer, a, 0, shot, part).
 *
 * unsetKiller: the original never writes its killer local on the "neither or both kind 9" path; it reads what the
 * last call at the same stack depth left there, which in its only caller with a b (0x1006788c) is always the
 * distance estimate 0x10084a94: W17_KillerLeftover of its x and z. The caller computes that and passes it in.
 */
void __cdecl Sim_10067ec6_Credit(uint8_t *a, uint8_t *b, uint8_t *shot, uint32_t unsetKiller);

/*
 * The hooked entry of 0x10067ec6 for callers through its address. Its only callers (0x1006788c and 0x100680dd, which
 * passes b = 0) are converted and call Sim_10067ec6_Credit; the unset killer is given as 0.
 */
void __cdecl Sim_10067ec6(uint8_t *a, uint8_t *b, uint8_t *shot);

/*
 * What the original 0x10084a94 (distance estimate of (x, y, z)) leaves in its local [ebp-4], the bits of
 * min(|x|, |z|): |x| when |x| < |z| or either is NaN, else |z|. 0x10067ec6 reads that slot as its unset killer. The
 * absolute values are the bits with the sign cleared (the original's fchs of a negative signalling NaN would also
 * quiet it on a real FPU; not modelled, as in Sim_10084a94).
 */
uint32_t W17_KillerLeftover(float x, float z);

/*
 * Collision damage between the damage blocks a and b (unit at +0) when the game's damage switch ([0x1017b8c4] byte 3)
 * is on: the speed difference (+0x554..+0x55c, a minus b) by Sim_10084a94, times [0x1012b614], picks the shot (damage
 * +0x70, factor +0x74, armour +0x78, kind +0x7c = 1, sound +0x86 = 0) from the thresholds [0x1012b618..28] (none at or
 * below [0x1012b618]); from the second level on each kind-1 unit not in state 5 (+0xbdb) that is local (no network
 * [0x1015ec58], the own unit, or a kind-3 one of the own player) and wins a Sim_1009c929 draw (range +0xbdb plus 0x14
 * for the own unit, else 0x50; halved on the fourth level; no draw on the fifth) gets the shake +0xc15 (2 with the
 * byte +0x59 of its unit's block +0x54 set, else 1). Then the kill credit for a hit by b, its unset killer being the
 * leftover of the distance call (W17_KillerLeftover of the x and z speed differences), exactly as in the original.
 */
void __cdecl Sim_1006788c(uint8_t *a, uint8_t *b);

/*
 * Collision damage of the damage block a (unit at +0) against the world object obj (0 for the ground), when
 * [0x10179ce0] is set. An object of kind 0xb0 (obj +2 & 0xf0) is hit instead: the counter word [0x101654f4] + 1,
 * the fixed shot 0x101450a8 on parts 5 and 6 (Sim_1006c161, killer -2), the effect Sim_10024616 (0x24 or 0x16 by bit
 * 1 / 2 of the dword +2 of its entry 0x109e9ba0 + 0x50 * obj word +0x24, at obj +0x44..) and Sim_1002597b of that
 * index. Otherwise a's own speed (+0x554..) by Sim_10084a94 times [0x1012b614] picks the shot as in Sim_1006788c, only
 * a shakes (the third level draws from +0xbdb + 10), the kill credit runs with no second unit (killer -2) when the
 * damage switch ([0x1017b8c4] byte 3) is on, and an object with flag 0x200 gets Sim_10025aee(a's id, obj, the shot's
 * damage +0x70 as a float, obj +0x44..).
 *
 * Deviation (DECISIONS.md, round 17): the original never sets the shot's kind word +0x7c; it holds what the slide
 * test before it left on the stack, never 1..4 except after the own unit's crash sound. The C uses 0, which
 * Sim_1006c161 treats like every value other than 1..4.
 */
void __cdecl Sim_100680dd(uint8_t *a, uint8_t *obj);

#endif
