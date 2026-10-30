/* Round 4 (letter u), originals 0x1005a138..0x1005b14e: freeing and loading heap records. */
#ifndef HG_SIM_UNSORTED_1005_W4U_H
#define HG_SIM_UNSORTED_1005_W4U_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Frees a record and its three blocks (pointers at +0x88, +0xc8 and +0xd0, each only when not 0). While it does so
 * the game heap handle 0x1017b8f8 is replaced by the handle 0x1017b910 when that is not 0, and afterwards by the
 * handle 0x1017b900 (whatever it was before).
 */
void __cdecl Sim_1005a138(void *record);
/*
 * Builds a record from a serialized image: allocates 0x1a1 bytes, copies 0x12c bytes of the image to +9, then, when the
 * (unaligned) dword at +0x12d is a count n, allocates and fills n * 0x48 bytes for the pointer at +0x131 (else that pointer
 * is 0) and finally copies 0x6c bytes to +0x135. Returns the record.
 */
void *__cdecl Sim_1005b026(const uint8_t *image);
/* Frees the block at +0x131 (when not 0), then the record. */
void __cdecl Sim_1005b112(void *record);
/* Sim_1005b026(image); stores the record's serialized size, count * 0x48 + 0x1a1, in *size (0 when no record), and returns the record. */
void *__cdecl Sim_1005b14e(const uint8_t *image, uint32_t *size);

#endif
