/*
 * Round 4, letter v: three functions of HGSIM.DLL around 0x1008c500 that keep a palette fade (3 bytes per colour):
 * a record with the source pointer, a work copy, the colour range and the step counter.
 */
#ifndef HG_SIM_UNSORTED_1008_W4V_H
#define HG_SIM_UNSORTED_1008_W4V_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Copies count*3 bytes of src into a new heap block and fills the record: [0] = src, [4] = block, byte [8] = first
 * (low byte), dword at [9] = count. Returns 1, 0 when the allocation fails.
 */
int __cdecl Sim_1008c550(void *record, const uint8_t *src, uint32_t first, int32_t count);
/*
 * Allocates count*12 bytes and splits them: [0] = block (count*3 bytes: the colours of `from` starting at colour
 * first), [4] = block + 0x300 (bytes `to - from`), [8] = block + 0x600 (a word of 0 per byte); [0xc] = steps,
 * [0x10] = 0, byte [0x14] = first (low byte), dword at [0x15] = count. Returns 1, -1 when the allocation fails.
 */
int __cdecl Sim_1008c6f5(void *record, const uint8_t *from, const uint8_t *to, uint32_t first, int32_t count,
                         uint32_t steps);
/*
 * One step of the fade of such a record (only while [0x10] < [0xc]): adds the byte deltas ([4]) to the word
 * accumulators ([8]), adds accumulator / steps to the colour bytes ([0] + byte [0x14]) and keeps the remainder,
 * hands the block to the display driver's method +0x18 ([0x1015e688]), counts the step, and when the last one is
 * done frees the block and clears [0], [4] and [8].
 */
void __cdecl Sim_1008c803(void *record);

#endif
