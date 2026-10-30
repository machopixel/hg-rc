/*
 * Wave 2e, original 0x100c4699 and 0x100ca417: users of the pointer table at
 * 0x109e8b40 (count [0x1015dc64]) and of the 0x54-byte table at 0x109a8640
 * (count [0x1017e5a8]).
 */
#ifndef HG_SIM_UNSORTED_100C_W2E_H
#define HG_SIM_UNSORTED_100C_W2E_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * With obj non-null: clears the 0x64 bytes at 0x1017e240 and appends dword +4 of every
 * record of the pointer table whose ushort +0x1a8 has bit 0x200 and low byte equal to obj's
 * dword +4 (no check on the 25 entries that fit); stores 0x1017e240 in *out and returns how
 * many were found. With obj null: *out = 0, returns 0.
 */
int __cdecl Sim_100c4699(void *obj, uint32_t *out);
/*
 * Removes entry b & 0xff of the 0x54-byte table (b needs bit 0x100; entry +0x24 bit 0 set and
 * dword +8 == (a | 0x200)) by moving the later entries down and decrementing the count, then
 * lowers the counters in the records of the pointer table that referred to later entries:
 * the low byte of the ushorts +0x1a8 and +0x1aa, and of the dwords +0xf0, +0x118 (not for
 * records with dword +4 == [0x1017b8c8]) and +0x1d6, when their bit 0x100 is set and they are
 * above the removed index. Returns the new count, or -1 when b has no bit 0x100, the index is
 * out of range or the entry does not match.
 */
int __cdecl Sim_100ca417(uint32_t a, uint32_t b);

#endif
