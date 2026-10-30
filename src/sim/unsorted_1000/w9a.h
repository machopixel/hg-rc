/*
 * Round 9, letter a: the text tables filled from the UI text pointers that Text_InitStrings
 * (unsorted_100a/w9a.h) resolves. Plain C, original cdecl convention.
 */
#ifndef HG_SIM_UNSORTED_1000_W9A_H
#define HG_SIM_UNSORTED_1000_W9A_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Copies 134 text pointers of the 0x101c4558..0x101c5150 block (the formation, order, status and
 * squad-channel texts, e.g. [0x101c5044] "Echelon Left", [0x101c4790] "Attack My Target",
 * [0x101c5150] "Status:", [0x101c4b98] "NO SQUAD MATE") into fixed dwords of the records at
 * 0x1012df80..0x1012ef10, and stores the addresses of the three text buffers 0x101c4a40,
 * 0x101c5180 and 0x101c4860 at 0x1012e51c, 0x1012e544 and 0x1012e56c. Called by Text_InitStrings.
 */
void __cdecl Sim_1000f560(void);

#endif
