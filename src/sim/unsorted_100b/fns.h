/*
 * Wave 2: functions around 0x100b3530..0x100bf026 that call converted
 * helpers of util and unsorted_1003/unsorted_100d/view. Plain C, original
 * cdecl convention.
 */
#ifndef HG_SIM_UNSORTED_100B_H
#define HG_SIM_UNSORTED_100B_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Sim_1003a52b(value); the first argument is not used. */
void __cdecl Sim_100b3530(uint32_t unused, uint32_t value);
/*
 * Entry `index` (0x88 bytes each) of the table at 0x101a2100, valid for
 * 0 <= index <= the signed dword at 0x1017d218. Writes three dwords to a, b, c:
 * from +0x44/+0x48/+0x4c of the block pointed to by entry+0x28 if that is
 * non-zero, else what Sim_100a9cfb reads from the object at entry+0x2c if that
 * is non-zero; zero in every other case.
 */
void __cdecl Sim_100b53f0(int index, uint32_t *a, uint32_t *b, uint32_t *c);
/* 1 when the signed dword obj+0x514 is positive and Sim_1009c929((0x14 - obj+0x514) * 2) returns 0. */
int __cdecl Sim_100beea2(void *obj);
/*
 * Returns a 16-bit result in ax: 0xffff when obj+0 is 1, 0 when obj has no
 * entry in the pointer table at 0x109e8b40, else 2 when the entry's kind
 * (+0) satisfies Sim_10079feb and obj's kind does not, 0 otherwise. The index
 * is the low byte of the word obj+0x1aa, used only if its bit 0x200 is set.
 * The original leaves the rest of eax as it happened to be (obj itself or the
 * index while no call was made, 0 after a call); that is kept so the whole
 * register is identical.
 */
uint32_t __cdecl Sim_100bf026(void *obj);

#endif
