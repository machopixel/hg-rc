/*
 * Round 4, letter w (area 0x10074xxx..0x1007fxxx): a list of 0x18 byte entries, and the display window functions that
 * send WM_COPYDATA (0x4a) messages to the window 0x1017b35c or draw the pixel buffer 0x1015e9a0 into the window.
 */
#ifndef HG_SIM_UNSORTED_1007_W4W_H
#define HG_SIM_UNSORTED_1007_W4W_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Appends a new zeroed 0x18 byte entry holding `value` at +0 to the list with head dword 0x1015dc14, tail dword
 * 0x1015dc10 and next pointer at +0x14. Returns 1, or 0 when the allocation fails.
 */
int __cdecl Sim_10074b80(uint32_t value);




#endif
