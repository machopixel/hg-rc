/*
 * Round 3 functions of HGSIM.DLL around 0x10070000..0x10080000 (letter p).
 *
 * Both functions forward to the "dpSend" export of the network DLL, whose
 * address the DLL looks up with GetProcAddress and keeps in the dword at
 * 0x1015eab4 (the string "dpSend" is at 0x1013a838). The converted code calls
 * it through that dword like the original does.
 */
#ifndef HG_SIM_UNSORTED_1007_W3P_H
#define HG_SIM_UNSORTED_1007_W3P_H

#include <stdint.h>

#include "../simmem.h"

/*
 * dpSend(session [0x1015ecd4], hi(len) | word [0x1013acac], 0, flag, buf, len); returns its result.
 * hi(x) is the upper 16 bits of x: the original builds the second argument with
 * "mov ax, ..." in a register that still holds len, and the callee takes a word.
 */
int __cdecl Net_DpSend_1007e920(void *buf, int len, int flag);

/*
 * dpSend(session, hi(buf) | word [0x1013acac], hi(len) | low word of to, flag, buf, len); returns its
 * result. Same upper-half leftovers as above (here of buf and len).
 */
int __cdecl Net_DpSend_1007e950(uint32_t to, void *buf, int len, int flag);

#endif
