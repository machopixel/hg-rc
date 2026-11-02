/*
 * C runtime replacement, memory (DECISIONS.md, "C runtime replacement: memory"): the game's malloc / free / realloc /
 * calloc on the standard C library. Each block carries a 16-byte head so that free / realloc can tell our blocks from
 * the ones the DLL's own C runtime made at start-up in its private heap (those still go back to it, AFTER_DLL.md).
 */
#ifndef HG_SIM_CRT_HG_HEAP_H
#define HG_SIM_CRT_HG_HEAP_H

#include <stddef.h>

void *hg_malloc(size_t size);
void *hg_calloc(size_t count, size_t size);
/* Our block: resized (size 0 frees it and answers NULL, as the original). Not ours: never called (see Heap_Realloc). */
void *hg_realloc(void *block, size_t size);
void hg_free(void *block);
/* 1 when block came from hg_malloc / hg_calloc / hg_realloc and is still allocated. */
int hg_heap_owns(const void *block);

#endif
