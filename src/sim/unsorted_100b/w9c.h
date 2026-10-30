/*
 * Round 9, letter c: 0x100b5bcb and the resource cache loader 0x100bb405.
 */
#ifndef HG_SIM_UNSORTED_100B_W9C_H
#define HG_SIM_UNSORTED_100B_W9C_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* ---- w9c_misc.c ---- */
/* [0x1015084c] = 1 and dword +0x24 of the record [0x1015d1b8] = value. The first argument is not used. */
void __cdecl Sim_100b5bcb(uint32_t unused, uint32_t value);

/* ---- w9c_cache.c ---- */
/* The caller's fixup of a loaded block: (block, in/out size) -> a replacement block, or 0 to keep it. */
typedef uint32_t(__cdecl *W9cItemFixupFn)(uint32_t block, uint32_t *size);
/*
 * Upper-cases name and ext in place and returns the cached block for them (Sim_100baf3f) when there is one.
 * Otherwise loads it: the file "name.ext" ("rb"), else the pack file (0x100bca49) while [0x1017d5fc] is set,
 * else the search path (Sim_100bae0a); a file is read whole into a Heap_Malloc block (_assert "item" at
 * syscache.c line 0x598, or 0x5cd for the search path, when the allocation fails) and closed. Then the fixup, when
 * given, may replace the block (the old one is freed), the type record of ext (Sim_100bda9f on [0x1017d604])
 * takes `arg` at +0x20 and the block is filed under (name, ext, size) with Sim_100b9b3f. Returns the block, or 0
 * when nothing could be opened.
 *
 * Not referenced anywhere in the DLL. Without a fixup the original tests an uninitialised local as the
 * replacement block; here it is 0 (no replacement), which the checker cannot prove: the proof covers calls with
 * a fixup.
 */
uint32_t __cdecl Cache_LoadItem(char *name, char *ext, W9cItemFixupFn fixup, uint32_t arg);

#endif
