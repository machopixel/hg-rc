/*
 * Round 5, letter b: functions around 0x100a78fd..0x100aab8e that call helpers which are not converted yet
 * (called through their address with SIM_FN).
 */
#ifndef HG_SIM_UNSORTED_100A_W5B_H
#define HG_SIM_UNSORTED_100A_W5B_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Not converted. 0x10105750 is the C library's _write(handle, buffer, count). */
#define SIM_WRITE SIM_FN(int(__cdecl *)(uint32_t, const void *, uint32_t), 0x10105750)
/* The second argument is only ever read as a word. */
#define SIM_CALL_100a7f8c SIM_FN(uint32_t(__cdecl *)(int, int, uint16_t), 0x100a7f8c)
#define SIM_CALL_10060b76 SIM_FN(void(__cdecl *)(void *, void *), 0x10060b76)
#define SIM_CALL_10060d2e SIM_FN(void(__cdecl *)(const void *, void *), 0x10060d2e)
#define SIM_CALL_100aa8cb SIM_FN(void(__cdecl *)(void *), 0x100aa8cb)
/* 0x100ff1e0 is the C library's case-insensitive string compare (_strcmpi). */
#define SIM_STRCMPI SIM_FN(int(__cdecl *)(const char *, const char *), 0x100ff1e0)

/* Writes `size` bytes in pieces of at most 0x4000 with _write(handle, ...). Returns size, or -1 as soon as a piece is not written completely. */
int __cdecl Sim_100a78fd(uint32_t handle, const void *buffer, uint32_t size);
/*
 * With w = the low word of c: 0x100a7f8c(a, b, w) when w < 0xf000, else Sim_100c6f0e([0x1017d158], w - 0xf000). (The original also
 * compares the zero-extended w with -1, which never matches.)
 */
int __cdecl Sim_100a8068(int a, int b, int c);
/*
 * Unless dword obj[0x94] has bit 0x4000: keeps dwords obj[0x30], obj[0x34], obj[0x38] around 0x10060b76(obj + 0x3c, obj + 0xc)
 * (restoring them after it) and sets bit 0x4000.
 */
void __cdecl Sim_100a9c25(void *obj);
/* Copies 0x30 bytes from src to obj + 0xc, runs 0x10060d2e(src, obj + 0x3c) and sets bits 0x4001 of dword obj[0x94]. */
void __cdecl Sim_100a9dc3(void *obj, const void *src);
/*
 * Tree walks over nodes with the first child at +4 and the next sibling at +8 (read after the child was handled).
 * Sim_100aa39f: for node[0x8c] (a record) not 0 and whose word +2 does not have 0x70 in its mask 0xf0 runs Sim_1008a46e(record);
 * then itself on every child.
 */
void __cdecl Sim_100aa39f(void *node);
/* Sim_1002d890(record, state) for node[0x8c] not 0, then itself (with state) on every child. */
void __cdecl Sim_100aa592(void *node, int32_t state);
/*
 * Runs 0x100aa8cb(node); then itself on every child (next sibling read before the call); then, when node[0x8c] and `callback`
 * are not 0, callback(node[0x8c]); and when bit 4 of dword node[0x94] is set frees the node itself with HeapFree(heap [0x1017b8f8], 1, node).
 */
void __cdecl Sim_100aab03(void *node, void(__cdecl *callback)(void *));
/*
 * The first node of the tree (node itself first, then the children in depth-first order) whose string pointer node[0x90] is not 0
 * and equal to `name` ignoring case (_strcmpi); NULL when there is none.
 */
void *__cdecl Sim_100aab8e(void *node, const char *name);

#endif
