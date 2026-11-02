/*
 * The game's Windows-style heaps (DECISIONS.md, "Windows calls: memory"): HeapCreate / HeapAlloc / HeapReAlloc /
 * HeapFree / HeapSize / HeapValidate / HeapDestroy on the standard C library. SimMain makes seven heaps (the game
 * heap [0x1017b8f8] is one of them) and frees each as a whole with HeapDestroy at the end, so every block is kept in
 * its heap's list. While HGSIM.DLL is loaded its own import slots point here too (hooks_heap.c), so the original
 * code and the converted code share these heaps.
 *
 * Handles are 32-bit, as the DLL keeps them in 32-bit globals. Flags as Windows': 8 (HEAP_ZERO_MEMORY) clears a
 * new block or the grown part of a resized one, 0x10 (HEAP_REALLOC_IN_PLACE_ONLY) never moves the block; 1
 * (HEAP_NO_SERIALIZE) and 4 (HEAP_GENERATE_EXCEPTIONS) are accepted and change nothing (no lock, as with the
 * game's own flag 1; failures answer 0).
 */
#ifndef HG_SIM_CRT_HG_WINHEAP_H
#define HG_SIM_CRT_HG_WINHEAP_H

#include <stdint.h>

#define HG_HEAP_ZERO_MEMORY 0x08u
#define HG_HEAP_REALLOC_IN_PLACE_ONLY 0x10u

/* maximum 0: the heap grows as needed (all 10 of the game's HeapCreate calls); otherwise its blocks never add up
 * to more than maximum bytes (Windows also counts its own overhead, so its limit comes a little sooner). */
uint32_t Hg_HeapCreate(uint32_t options, uint32_t initial, uint32_t maximum);
/* Frees every block of the heap and the heap itself; 1 on success. */
int Hg_HeapDestroy(uint32_t heap);
void *Hg_HeapAlloc(uint32_t heap, uint32_t flags, uint32_t bytes);
void *Hg_HeapReAlloc(uint32_t heap, uint32_t flags, void *block, uint32_t bytes);
/* 1 on success; a null block is a success, as Windows'. */
int Hg_HeapFree(uint32_t heap, uint32_t flags, void *block);
/* The size asked for, 0xffffffff for a block that is not the heap's. */
uint32_t Hg_HeapSize(uint32_t heap, uint32_t flags, const void *block);
/* 1 when the heap (and the block, when not null) is valid. */
int Hg_HeapValidate(uint32_t heap, uint32_t flags, const void *block);
/* 1 when heap is a handle Hg_HeapCreate gave and is not destroyed. */
int Hg_HeapOwns(uint32_t heap);

#endif
