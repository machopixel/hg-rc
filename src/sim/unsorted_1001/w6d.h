/*
 * Round 6, letter d: functions in 0x10010b92..0x1001fee5 that call helpers which are not converted yet (called
 * through their original address with SIM_FN, prototypes below) besides converted ones.
 */
#ifndef HG_SIM_UNSORTED_1001_W6D_H
#define HG_SIM_UNSORTED_1001_W6D_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Not converted: six floats (a point and a direction). */
#define W6D_CALL_10095e0a SIM_FN(void(__cdecl *)(float, float, float, float, float, float), 0x10095e0a)
/* Not converted: three floats and four float pointers it writes. */
#define W6D_CALL_100cbc6b SIM_FN(void(__cdecl *)(float, float, float, float *, float *, float *, float *), 0x100cbc6b)
/* Not converted: (object, 1). */
#define W6D_CALL_1000a6f0 SIM_FN(int(__cdecl *)(void *, int), 0x1000a6f0)
/* The C library fabs of the DLL (0x100fcb70), on a double. */
#define W6D_FABS SIM_FN(double(__cdecl *)(double), 0x100fcb70)
/* The C library sprintf of the DLL (0x100fc780). */
#define W6D_SPRINTF SIM_FN(int(__cdecl *)(char *, const char *, ...), 0x100fc780)
/* The C library sscanf of the DLL (0x100fd710): (text, format, ...). */
#define W6D_SSCANF SIM_FN(int(__cdecl *)(const char *, const char *, ...), 0x100fd710)
/* Not converted; reads only the low 16 bits of its second argument. */
#define W6D_CALL_10016117 SIM_FN(uint32_t(__cdecl *)(uint32_t, uint32_t), 0x10016117)
/* Not converted: one pointer. */
#define W6D_CALL_100463bf SIM_FN(int(__cdecl *)(void *), 0x100463bf)
#define W6D_CALL_1001fc18 SIM_FN(int(__cdecl *)(uint32_t), 0x1001fc18)
/* Not converted: twelve dwords (the third is the name index from Sim_1000df9c). */
#define W6D_CALL_1000e737_1001 SIM_FN(int(__cdecl *)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t), 0x1000e737)

/* Sim_100109e7([0x1015d100]); when that is >= 0, Sim_10010767(it, 0). */
void __cdecl Sim_10010b92(void);
/*
 * Transforms the points (0, 0, -1000) and (0, 0, -1300) with the matrix Sim_100aa863(obj[0x48]) and calls
 * 0x10095e0a(first point, second point - first point).
 */
void __cdecl Sim_1001274b(void *obj);
/*
 * 0x100cbc6b(a[0] - b[0], a[1] - b[1], a[2] - b[2], &u, &u, &v, &u) on two float triples; returns v.
 */
float __cdecl Sim_100127f0(const float *a, const float *b);
/*
 * For the record PTRTAB(key & 0xff) without bit 0x10 in its word +0x1c the answer is 1, with it 0, when flag is 0
 * or the word obj[0x1da] >= [0x1015dc78]. Otherwise the answer is 1, except when value > the float obj[0xd4],
 * key has bit 0x200, the record has bit 0x10 and 0x1000a6f0(obj, 1) is 0: then 0 and the word [0x10a38a74] counts
 * up; and obj[0x1da] becomes [0x1015dc78] + 0x21f.
 */
int __cdecl Sim_10013516(void *obj, int16_t key, int16_t value, int flag);
/* Sim_1001655f(f / 11520 (the float at 0x1011114c), 2 * n, 1). */
float __cdecl Sim_1001448d(uint32_t unused, float f, int n);
/*
 * x + n * 2^-16 limited to [-1/64, 1/64] (the floats at 0x1011116c, 0x10111118, 0x10111170). When flag is set, n
 * is first negated when its sign (-1, 0, 1) differs from the sign of x, which counts as 0 when |x| < 1e-7
 * (0x10111120) or x is a NaN.
 */
float __cdecl Sim_1001655f(float x, int n, int flag);
/*
 * Four calls Sim_100ca350(obj[4], ...) with the point of the target selected by the word obj[0x1a8] moved by
 * -r and +r along x and along z, r being Sim_10014061 of that word (or the dword obj[0x1d2] for group 0x400,
 * the larger of it and the sum of the floats +0x53c of both objects + 3000 for group 0x200), then
 * Sim_100159cf(obj, 0x100). Nothing for other groups.
 */
void __cdecl Sim_100157ca(void *obj);
/*
 * Unless the record PTRTAB(index) already has word +0x1a8 == flags | 0x200 and word +0x1a6 == 3, prints a line
 * (format 0x10131364 with [0x1015dc78], record[8], index, flags) with Sim_1005f9aa; then sets +0x1aa to
 * flags | 0x200 and +0x1a6 to 3. Nothing for a record other than the current one ([0x1017b8c8]) when
 * [0x1015ecd0] is 0, or when its dword +0x14 is 3 and its +0xc is the current one.
 */
void __cdecl Sim_10015afd(int index, uint32_t flags);
/*
 * a == -1: Sim_1005259e([0x1013bb40], entry b of the table 0x109b68e0) and returns Sim_100203b8(b); otherwise
 * returns 0x10016117(b, a).
 */
uint32_t __cdecl Sim_100178af(int a, int b);
/*
 * Looks up the word key[0] in the (key, value) dword pairs at 0x10a37aa0 (count [0x1015d188]): found, sets
 * [0x1015d184] to its value and returns -1. Otherwise, while fewer than 25 pairs: [0x1015d184] = [0x1015d180] + 1
 * when that is > 0, result = 0x100463bf(key), and appends ([0x1015d184], key). Returns the result (0 when full).
 */
int __cdecl Sim_10019cc0(int16_t *key);
/*
 * Once (while [0x10131750] is set, then cleared): for every entry i (count at dword +0xc of obj[0x14]) of the
 * 0xc-byte entries at obj + 0x1c, finds the node named (entry dword + 4) in the tree of PTRTAB(index)[0x48] with
 * Sim_100aae0f; a found node gets +0x9c = 0 and +0xa0 = the entry's address.
 */
void __cdecl Sim_1001c107(int index, void *obj);
/*
 * The first time ([0x1015d1c0] 0, then 1) reads the track number [0x101321a8] from the text 0x1015dcb5 (format
 * 0x1013221c). The track becomes -1 when the CD is not open, its status is 1, the track is not in range, or (when
 * [0x101321c0] has bit 8 and it was played) the status after playing is not 3. Ends with [0x101321ac] = -1.
 */
void __cdecl Sim_1001f26e(void);
/*
 * For the list at [0x1015d528] (next at +0x67): when its head's dword +0x2c is set and > [0x1015dc78], or is 0 and
 * [0x1015d52c] is 0, clears the head's first dword and moves to the next until 0x1001fc18(next) >= 0 or the list ends.
 */
void __cdecl Sim_1001fb6d(void);
/*
 * Sim_1000df9c of the name 0x10133968 (with bit 2 of [0x101321c0]) or 0x10133950 + 0x60 * dword +0x613 of the
 * current record's block, then 0x1000e737 with it; returns what 0x1000e737 returns.
 */
int __cdecl Sim_1001fe29(void);
/*
 * Unless [0x1015d528] is null or [0x1015d530] is set: frees (Sim_1001ffc8) the head when its dword +0x30 < 0x50,
 * then every following element (next at +0x67) when flag is 0, or when its +0x24 is 0 and +0x30 < 0x50; sets
 * [0x1015d530] to 1.
 */
void __cdecl Sim_1001fee5(int flag);

#endif
