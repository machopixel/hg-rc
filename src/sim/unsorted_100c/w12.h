/*
 * Round 12: game functions of HGSIM.DLL around 0x100c0000..0x100cffff that read uninitialised stack in the original;
 * converted with the start values recorded in DECISIONS.md ("Round 12: uninitialised stack").
 */
#ifndef HG_SIM_UNSORTED_100C_W12_H
#define HG_SIM_UNSORTED_100C_W12_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
/*
 * Contact test of a moving unit (record +0x28, motion block +0x54). Returns 0 when the record's +0x4d4 is 0, when
 * the block's byte +0x59 is set, or when the word +0x25e (contact side) is 0 and the block's +8 is 0; the stored
 * result (+0x24a != 0) when +0x25a > [0x1015dc78]. Without a side, +0x260 = |velocity +0x548| / 7.7 scaled by the
 * unit type (1, 9: / 3; 4: / 2; 10: * 1.1), at least 0.3. Up to four probes (Sim_100c31a6) look for something hit
 * (Sim_1002dda9 / Sim_100c2851, else Sim_100c292c for units Sim_10079feb rejects); each hit not excluded by
 * Sim_100c312c sets the side (Sim_100c3419 / Sim_100c3372) and adds side * 819.2 / 4 to the steering sum; a miss
 * clears the side when |Sim_10015266| <= 1 on the first probe and tries the opposite side once (adding side * 409.6).
 * A non-zero sum (clamped to +-819.2) goes to block +0xc with a weight to +8; otherwise type-8 units cool +0x242.
 * Ends with Sim_100c275b; returns 1 when the sum is non-zero. The thing hit starts at 0 (the original reads an
 * uninitialised local when only Sim_100c292c reports a hit, DECISIONS.md).
 */
int __cdecl Sim_100c2ae5(uint8_t *unit);

/*
 * The target readout pane: name / status text of the player's target (kind 0x100 / 0x200 / 0x400 by the target word
 * +0xf0 of the player's record), its range ("%2.2fk" above 1000, else "%3ldm") in a colour by its side, and, when the
 * target projects on screen, a box around it with the range text beside it (lines with 0x100ef727 / 0x100e86a1);
 * off screen the text goes to the fixed corner. The pane's method +0x114 sets its texts. For a target kind other
 * than 0x200 / 0x400, and for 0x400 without its object, the position, size and rank start at 0 (the original uses
 * uninitialised locals there, DECISIONS.md).
 */
void __cdecl Sim_100c7120(uint8_t *pane);

#endif

/*
 * The position of a selected thing: index sel & 0xff, kind sel & 0xf00. Kind 0x200: *out = unit +0x68, the node is
 * unit +0x48 (PTRTAB). Kind 0x400: the node is Sim_100b5386 of the dword 0x109e9ba8 + index * 0x50; without one,
 * Sim_100b53f0(that dword, x, out, y), *out = 0 and *x / *y grow by the offsets of part (below), done. Otherwise
 * *x / *y = the offsets of part (floats 0x101516f8 / 0x101516fc + part * 8: 0 when equal to [0x1012bb18] or NaN,
 * scale when 1.0, else scale / it), then Mat3x4_TransformPoint(Sim_100aa863(node), x, &zero, y). Any other kind
 * uses node 0 (the original passes the uninitialised local, DECISIONS.md). scale is a float, taken as its bits.
 */
void __cdecl Sim_100c0158(uint32_t sel, int16_t part, float *x, float *y, uint32_t *out, uint32_t scale);

/*
 * Contact test of a moving unit (record +0x28, motion block +0x54). Returns 0 when the record's +0x4d4 is 0, when
 * the block's byte +0x59 is set, or when the word +0x25e (contact side) is 0 and the block's +8 is 0; the stored
 * result (+0x24a != 0) when +0x25a > [0x1015dc78]. Without a side, +0x260 = |velocity +0x548| / 7.7 scaled by the
 * unit type (1, 9: / 3; 4: / 2; 10: * 1.1), at least 0.3. Up to four probes (Sim_100c31a6) look for something hit
 * (Sim_1002dda9 / Sim_100c2851, else Sim_100c292c for units Sim_10079feb rejects); each hit not excluded by
 * Sim_100c312c sets the side (Sim_100c3419 / Sim_100c3372) and adds side * 819.2 / 4 to the steering sum; a miss
 * clears the side when |Sim_10015266| <= 1 on the first probe and tries the opposite side once (adding side * 409.6).
 * A non-zero sum (clamped to +-819.2) goes to block +0xc with a weight to +8; otherwise type-8 units cool +0x242.
 * Ends with Sim_100c275b; returns 1 when the sum is non-zero. The thing hit starts at 0 (the original reads an
 * uninitialised local when only Sim_100c292c reports a hit, DECISIONS.md).
 */
int __cdecl Sim_100c2ae5(uint8_t *unit);

/*
 * The target readout pane: name / status text of the player's target (kind 0x100 / 0x200 / 0x400 by the target word
 * +0xf0 of the player's record), its range ("%2.2fk" above 1000, else "%3ldm") in a colour by its side, and, when the
 * target projects on screen, a box around it with the range text beside it (lines with 0x100ef727 / 0x100e86a1);
 * off screen the text goes to the fixed corner. The pane's method +0x114 sets its texts. For a target kind other
 * than 0x200 / 0x400, and for 0x400 without its object, the position, size and rank start at 0 (the original uses
 * uninitialised locals there, DECISIONS.md).
 */
void __cdecl Sim_100c7120(uint8_t *pane);

#endif