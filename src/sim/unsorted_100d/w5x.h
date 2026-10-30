/*
 * Round 5, letter x: small functions at 0x100d1510..0x100d3830 (tree nodes of 0x18 bytes, two printf calls, the
 * loader of a named file) that call helpers which are not converted yet (called through their address, SIM_FN).
 */
#ifndef HG_SIM_UNSORTED_100D_W5X_H
#define HG_SIM_UNSORTED_100D_W5X_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Not converted. 0x100fef30 (size -> block), 0x100fefe0 (block) and 0x100fdc80 (format, argument) are library
 * code (allocate, free and print through the stream 0x10159b68); 0x100ba7d5 is the cache loader (name, section, 0).
 */
#define SIM_CALL_100fef30 SIM_FN(void *(__cdecl *)(uint32_t), 0x100fef30)
#define SIM_CALL_100fefe0 SIM_FN(void(__cdecl *)(void *), 0x100fefe0)
#define SIM_CALL_100fdc80 SIM_FN(int(__cdecl *)(const void *, const void *), 0x100fdc80)
#define SIM_CALL_100ba7d5 SIM_FN(void *(__cdecl *)(const void *, uint32_t, int), 0x100ba7d5)
#define SIM_CALL_100bb2b0 SIM_FN(int(__cdecl *)(const void *, uint32_t), 0x100bb2b0)
#define SIM_CALL_100d3510 SIM_FN(uint32_t(__cdecl *)(uint32_t, uint32_t, int), 0x100d3510)

/* Allocates a 0x18-byte node with 0x100fef30, stores `first` in its first dword, clears the dwords +4..+0x14 and returns it. */
void *__cdecl Sim_100d1510(uint32_t first);
/* List_Unlink(node), then Sim_100d15fd(node). */
void __cdecl Sim_100d15e0(void *node);
/* Frees a node tree: the nodes at +0xc and +8 first, then the block the first dword points at (when non-zero), then the node. NULL is ignored. */
void __cdecl Sim_100d15fd(void *node);
/* Two calls of 0x100fdc80: (0x10152288, 0x10152230) and (0x101523c0, 0x10152290). */
void __cdecl Sim_100d1dd0(void);
/*
 * Str_Copy(0x10185620 + 16 * slot, name); then 0x100ba7d5(name, [0x1014ced4], 0) and 0x100bb2b0(name, [0x1014ced4]).
 */
void __cdecl Sim_100d37ea(const char *name, int slot);
/* 0x100d3510(a, b, 0); then [0x1017e654] = a and [0x10152424] = a. */
void __cdecl Sim_100d3830(uint32_t a, uint32_t b);

#endif
