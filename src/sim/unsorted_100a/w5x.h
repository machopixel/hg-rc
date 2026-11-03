/*
 * Round 5, letter x: small functions around 0x100a6009..0x100ae64d (object trees, lists and
 * single-call wrappers) that call helpers which are not converted yet (called through their address, SIM_FN).
 */
#ifndef HG_SIM_UNSORTED_100A_W5X_H
#define HG_SIM_UNSORTED_100A_W5X_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Not converted. */
#define SIM_CALL_100a5d38 SIM_FN(void *(__cdecl *)(uint32_t, uint32_t, uint32_t, uint32_t), 0x100a5d38)
#define SIM_CALL_100a9ba7 SIM_FN(void(__cdecl *)(void *), 0x100a9ba7)
#define SIM_CALL_10060681 SIM_FN(void(__cdecl *)(void *, uint32_t, uint32_t, uint32_t), 0x10060681)
#define SIM_CALL_10060d2e SIM_FN(void(__cdecl *)(const void *, void *), 0x10060d2e)
#define SIM_CALL_100aaf32 SIM_FN(void(__cdecl *)(void *), 0x100aaf32)
/* C library functions (copies inside the DLL) used by Sim_100ae64d: a stream function (stream, a, b) and a one-argument stream function. */
#define SIM_CALL_100feb20 SIM_FN(int(__cdecl *)(uint32_t, uint32_t, uint32_t), 0x100feb20)
#define SIM_CALL_100fe970 SIM_FN(int(__cdecl *)(uint32_t), 0x100fe970)

/* Returns 0x100a5d38(a, 0, b, 0) (round 10: the result was lost before). */
void *__cdecl Sim_100a6009(uint32_t a, uint32_t b);
/*
 * The polygon buffers (RenderInit's AllocPolyBuffers): [0x1017d17c] = sortEntries, [0x1014d1a4] = arenaKiB KB, one
 * zeroed block of the arena plus 16 bytes per sort entry ([0x1017d180]; SystemError 0x18 when it fails) laid out as
 * the arena ([0x1017d170] = [0x1017d178], its end less 0x30 at [0x1017d174]) followed by the sort entries
 * ([0x1017d168], [0x1017d16c] = 8 bytes per entry further); then 0x100846e0 and 0x10084a17, whose answer it returns.
 * The view-detail hook (hooks_view.c) calls it with larger buffers.
 */
int __cdecl Sim_100a8816(int arenaKiB, int sortEntries);
/* 0x100a9ba7(obj), then 0x10060681(obj + 0x4c, a, b, c). */
void __cdecl Sim_100a9ca3(void *obj, uint32_t a, uint32_t b, uint32_t c);
/*
 * Walks the child list of the node (first child at +4, next sibling at +8) and runs Sim_100aa351 on every child,
 * after Sim_1008a40a on the node's dword at +0x8c when that is non-zero.
 */
void __cdecl Sim_100aa351(void *node);
/* The same walk with Sim_1008a220 in place of Sim_1008a40a, recursing into Sim_100aa454. */
void __cdecl Sim_100aa454(void *node);
/*
 * 0x10060d2e(src, dst + 0x3c); clears bit 0x4000 of the dword dst+0x94; copies the three dwords src+0x24..0x2c
 * to dst+0x30..0x38.
 */
void __cdecl Sim_100aa812(void *dst, const void *src);
/* 0x100a9ba7(obj); returns obj + 0x4c. */
void *__cdecl Sim_100aa863(void *obj);
/* Post-order walk of a binary tree (children at +8 and +4): 0x100aaf32(node) for every node. NULL is ignored. */
void __cdecl Sim_100aaf9b(void *node);
/* For 0 <= index < [0x1017d1c8] (signed): Sim_100aa351(dword +0x30 of the 0x34-byte record 0x101c57e0 + 0x34 * index), then Sim_100abbe8(index). */
void __cdecl Sim_100ac58e(int index);
/*
 * r = 0x100feb20([p], a, b); when r is non-zero returns it, else stores 0x100fe970([p]) at p+4 and returns 0.
 */
int __cdecl Sim_100ae64d(void *p, uint32_t a, uint32_t b);

#endif
