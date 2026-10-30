/*
 * Wave 2l: functions around 0x100c3eb5..0x100cb9f3 on the record objects. Names
 * carry the original address because the code alone does not say what the
 * fields mean. Plain C, original cdecl convention.
 */
#ifndef HG_SIM_UNSORTED_100C_W2L_H
#define HG_SIM_UNSORTED_100C_W2L_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * q = dword at obj+0, r = dword q+0x54. 1 when the dword obj+0x4f4 is non-zero, the signed byte r+0x59 is not 1,
 * the dword q+0x264 is zero and the short q+0x1f8 is not 10; otherwise 0.
 */
int __cdecl Sim_100c3eb5(const void *obj);
/* For a non-NULL obj with a non-zero dword +0x54: dword (obj+0x54)+8 = 0 and byte (obj+0x54)+0x54 = 1. */
void __cdecl Sim_100c4898(void *obj);
/*
 * obj+0x208 is a cursor (0..2) and obj+0x20c holds three dwords, -1 for an empty one. Steps the cursor
 * round the three entries to the next that is not -1 and stores its index in *out, returning the entry; when
 * none other than the starting one qualifies (the cursor came back to its start) *out = -1 and the result is -1.
 * A NULL obj gives *out = -1 and -1.
 */
int __cdecl Sim_100c4966(void *obj, int *out);
/* The low byte of the dword +0xf0 of the entry of the pointer table 0x109e8b40 selected by [0x1017b8c8] when its bits 0xf00 are 0x400, else -1. */
int __cdecl Sim_100cb9f3(void);

#endif
