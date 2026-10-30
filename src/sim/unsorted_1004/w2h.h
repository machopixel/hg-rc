/*
 * Wave 2 functions of HGSIM.DLL around 0x10040000..0x10050000 (letter h).
 * Names carry the original address because the code alone does not say what
 * the fields mean.
 */
#ifndef HG_SIM_UNSORTED_1004_W2H_H
#define HG_SIM_UNSORTED_1004_W2H_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Copies a packed record into obj: word obj+0x198 = src word +8, dwords
 * obj+0x180/0x184 = src dwords +0/+4, word obj+0x1a0 = src byte +0xa, words
 * obj+0x1a2/0x1a4 = src words +0xb/+0xd, word obj+0x1a6 = src signed byte +0xf, words
 * obj+0x1a8/0x1aa = src words +0x10/+0x12, dword obj+0x1ae = src word +0x14, then
 * obj words 0x1f8 and 0x1fa = 0xffff, dword 0x1fc = -1, dwords obj+0x23e/0x242 =
 * src dwords +0x16/+0x1a, calls Sim_100119a7(obj) and sets dword obj+0xf0 to word obj+0x1aa.
 */
void __cdecl Sim_10040721(void *obj, const void *src);
/*
 * With rec = 0x109e8b40[index] (nothing when index is outside 0..[0x1015dc64) or
 * rec is NULL), when rec[0x14] is 3 and rec[0xc] differs from [0x1017b8c8]:
 * Sim_10040721(rec, record + 3).
 */
void __cdecl Sim_1004094e(const void *record, int32_t index);
/*
 * 0 when [0x101655ac] is -1; otherwise, with rec = 0x109e8b40[[0x101655ac]], 2 when
 * the dword at 0x109b7d54 + 0x6c * rec[8] is 0 and 3 when not. Only the low byte
 * is the result (the caller masks it); the upper bits of the value returned here
 * are those the original leaves in eax, except for the -1 case where it keeps
 * the caller's eax and this returns 0.
 */
uint32_t __cdecl Sim_100429d5(void);
/*
 * p[0] += (p[1] - p[0]) >> (signed word p+8, low 5 bits), signed shift; returns the new p[0].
 */
int32_t __cdecl Sim_10047e95(int32_t *p);
/*
 * Translates the byte *p: when it equals one of the byte pairs (x, y) at
 * 0x10144220 (6 pairs) it becomes y; otherwise 0x2b..0x3c map through the byte
 * table at 0x101441dd and 0x5b..0x5d get 0x20 added.
 */
void __cdecl Sim_10048bd6(uint8_t *p);
/* Sim_1003830e(0, 0x100, 0x109b8c00): copies the whole 256-entry table. */
int __cdecl Sim_1004b7b9(void);
/* Bit 12 of word. */
int __cdecl Sim_1004bfa9(int word);
/*
 * Dword 0x101116a6 + 0xaa * a, plus the dwords of the 0x98-byte-stride table at
 * 0x10124eac selected by Sim_1004bf79(b), Sim_1004bf87(b) and Sim_1004bf98(b), plus
 * the dword at 0x10126cf4 when Sim_1004bfa9(b).
 */
int __cdecl Sim_1004c221(int a, int b);
/*
 * Stores three bytes at [0x1015f360] + 3 * index: each of r, g and b times 255
 * divided by ((1 << [0x1015f22c]) - 1), signed. Faults when that is 0.
 */
void __cdecl Sim_1004de34(int32_t index, int32_t r, int32_t g, int32_t b);
/*
 * When [0x1013b9a8] is 0: for each entry i below the signed dword [[0x109e3da4] + 0xc]
 * takes its 3 bytes with Sim_1003830e(i, 1, buf) and calls
 * Sim_1004de34(i, buf[0], buf[1], buf[2]), then sets [0x1013b9a8] to 1.
 * The count must not exceed 256: past that the original passes uninitialised stack bytes.
 */
void __cdecl Sim_1004ecb8(void);

#endif
