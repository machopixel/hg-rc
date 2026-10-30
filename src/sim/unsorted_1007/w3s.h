/*
 * Wave 3S, original 0x10075371..0x1007e984. Plain C, original cdecl convention.
 */
#ifndef HG_SIM_UNSORTED_1007_W3S_H
#define HG_SIM_UNSORTED_1007_W3S_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Turns the pending key (signed word 0x1015fd4c) or the held inputs (bytes 0x1015fde6 / 0x1015fde7,
 * floats 0x1015fdd8 / 0x1015fddc against -8192.0 and 8192.0, timer [0x10149948]) into a command code in
 * [0x1015dc18] (0 when none); the flags are bit 0 of the dword at +4 of the node Sim_10076689 returns
 * (nothing happens without a node). A key that became the code is consumed. With a code and the flag
 * the timer is reset and the bytes 0x1015fde2 and 0x1015fde5 are set.
 */
void __cdecl Sim_10075371(void);
/* Calls method +0x11c of each of the objects at 0x109e8928, 0x109e8938, 0x109e890c, 0x109e88e8, 0x109e894c, 0x109e8950, 0x109e8954 as method(object, 0). */
void __cdecl Sim_100780cf(void);
/*
 * Sets *out = 0x100 and calls the function at [0x1015eab8] (filled by GetProcAddress("dpReceive") at
 * 0x1003bf3e) up to three times as f([0x1015ecd4], &a, &b, [0x10372d98], [0x1015d008], out). When a call
 * returns 0 gives the 16-bit value it stored in a. After three failures sets *out = 0 and gives 0xfa01 with
 * the upper half of the last return value in the upper half of eax (the callers store ax only).
 */
uint32_t __cdecl Sim_1007e984(uint32_t *out);

#endif
