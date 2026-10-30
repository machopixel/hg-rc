/*
 * Round 3 (letter t) functions of HGSIM.DLL in 0x1000xxxx. Plain C, original
 * cdecl convention. Names carry the original address because the code alone
 * does not say what the records are for.
 *
 * Shared layout (proved by the code below and by w2f.h / w2i.h of
 * unsorted_100d): an object has a pointer at +0x84 to 0x128-byte records,
 * their count at +0x4f8 and the number of the current record at +0x4fc
 * (-1 = none). A record's dword +4 is an index into the 0xd6-byte table at
 * 0x10145508 (called "entry" below).
 */
#ifndef HG_SIM_UNSORTED_1000_W3T_H
#define HG_SIM_UNSORTED_1000_W3T_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Compares the float at obj+0xd8 with a range taken from a table entry: the
 * lower limit is the float at entry+0x4c, the upper limit is the float at
 * entry+0x50 when the bit with value 0x40 of the dword entry+0x86 is set, else
 * the smaller of that and (int at entry+0x5c) * (float at entry+0x3c).
 * Returns 3 below the lower limit, 2 below the upper limit (a NaN counts as
 * below), 1 when equal to the upper limit, 0 above it.
 */
int __cdecl Sim_1000a9bf(const void *obj, const void *entry);

/*
 * 1 when the current record of obj (obj+0x4fc) passes: Sim_1000a9bf(*obj, entry)
 * is 1 or 2, Sim_100d6200(obj) is 1, the dword at record+0x20 is not zero and
 * the int at obj+0x4dc is not above the int at 0x1015dc78. Then the optional
 * outputs receive the record number, its entry index (record+4), the entry
 * address and the record address. Returns 0 for a null obj, with no current
 * record, and (before looking at the record) when *obj is of kind 3 (dword
 * +0x14) and the owner it names in word +0x1aa (low byte, group 0x200 in the
 * next nibble) is another object that is of kind 0 or 1.
 */
int __cdecl Sim_1000b9f1(void *obj, int *record, int *entryIndex, uint32_t *entry, uint32_t *recordAddr);

/*
 * Walks the records of the object obj+0x28 (the owner) and marks (record+0xc
 * = 1) those whose entry gives Sim_1000a9bf(obj, entry) == 2 (and record+0x20
 * != 0, record+8 != -1), as long as the sum of the floats owner+0x4e8 and the
 * entries' +0x44 stays below the int obj+0x1e4 (not checked when obj+4 is the
 * player index 0x1017b8c8). Adds a bonus to the float obj+0x54 points to,
 * and selects the first marked record whose entry has bit 1 of entry+0x86 and
 * record+8 == 1 (else lets Sim_100d60b6 pick one; when nothing is marked
 * Sim_100d5cc1 runs). *flag is set to 1 when a record was selected. Returns
 * 1 when anything was marked.
 */
int __cdecl Sim_1000bf73(void *obj, int *flag);

/*
 * Looks for the first of the entries (0xa8 bytes each, count at 0x1015d0a0,
 * from 0x10a3a600) whose first dword equals key. Then the float
 * (value - float at 0x10145554 + 0xd6 * key) * (float at entry+4) - 1.0 is
 * truncated, limited to 0..0x13, and the dword at entry+0x58+4*that is stored
 * in *out. Returns 1 when an entry was found, else 0.
 */
int __cdecl Sim_1000cd55(int key, float value, uint32_t *out);

/*
 * 1 when one of the `count` 0x128-byte records at recs, other than skip, has an
 * entry whose dword +0x78 is above (unsigned) that of the entry of skip and
 * Sim_1000a9bf(obj, entry) is 2. Stops at the first.
 */
int __cdecl Sim_1000ce13(const void *obj, const void *recs, const void *skip, int count);

#endif
