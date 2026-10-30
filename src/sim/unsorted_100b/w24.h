/*
 * Round 24: the leftover readers (DECISIONS.md, "Round 24").
 */
#ifndef HG_SIM_UNSORTED_100B_W24_H
#define HG_SIM_UNSORTED_100B_W24_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Loads the resource named at key + 2 (table `table`) through the cache loader 0x100ba7d5 (its size to *size). When
 * the cache has none, loads the loose file of extension ext (at most 4 characters, the path by Sim_100474db) with
 * 0x10046ac4 into a heap block and closes it; key[0] = -1 then (the callers free that block with HeapFree, the
 * cache's through 0x100bb2b0), else 0. With copy non-null and a size above 0, a block of 0x100b659a(*size, *copy)
 * gets a copy and the cache entry is released (0x100bb2b0, 0x100b9203). Returns the data (or the copy), 0 on failure.
 * Two unset locals of the original are fixed values here (DECISIONS.md, round 24): key[0] on the cache path is 0,
 * and the loose file's name is the extension alone.
 */
void *__cdecl Sim_100b8d6c(int16_t *key, uint32_t table, const char *ext, int unused, uint32_t *size,
                           const uint32_t *copy);

#endif
