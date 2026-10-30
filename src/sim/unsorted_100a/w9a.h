/*
 * Round 9, letter a: Text_InitStrings (0x100ae7c0), the initialisation of every UI text pointer of
 * the DLL. Plain C, original cdecl convention. Everything it calls is converted and called by name.
 */
#ifndef HG_SIM_UNSORTED_100A_W9A_H
#define HG_SIM_UNSORTED_100A_W9A_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Resolves 630 text literals of the DLL ("NO MESSAGE", "Mission successful", the squad replies,
 * the weapon names, ...) through the text lookup table [0x1017d594] (Sim_100b7834: the table's value
 * for the literal, else the literal itself) and stores the results in the text pointer globals at
 * 0x101c42a8..0x101c5290; three results are copied (Str_Copy) into the buffers 0x101c4a40,
 * 0x101c5180 and 0x101c4860, and 30 globals get the address of a literal without a lookup (the
 * axis and button names "Down/Up", "HatUp", "Mouse", ...). Then it fills the tables that hold copies
 * of these pointers: Sim_100204c4, Sim_1007dc1f, Sim_100d1cce, Sim_100b3547, Sim_10098a80,
 * Sim_1005fa30, Sim_1000f560, Sim_1009c710 and Sim_10074710, in that order.
 */
void __cdecl Text_InitStrings(void);

#endif
