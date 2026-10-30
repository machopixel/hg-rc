/*
 * Round 14: game functions of HGSIM.DLL around 0x10070000..0x1007ffff that were left original for a stack overflow,
 * settled by the shipped data (DECISIONS.md, "Round 14").
 */
#ifndef HG_SIM_UNSORTED_1007_W14_H
#define HG_SIM_UNSORTED_1007_W14_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Per-frame update of a gear's record rec (unit rec[0], its control block unit +0x54), unless word +0x560 has bit
 * 0x200. Running (+0x4f0 == 2): Sim_100d4590 and, for the player with the block bytes +0x2c / +0x28, Sim_100d63d4;
 * a pending kill [0x1014b930] / [0x1014b934] of the player's unit (Sim_1006b52f); the AI (Sim_10011681 for sides 2
 * and owned 3, else Sim_1001349a when [0x10a3ade8]). For a running unit the player controls (or any in single
 * player): the control requests of the block (bytes +0x31..+0x4c: Sim_100d6392, Sim_100cbdb8, Sim_100ca670,
 * Sim_100cbe94, Sim_100cbe3c, Sim_100cbe68, Sim_100cbdea, Sim_100cbe13, Sim_100cbaf8, each cleared after use),
 * Sim_100cb4fb, Sim_100d6833, the stop / go toggle +0x50c with its sounds, the throttle +0xc / +0x10 and the
 * engine rate +0x78 (with the boost and gear requests +0x50..+0x53 of the player). Then Sim_1004f257, the jump jets
 * (+0x510 / +0x514 / +0x518, sounds AFXHGJJT / AFXHGJJF / Sim_1000ecb0), the forward speed +0x38 from +0x2c and the
 * block's throttle through Sim_10084766 (and the lean +0x48 for Sim_10079feb kinds), the auto-turn of the player
 * ([0x1015f1e0]: heading [0x10179cdc], turn rate +0x38, +8 / +0xc), the speed +0x28 from +0x7c (kind 5 adds the
 * bank +0x58 from Sim_10084a94; the MASC overheat +0x88 bit 2 with "MASC malfunction." [0x101c4648]), the steering
 * +8 limited to +0x534, the turn +0x18 (with the player's offset [0x10179cec]) and the heat +0x4e0. A unit that is
 * not running is stopped (Sim_1004f158, Sim_100d4500 for the player's). Then the state step of +0x4f0 (0 start-up
 * with Sim_1009c929(0x16a) and Sim_10052be7, 1 ready, 2 / 3 the shut-down flag 0x10 of word +0x1c, 4 destroyed
 * with Sim_10061c32 / Sim_1007cbb4, 7 Sim_1006c9a0 when its timer +0x4d8 ran out). A unit not the player's clears
 * bit 0 of word +0x1c.
 */
void __cdecl Sim_10070eb5(uint8_t *rec);

#endif
