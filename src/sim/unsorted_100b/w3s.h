/*
 * Wave 3S, original 0x100bd76b..0x100beee4. Plain C, original cdecl convention.
 */
#ifndef HG_SIM_UNSORTED_100B_W3S_H
#define HG_SIM_UNSORTED_100B_W3S_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * arr = { int capacity; int count; uint32_t *items }. Inserts item (non-zero) when count < capacity and returns 1,
 * else 0. Without compare it is appended; with it the item goes before the first existing item for which
 * compare(existing, item) is above 0 (at the end if none), the later items moving up by one.
 */
int __cdecl PtrArray_Insert(void *arr, uint32_t item, int(__cdecl *compare)(uint32_t, uint32_t));
/*
 * 16-bit result (the callers store ax only; the upper half of eax is whatever the original had there, so the
 * check compares the sign-extended low word): -1 unless the dword obj+0 is 1; then 10 when Sim_100c3eb5(obj+0x28)
 * is non-zero, else 0 without a partner object (word obj+0x1aa bit 0x200 selects pointer-table entry
 * low byte of it), or when Sim_10079feb(partner[0]) is set, or when partner[0] == 1 and bit 0x20 of the word +0x560
 * of the partner's +0x28 is set; otherwise, while the float obj+0xd4 is not above 1000.0 [0x1012bb10]: 0x15 when
 * Sim_1009c929(0x13) == 3, else 0x14 when Sim_100beea2(obj+0x28) is non-zero, else -1.
 */
int16_t __cdecl Sim_100beee4(void *obj);

#endif
