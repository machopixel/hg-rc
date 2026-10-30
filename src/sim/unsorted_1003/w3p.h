/*
 * Round 3 functions of HGSIM.DLL in 0x1003xxxx (letter p). Names carry the
 * original address because the code alone does not say what the fields mean.
 * The network ones fill a message buffer and send it with Net_DpSend_*
 * (unsorted_1007/w3p.h).
 */
#ifndef HG_SIM_UNSORTED_1003_W3P_H
#define HG_SIM_UNSORTED_1003_W3P_H

#include <stdint.h>

#include "../simmem.h"

/*
 * Picks, among the 7 pointers at 0x1013a2a8, the one whose dwords +8 and +0xc are both non-zero and whose
 * unsigned dword +0xc is smallest (first one on ties). When it differs from the current one (dword
 * 0x1015e68c) it calls the current one's function at +0x14 (no arguments), makes the pick current and
 * calls its function at +0x10 with (dword 0x1015e698, dword 0x109e3d14, dword 0x109e3d18). Then copies the
 * current pointer to 0x1015e690, sets dword 0x10179d68 to 1 and calls Sim_10083495(0x1013a340, *cur, cur[3])
 * (an empty function in this build).
 */
void __cdecl Sim_10037e8f(void);

/* Divides the float at +0x4ec of the block (+0x28) of each of the [0x1015dc64] pointer-table entries by the float at 0x101114fc. */
void __cdecl Sim_1003ccc4(void);

/*
 * Chat-style text message: 0 when the string is 0x50 characters or longer, else 1 after sending the 2-byte
 * header at 0x1013ad4c and the string through the buffer at [0x1015ecc8]. target -4 sends nothing, -1
 * sends to everybody (Net_DpSend_1007e920), -3 and -2 to each player of the list in the record at
 * [0x1015d004] whose table entry is in state 1 and whose Sim_10052be7 is 1 (-3) or 0 and which is not
 * the local index [0x1017b8c8] (-2), any other value to that table entry when its state is 1.
 */
int __cdecl Sim_1003fcdc(int target, const char *text);

/*
 * Message of table entry `index` with a 17-entry list of nibble levels computed from floats of the block
 * (+0x28 -> +0x88, 0x3c bytes each); sent when a level differs from the byte in the table at
 * 0x1015da45 + 0x16 * entry[4], which is updated.
 */
void __cdecl Sim_1003ffe9(int index);

#endif
