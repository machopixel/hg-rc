/*
 * Round 9, letter a: the text tables filled from the UI text pointers that Text_InitStrings
 * (unsorted_100a/w9a.h) resolves. Plain C, original cdecl convention.
 */
#ifndef HG_SIM_UNSORTED_1007_W9A_H
#define HG_SIM_UNSORTED_1007_W9A_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Copies (strcpy, Str_Copy) the 46 texts [0x101c4e38], [0x101c4dfc], ... [0x101c4edc] (the weapon
 * and ammunition names, e.g. [0x101c4b44] "LRP16", [0x101c4b74] "MRP18") into the 0xd6-byte name
 * records at 0x10145592 + 0xd6 * i, in the order of the original. Called by Text_InitStrings.
 */
void __cdecl Sim_10074710(void);

#endif
