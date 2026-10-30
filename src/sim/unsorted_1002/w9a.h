/*
 * Round 9, letter a: the text tables filled from the UI text pointers that Text_InitStrings
 * (unsorted_100a/w9a.h) resolves. Plain C, original cdecl convention.
 */
#ifndef HG_SIM_UNSORTED_1002_W9A_H
#define HG_SIM_UNSORTED_1002_W9A_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Copies 316 text pointers of the 0x101c455c..0x101c5170 block (the squad-mate replies, e.g.
 * [0x101c4fe8] "Reeves here!", [0x101c46c8] "Henry reporting", up to the mission-status words
 * [0x101c4e14] "jump jets") into the first dword of the 24-byte records at 0x10132400..0x10134248.
 * Called by Text_InitStrings.
 */
void __cdecl Sim_100204c4(void);

#endif
