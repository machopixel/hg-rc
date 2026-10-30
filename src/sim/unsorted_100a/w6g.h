/*
 * Round 6, letter g: functions around 0x100a2823..0x100ad87d. Most work on the scene nodes of 0x100a9830
 * (0xa4 bytes: parent at +0, first child at +4, next sibling at +8, two 3x4 matrices at +0xc and +0x4c, the
 * vectors at +0x3c and +0x7c, the object at +0x8c, flags at +0x94); helpers that are not converted are called
 * through their original address.
 */
#ifndef HG_SIM_UNSORTED_100A_W6G_H
#define HG_SIM_UNSORTED_100A_W6G_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#define __stdcall
#endif

/* Patched by src/hooks/hooks_view.c: always called through the original address. */
#define W6G_CALL_100d1f90 SIM_FN(void(__cdecl *)(uint32_t), 0x100d1f90)
#define W6G_CALL_10087d83 SIM_FN(void(__cdecl *)(uint32_t), 0x10087d83)
#define W6G_CALL_100585c4 SIM_FN(int(__cdecl *)(int), 0x100585c4)
/* Not converted. */
#define W6G_CALL_100a4f03 SIM_FN(void(__cdecl *)(uint32_t), 0x100a4f03)
#define W6G_CALL_100c6e27 SIM_FN(uint32_t(__cdecl *)(const char *), 0x100c6e27)
#define W6G_CALL_100a7e55 SIM_FN(int(__cdecl *)(int), 0x100a7e55)
#define W6G_CALL_10047bcd SIM_FN(double(__cdecl *)(void *), 0x10047bcd)
#define W6G_CALL_100aaa08 SIM_FN(void(__cdecl *)(void *, void *), 0x100aaa08)
#define W6G_CALL_10060d2e SIM_FN(void(__cdecl *)(const void *, void *), 0x10060d2e)

/* The drawing function the DLL stores in dword 0x1014cb4c: (vertex count, vertices of 0x24 bytes, colour/flags). */
typedef uint32_t(__cdecl *W6gDrawFn)(int, void *, uint32_t);
/* HeapAlloc (import slot 0x10ac5658). */
typedef void *(__stdcall *W6gHeapAllocFn)(uint32_t, uint32_t, uint32_t);

/*
 * Five vertices (x, y) = the ten integer arguments as floats, each with dword +0x10 = colour and +0x18 = 0xff000000
 * (the other fields are not set, as in the original), drawn through the function in dword 0x1014cb4c with
 * (5, vertices, colour). Returns what that function returns.
 */
uint32_t __cdecl Sim_100a2823(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4,
                              uint32_t colour);

/* 0x100d1f90(b), 0x10087d83(b), then 0x100a4f03(entry) for each entry of the zero-terminated dword table 0x1036f2a0. Returns 0. */
int __cdecl Sim_100a5336(uint32_t a, uint32_t b);

/*
 * Sim_10027df4(); Sim_100c6e87 on dword 0x1017d158 when set; copies name to 0x101c6520; dword 0x1017d158 =
 * 0x100c6e27(name), which is returned.
 */
uint32_t __cdecl Sim_100a7880(const char *name);

/*
 * -1 when the low word of slot is 0 or Sim_100a7da3(index, key) gives a negative (16-bit) entry number; when the
 * entry's pointer (record 0x101c65e6 + 0x14a * index, dword +0x24 + 8 * entry) is 0 it is loaded with
 * 0x100a7e55(index) first (-1 when that answers -1). Then v = dword (pointer + 0x16 + 8 * slot + 4) minus the word
 * +0x14 of the entry's 0x18-byte name record; returns v when it is positive, else 0.
 */
int __cdecl Sim_100a7f8c(int index, const char *key, uint32_t slot);

/*
 * Unless obj is NULL: dword +0x4e0 = 0, 0x10047bcd(obj + 4) and 0x10047bcd(obj + 0x14) (results dropped);
 * Sim_100aa0da(dword +0x490, dword +0x1c, 0, 0, 0) when +0x490 is set; when +0x48c is set: the dwords +0x70, +0x74,
 * +0x78 of the object at dword +0 become 0, dword obj+0xc, 0 and Sim_100aa0da(dword +0x48c, those three, 0); finally
 * Sim_100aa799(dword +0x48 of the object at dword +0).
 */
void __cdecl Sim_100a8c24(void *obj);

/* When dword 0x1017d1b8 is set: Sim_100aa39f (flag != 0) or Sim_100aa351 (flag == 0) on it, Sim_100aa799 on it, dword 0x1017d1b0 = flag. */
void __cdecl Sim_100a96d8(int flag);

/*
 * New node of 0xa4 bytes: Heap_AllocRaw when flags bit 1 is set (flags +0x94 = 2), else HeapAlloc(dword 0x1017b8f8,
 * 1, 0xa4) when bit 2 is set (flags = 4); NULL when neither or when the allocation fails. Linked under parent (as
 * the last child with flags bit 0x10, else as the first), +0xc identity, +0x3c from Sim_100610ae(0, 0, 0, 0), +0x4c
 * and +0x7c copied from the parent (identity and Sim_100610ae without one); clears +0x8c, +0x90, +0x9c, +0xa0, sets
 * flags bit 0, +0x98 = dword 0x1014d1a8, which steps to ((v + 7) & 0x7f) + 0x40. Returns the node.
 */
void *__cdecl Sim_100a9830(void *parent, uint32_t flags);

/* The same set-up on the caller's node (always linked as the first child, flags +0x94 = 1). Returns the node. */
void *__cdecl Sim_100a9a1a(void *parent, void *node);

/* Copies the nine dwords src[0..8] to node+0xc, 0x10060d2e(src, node + 0x3c), sets bits 0x1 and 0x4000 of +0x94. */
void __cdecl Sim_100a9e1a(void *node, const void *src);

/*
 * Unlinks the node from its parent's child list, clears +0 and +8, copies the 0x30 bytes +0x4c to +0xc and the
 * 0x10 bytes +0x7c to +0x3c, sets bits 0x4000 and 0x1 and clears bit 0x8000 of +0x94; with an object at +0x8c: its
 * word +0x24 = 0xffff, Sim_1008a40a and Sim_1008a46e on it; then Sim_100aa799(node).
 */
void __cdecl Sim_100aa8cb(void *node);

/*
 * For 0 < level < 16 and level greater than bits 4..7 of Sim_100093ea(object +0x8c): when the object's word +0x26
 * equals key, Sim_100093be(object, level << 4); the same for every child (recursively).
 */
void __cdecl Sim_100aae7b(void *node, int level, uint32_t key);

/*
 * Unless node or its object (+0x8c) is NULL: Sim_10009408(object, 0x50), Sim_100aa8cb, Sim_100aa351, Sim_100aa406,
 * Sim_100aa799 on the node, Sim_1003b8e3(node).
 */
void __cdecl Sim_100aaf32(void *node);

/*
 * Unless one of them is NULL: 0x100aaa08(b, a), Sim_100a9d47(b, 0, 0, 0), Sim_100a9ccf(a, &x, &y, &z),
 * Sim_100aa0da(b, x, y, z, 0).
 */
void __cdecl Sim_100ab186(void *a, void *b);

/*
 * For 0 < kind < 0x40: kind 1 (src required), 2 (0x100585c4(-1) non-zero and src required) and 6 (0x100585c4(-1)
 * non-zero) take an entry from Sim_100abc5d and fill it (kind at +4, src at +8, a float at +0x2c: 1000, 2000 or 1)
 * and its node (+0x30) and the node's object (+0x8c) from src; returns the entry's dword +0 (or -1 when no entry
 * was free, another kind is given, or a condition fails).
 */
int __cdecl Sim_100ac21a(int kind, void *src);

/*
 * For 0 <= index < dword 0x1017d1c8 (record 0x101c57e0 + 0x34 * index): flags bit 0 stores vals[0..2] at +0x14 and
 * in the node (+0x30) and its object, bit 1 stores vals[3] at +0x20 and clears +0x28, bit 2 stores vals[4] at +0x24
 * and 1.0 at +0x2c.
 */
void __cdecl Sim_100ac620(int index, uint32_t flags, const uint32_t *vals);

/*
 * Unless ctx, node or the node's object (+0x8c) is NULL: Sim_100aa307(object) when its +0x9c differs from +0x10 of its
 * mesh (+0x30); when bits 0x70 of the object's word +2 are clear, Sim_100ad1f3(ctx, part) for each of the (word +6)
 * parts of 0x28 bytes at mesh + dword mesh+8; then the same for every child.
 */
void __cdecl Sim_100ad87d(void *ctx, void *node);

#endif
