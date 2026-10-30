/*
 * Round 5, letter b: functions around 0x1005259e..0x1005f9aa that call helpers which are not converted yet
 * (called through their address with SIM_FN).
 */
#ifndef HG_SIM_UNSORTED_1005_W5B_H
#define HG_SIM_UNSORTED_1005_W5B_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Not converted. 0x100ff1e0 is the C library's case-insensitive string compare (_strcmpi), 0x100fd7e0 its fprintf. */
#define SIM_STRCMPI SIM_FN(int(__cdecl *)(const char *, const char *), 0x100ff1e0)
#define SIM_FPRINTF SIM_FN(int(__cdecl *)(void *, const char *, ...), 0x100fd7e0)
#define SIM_CALL_100bc280 SIM_FN(void(__cdecl *)(int, int), 0x100bc280)

/*
 * Table of 0x74-byte records at 0x109b68e0, [0x1015f3ac] of them. When index < 0x10 (signed) and a record's first string equals
 * `name` ignoring case: dword [0x109b7d58 + 0x6c * index] = its number, the record is copied to 0x109b8420 + 0x74 * index
 * and 1 is returned. Otherwise 0.
 */
int __cdecl Sim_1005259e(int index, const char *name);
/*
 * Allocates a zeroed 0xf-byte node (Heap_AllocZero): dword +0 = 0, word +4 = the word at src[0], bytes +6..+13 = the 8
 * bytes at src + 2 (strncpy rules), byte +0xe = 0, and appends it to the chain whose first node is [0x1015f3b0] and
 * whose last node is [0x109b6880] (the last node's dword +0 points to the new one). Returns 1. When the allocation fails
 * runs 0x100bc280(0x42, 0) and returns 0.
 */
int __cdecl Sim_10053010(const uint16_t *src);
/* When [0x101668f4] and [0x101668f8] are not 0: fprintf((FILE *)[0x101668f8], "%s", text) and 1; else 0. */
int __cdecl Sim_1005f9aa(const char *text);

#endif
