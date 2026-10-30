/*
 * Wave 2l: Sim_10062780, a per-step update of ten animated pointer-table
 * entries. Plain C, original cdecl convention.
 */
#ifndef HG_SIM_UNSORTED_1006_W2L_H
#define HG_SIM_UNSORTED_1006_W2L_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Ten 0x18-byte slots at 0x10165be8 (timer +0, frame +4, active +0x10, restart +0x14), each tied to a pair of
 * dwords at 0x10144ed8 + 8 * slot: a target index into the pointer table 0x109e8480 and a base index into
 * the same table. An active slot's timer drops by the dword 0x1015dc80; while it stays above 0 the frame
 * advances modulo 3 and table[target] = table[base + frame]. At 0 or below the slot becomes inactive and
 * table[target] points to the 16-byte entry `target` of the block 0x101362e0 + 0x770 * (k + 2 * [0x101c64f0]),
 * k being the dword at +0x613 of the object at +0x28 of the entry of the pointer table 0x109e8b40 selected
 * by [0x1017b8c8]. An inactive slot with its restart flag set takes a timer of 90 * (Sim_1009c929(3) + 1),
 * becomes active and sets table[target] = table[base + frame]; the restart flag is cleared by both.
 */
void __cdecl Sim_10062780(void);

#endif
