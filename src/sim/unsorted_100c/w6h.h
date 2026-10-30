/*
 * Round 6, letter h: functions at 0x100c17b7..0x100cb8ea that call helpers which are not converted yet (called
 * through their original address, SIM_FN, with the prototypes below). Floating point is x87 in the originals at the
 * 53-bit precision the Windows loader sets: intermediates are doubles, stored results floats.
 */
#ifndef HG_SIM_UNSORTED_100C_W6H_H
#define HG_SIM_UNSORTED_100C_W6H_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Not converted: the C library fabs (a double on the stack, the result in st(0)). */
#define W6H_FABS_C SIM_FN(double(__cdecl *)(double), 0x100fcb70)
/* Not converted: (obj, a float as bits); returns an int. */
#define SIM_CALL_100c3550 SIM_FN(int(__cdecl *)(void *, uint32_t), 0x100c3550)
/* Not converted: (four floats as bits, a float block); returns a float in st(0). */
#define SIM_CALL_10009fe8 SIM_FN(double(__cdecl *)(uint32_t, uint32_t, uint32_t, uint32_t, void *), 0x10009fe8)
/* Not converted: (three floats as bits, four out floats); only the first out float is used here. */
#define SIM_CALL_100cbc6b SIM_FN(void(__cdecl *)(uint32_t, uint32_t, uint32_t, float *, float *, float *, float *), 0x100cbc6b)
/* Not converted: (three floats as bits, a word, out pointer to a dword array); returns the number of entries. */
#define SIM_CALL_10033eb3 SIM_FN(int(__cdecl *)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t **), 0x10033eb3)
/* Not converted: (number of characters, text); the number in the text. */
#define SIM_CALL_100c6b30 SIM_FN(uint32_t(__cdecl *)(int, const char *), 0x100c6b30)
/* Not converted: (dword, the low byte of a key, 0); a negative value means not found. */
#define SIM_CALL_100cab7b SIM_FN(int(__cdecl *)(uint32_t, uint32_t, uint32_t), 0x100cab7b)

/* The callback type of Sim_100c417e: (previous result (first -1), entry, rec, other); returns the next result. */
typedef int(__cdecl *W6hEntryFn)(int, uint32_t, void *, void *);

/*
 * x = Sim_10015266(obj); Sim_1000ae00(obj, x); when fabs(x) > 10.0f (0x1012bb60) also Sim_100c0379(obj,
 * Sim_100144e9(obj)); then Sim_100c4898(obj) and returns Sim_100c1853(obj).
 */
int __cdecl Sim_100c17b7(void *obj);
/*
 * When (dword obj[0xf0] & 0xf00) == 0x200 and 0x100c3550(obj, float obj[0x5c]) is 0: returns 1 when
 * float obj[0xe0] - 800.0f (0x1012bb64) > float obj[0x5c] and the dword [obj[0x28] + 0x510] >= 0. Else 0.
 */
int __cdecl Sim_100c25af(void *obj);
/*
 * Goes through the first [0x1015dc64] entries of the pointer table 0x109e8b40, skipping the ones whose dword +4
 * equals `skip` (and null ones): with the floats rec[0x58..0x60] and the float rec[0x246] (use_own) or
 * [rec[0x28] + 0x53c] asks 0x10009fe8(..., block); the first time the float result is below float block[0x30]
 * (or unordered) stores [rec[0x48] + 0x8c] in *out and returns 1. Returns 0 otherwise.
 */
int __cdecl Sim_100c2851(void *block, uint32_t *out, uint32_t skip, int use_own);
/*
 * 0x100cbc6b(other[0x44] - x, other[0x48] - y, other[0x4c] - z, &a, ...); d = a - obj[0x68] brought into
 * -180..180 (360 is subtracted when d > 180, added when the float d < -180); returns 1 when d is negative, else -1.
 */
int __cdecl Sim_100c3372(void *obj, void *other, float x, float y, float z);
/* As Sim_100c3372 with the differences x1 - x2, y1 - y2, z1 - z2. */
int __cdecl Sim_100c3419(void *obj, float x1, float y1, float z1, float x2, float y2, float z2);
/*
 * -1 when rec or other is null. Asks 0x10033eb3(rec[0x58], rec[0x5c], rec[0x60], key, &list) for a list of entries;
 * when there is none, returns Sim_10033d75 of those floats. Otherwise calls fn(result, entry, rec, other) for every
 * entry, result starting at -1, and returns the last result.
 */
int __cdecl Sim_100c417e(void *rec, void *other, uint32_t key, W6hEntryFn fn);
/*
 * -1 unless bit 0x200 of the word obj[0x1aa] is set. With the table entry of its low byte as other:
 * mode 1 -> Sim_100c417e(obj, other, key, 0x10002964), mode 2 -> the same with 0x10002513, else -1.
 */
int __cdecl Sim_100c43e1(void *obj, uint32_t key, int mode);
/*
 * Reads the numbers of a tar header (fields at the standard offsets, 0x100c6b30(width, text) each): mode (+0x64)
 * to out[8], mtime (+0x88) to out[0x24], size (+0x7c) to out[0x1c]. When the magic at +0x101 is "ustar  " (the
 * string at 0x10151ce8) sets *gnu = 1; otherwise *gnu = 0 and reads uid (+0x6c) to out[0x10] and gid (+0x74) to
 * out[0x14]. out[0] = 0 in both cases.
 */
void __cdecl Tar_ReadHeader(const char *header, void *out, uint32_t *gnu);
/*
 * When (dword obj[0x118] & 0xf00) == 0x100 and 0x100cab7b(obj[4], obj[0x118] & 0xff, 0) >= 0 returns 1. Otherwise
 * sets bit 0x1000 of the dwords obj[0xf0] and obj[0x118] and returns 0.
 */
int __cdecl Sim_100cb8ea(void *obj);

#endif
