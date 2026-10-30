/*
 * Round 10: game functions of HGSIM.DLL around 0x10010000..0x1001ffff, converted one at a time.
 * Calls of functions that are not converted yet go through their original address (SIM_FN).
 */
#ifndef HG_SIM_UNSORTED_1001_W10_H
#define HG_SIM_UNSORTED_1001_W10_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Returns (in st(0)) the float at (obj+0x28)->+0x53c times a factor chosen by obj's kind (dword +0):
 * 3: [0x101110fc], 5 and 8: [0x10111100], 10: [0x101110f8], any other: [0x10111104]; rounded to float. */
float __cdecl Sim_10010fbf(const void *obj);

/* Animation step of obj (node +0x48, record +0x28) at time t: for every bone of anim (count (anim+0x14)->+0xc,
 * entries of 0xc bytes from +0x1c) the node found by name (Sim_100aadaa) gets the key-framed rotation (Sim_1001bb9d,
 * the identity 0x10131740 when there is none), blended with the second animation of state (+0x10, time +4) by
 * blend / 10 (the original slerp 0x100608ef) when +0x18 is set, by the bone's kind (Sim_100aa227 / Sim_100aa1de /
 * Sim_100aa190); then the event of the time (Sim_1001c075) sets state+0x2c, flag 8 of obj+0x88, record+0x564 and
 * bit 0x1000 of record+0x562, and for kinds 1 and 9 the node is lowered onto the nearer of the two ground heights
 * (state+0x34, Sim_100a9d7c). */
void __cdecl Sim_1001c1ab(uint8_t *obj, int32_t t, uint8_t *anim, int32_t blend, uint8_t *state);

/* Sets the AI order of obj (+0x1a6 = order): leaves the hold state (+0x1c bit 0x10, record+0x4f0), keeps only bits 1|2 of
 * +0x1ae for the own side (not for order 3), then per order: 0/10/11 stop (+0xd4, control +8; 10/11 hold), 2 a new zone
 * claim (Sim_100ca350, zone table 0x109a8640) on a timer, 3 throttle off, 4 / 5 / 7 the order helpers, 8 the side's
 * squad check, 13 a flanking point beside the target (Sim_10084cee, Sim_100ca350 -> +0x1a8), 14 slow, 15 the path to the
 * target (Sim_100c4035). Ends with the AI log line through Crt_SPrintf / Sim_1005f9aa. */
void __cdecl Sim_1001484e(uint8_t *obj, uint32_t order);

/* The AI table of the debug monitor screen (shown while [0x10a3ade8] is set): a header (Mono_PutText) and for each unit a
 * line of side, squad, order, aim, objective, speed, throttle, state, hit, turn and name (Crt_SPrintf) at row id + 4. Each
 * unit is first turned to its wanted aim (Sim_1001403a; +0xf0 for the own unit, else +0x1a8), its block +0xd4..+0xfb put
 * back afterwards; a speed below [0x10111110] stops in the debugger (int 3). Units without an aim or a target are
 * logged (Sim_1005f9aa, 10 times at most: 0x1015d10c). */
void __cdecl Sim_10011c28(void);

/* Animation driver for every unit of kind 1, 9 or 5 (not the models named V10..): other units' animations only run while
 * they are on screen; a new animation id (+0x90, record by Anim_RecordOfId) is taken over (+0x8c) or blended in over
 * 10 steps (state 0x10a1d140 + i * 0x38), then Sim_1001c1ab steps the frame by the frame time and the animation's speed
 * (+0x94 1 / 3 scale it), with the end rules of the animation kind (+0x8c: hold at half, the fall flags of record
 * +0x562 / +0x560, back to the previous animation +0x1c, or loop). */
void __cdecl Sim_1001c693(void);

#endif
