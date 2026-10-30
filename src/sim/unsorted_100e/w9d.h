/*
 * Round 9, letter d: the packed 8-bit sample block decoder of HGSIM.DLL at 0x100e41cc (with its two register-
 * argument helpers 0x100e440b and 0x100e4463 as static functions) and the wrapper 0x100e3c6a.
 */
#ifndef HG_SIM_UNSORTED_100E_W9D_H
#define HG_SIM_UNSORTED_100E_W9D_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* ---- w9d_decode.c ---- */
/*
 * Decodes `count` blocks of `src` into `dst` (`len` bytes each); *state carries the last value (signed, -127..127)
 * from block to block. Each block starts with a header byte: bits 6..7 pick the rate (1: len / 2 values expanded
 * twice, 2: len / 4 values expanded four times, by linear interpolation of neighbours), bits 0..3 the coding:
 *  0 all 0x80 and state 0;  1 the previous block's bytes again;
 *  2 two delta entries (byte * 2 - 0x80), then 1-bit codes, 8 per byte, low bit first;
 *  3 four entries, 2-bit codes (4 per byte);  4 sixteen entries, 4-bit codes (2 per byte);
 *  5 the values as they are (the state becomes the last byte - 0x80).
 * Every decoded value is state + entry clamped to -127..127, stored + 0x80. Other codings end the decoding and
 * give NULL. The block is built in the DLL's buffer at 0x10152b94 (deltas at 0x10152f95, expansion scratch at
 * 0x10152794) and copied to dst. Returns the source pointer after the last block.
 */
uint8_t *__cdecl Sim_100e41cc(uint8_t *src, uint8_t *dst, uint32_t count, uint32_t len, int32_t *state);
/* Sim_10009741(obj). */
void __cdecl Sim_100e3c6a(void *obj);

#endif
