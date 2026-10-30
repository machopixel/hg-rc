/*
 * Round 4, letter w (area 0x10008xxx): allocation of the blocks that hang in the list at dword +0x2c of an owner object.
 */
#ifndef HG_SIM_UNSORTED_1000_W4W_H
#define HG_SIM_UNSORTED_1000_W4W_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Allocates a block of 0x18 + n*0x58 + m*0x28 + extra bytes (m = 1 when 0) from the game's heap and clears it. The block
 * has the float `key` (passed as its raw bits) at +0, the dwords/words 0 at +4, +6, +0x10, +0x14, the size of its header part
 * ((n*0x58 + 0x18) & 0xffff) at +8, and *end = block + header size + m*0x28. The block is stored in owner+0x30
 * (Sim_10008c69) and put into the owner's list at owner+0x2c, ordered by ascending key (next pointer at +0xc, a new
 * block goes after equal keys). Returns the block, or 0 when owner is 0 or the allocation fails.
 */
void *__cdecl Sim_10008cbc(void *owner, uint32_t key, int n, int m, uint32_t extra, uint32_t *end);

/*
 * Allocates a 0xb4 byte record, sets its word +0 to the word of 0x1015d09c with bit 15 set, fills it through
 * Sim_10008cbc(record, 0, a, b, c, d) and clears its fields (+0x34 = 4). Frees the record and returns 0 when that fails.
 */
void *__cdecl Sim_10008f1c(int a, int b, uint32_t c, uint32_t *d);

#endif
