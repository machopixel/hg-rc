/*
 * Round 5, letter c: functions at 0x100a6414..0x100ab0d6 that call helpers which are not converted yet.
 * Those helpers are called through their original address (SIM_FN) with the prototypes below.
 */
#ifndef HG_SIM_UNSORTED_100A_W5C_H
#define HG_SIM_UNSORTED_100A_W5C_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Not converted: six dwords. */
#define SIM_CALL_100a648c SIM_FN(int(__cdecl *)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t), 0x100a648c)
/* Not converted: (pointer to obj+0x7c, pointer to obj+0x4c). */
#define SIM_CALL_10060b76 SIM_FN(void(__cdecl *)(void *, void *), 0x10060b76)
/* Not converted: (four dwords of the caller, pointer to a 16-byte result). */
#define SIM_CALL_1006123b SIM_FN(void(__cdecl *)(uint32_t, uint32_t, uint32_t, uint32_t, float *), 0x1006123b)
/* Not converted: (string of node +0x90, name); returns non-zero for a match. */
#define SIM_CALL_100fc970 SIM_FN(char *(__cdecl *)(const char *, const char *), 0x100fc970)
/* Not converted: takes the node. */
#define SIM_CALL_100aa8cb SIM_FN(void(__cdecl *)(void *), 0x100aa8cb)

/* Picks the 0x58-byte entry b[a[byte at a+a->dword4]] and calls 0x100a648c with its first three dwords and the dwords at a+0xc..0x14, and returns its result (round 10: the result was lost before). */
int __cdecl Sim_100a6414(const void *a, const void *table);
/* Once only (flag 0x8000 of obj+0x94): calls 0x10060b76 on obj+0x7c / obj+0x4c, keeping the dwords at obj+0x70..0x78, and sets the flag. */
void __cdecl Sim_100a9ba7(void *obj);
/* Calls 0x1006123b to build a 16-byte value, multiplies it into the one at obj+0x3c (Quat_Multiply), clears flag 0x4000 and sets flag 1 of obj+0x94. */
void __cdecl Sim_100aa127(void *obj, uint32_t a, uint32_t b, uint32_t c, uint32_t d);
/* Depth-first searches of the tree (child at +4, sibling at +8) for the node whose string at +0x90 matches `name` per 0x100fc970. */
void *__cdecl Sim_100aae0f(void *node, const char *name);
/* The same search for the node whose pointer at +0x90 points to a record whose dword +0 / dword +4 equals dword +0 of `key`. */
void *__cdecl Sim_100aad46(void *node, const char *key);
void *__cdecl Sim_100aadaa(void *node, const char *key);
/* Walks the tree in reverse (sibling first); the first node with an object (+0x8c) whose word +0x26 equals `level` is published in 0x101c6504 / 0x101c64f4 and passed to 0x100aa8cb. */
void __cdecl Sim_100ab0d6(void *node, int level);

#endif
