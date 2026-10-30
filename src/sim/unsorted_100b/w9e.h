/*
 * Round 9, letter e: functions of HGSIM.DLL at 0x100b43ce (link records) and 0x100b9203 (resource cache).
 * Helpers that are not converted (or patched elsewhere) are called through their original address with the
 * prototypes below.
 */
#ifndef HG_SIM_UNSORTED_100B_W9E_H
#define HG_SIM_UNSORTED_100B_W9E_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Not converted: (link index, dword +0x10 of its record) -> 0 when the link cannot be activated. */
#define W9E_CALL_100b48de SIM_FN(int(__cdecl *)(int, uint32_t), 0x100b48de)
/* The C library's assertion failure (expression, file, line). */
#define W9E_ASSERT SIM_FN(void(__cdecl *)(const char *, const char *, int), 0x100ff860)
/* The comparators handed to the table search and the sorted insertion (original routines). */
#define W9E_CMP_100b971d SIM_FN(int(__cdecl *)(uint32_t, uint32_t), 0x100b971d)
#define W9E_CMP_100b9748 SIM_FN(int(__cdecl *)(uint32_t, uint32_t), 0x100b9748)

/* ---- w9e_cache.c ---- */
/*
 * Releases resources from the cache of the extension `ext` (both texts are upper-cased in place): the table
 * is found with Sim_100bda9f in [0x1017d604]; 0x1017b914 is set to the heap of the kind (WTB with 0x1017b904
 * set, XEL, WAV, 3DI, or MLT/TLT/GDF/BWD/SDF when 0x1017b910 is set) while the entries are freed. With a name
 * (8 bytes: two dwords, hashed bucket = (d0 ^ d1) % table[0]) the first unlocked entry (dword +8 <= 0) of that
 * name in its chain is unlinked (prev +0x20, next +0x1c, head in the bucket array +0x10), the counts +8 and
 * +0xc (its size) of the table reduced and the entry freed with Sim_100b9675(entry, table's +0x20); returns 1,
 * or 0 when there is none. Without a name every unlocked entry is collected into a sorted array of 0x400 (the
 * comparator 0x100b9748; stopping a chain when the array is full) and freed in that order until the cached
 * size +0xc drops below the limit +4; returns 1.
 */
int __cdecl Sim_100b9203(char *name, char *ext);

/* ---- w9e_links.c ---- */
/*
 * Closes the link record `index` (0x88 bytes each at 0x101a2100) and activates its successor (dword +0x20):
 * -1 when the record has no target (dword +0x24). Else the record gets the flags 0x200 and 0x800, its target
 * (0x50 bytes each at 0x109e9ba0) the flag 4 and dword +8 = -1, Sim_100b42aa(index); when the record owns an
 * object (dword +0x28) its node (dword +0x2c, when set) gives the matrix (Sim_100aa863, 0x30 bytes),
 * Sim_100aa807's 0x30 bytes and the position (Sim_100a9ccf), the object is released with Sim_10009787 and both
 * dwords cleared. The successor loses the flag 0x800; when 0x100b48de(successor, its dword +0x10) says 0 its
 * flags are reduced to the bit 0x800 and -1 is returned; else its node (when set) gets the matrix (Sim_100aa87a
 * and Sim_100aa812) and position (Sim_100aa0da), Sim_100b42d5(successor) and Sim_100aa64a(node). Returns the
 * successor.
 * The original passes the position of the closed node even when no node gave one (uninitialised stack): this
 * version passes zeros then; see the round 9 report.
 */
int __cdecl Sim_100b43ce(int index);

#endif
