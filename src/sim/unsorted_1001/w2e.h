/*
 * Wave 2e, original 0x10010183..0x1001f6c0: three fillers of a name and a
 * selected index from the table at 0x10473800, a blank-rows routine for the text
 * screen (see unsorted_1003/w2e.h) and two users of the table of Sim_100ca417.
 */
#ifndef HG_SIM_UNSORTED_1001_W2E_H
#define HG_SIM_UNSORTED_1001_W2E_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Three variants of one routine, selected by the offsets they use: obj holds a destination
 * string pointer (+0x1c / +0x44 / +0x6c) and a pointer to the record rec (+0x24 / +0x4c / +0x74)
 * that receives an index at its +0xc. Does nothing when obj or rec is null or rec+8 is 0.
 * Otherwise rec[+0xc] = 0, the string at [0x101c5158] is copied to the destination and
 * every entry i (below the count at the start of the block of 0x4fc3a bytes per
 * [0x1013bb40], entries of 0x13f bytes) whose byte +0x74 is set and whose dword at +2
 * is 0x8000 / 0x8001 / 0x8002 copies its string at +0xad to the destination and
 * stores i in rec[+0xc].
 */
void __cdecl Sim_10010183(void *obj, void *rec);
void __cdecl Sim_10010267(void *obj, void *rec);
void __cdecl Sim_1001034b(void *obj, void *rec);

/* Writes the 64-space string at 0x10130ed0 on rows 3..24 of the text screen (attribute 7) when [0x10a3ade8] is set. */
void __cdecl Mono_BlankRows(void);

/*
 * For every entry i of the 0x54-byte table at 0x109a8640 (count [0x1017e5a8]) whose short
 * +0x24 has bit 0 set and whose dword +8 equals obj[+4] | 0x200: runs Sim_100ca417(obj[+4], i | 0x100)
 * when obj's dword +0x1d6 has bit 0x1000 or its low byte differs from i.
 */
void __cdecl Sim_1001570f(void *obj);
/* If obj's dword +0x1d6 is 0: Sim_100ca417(obj[+4], 0) and +0x1d6 = 0x1000. */
void __cdecl Sim_10017852(void *obj);
/*
 * Fills the 0x4b0 records of 8 bytes at 0x109f1520 with 0xFF and then, for each of the
 * 0x2c entries of 8 bytes at 0x10132248 whose short +0 is in 1..0x4af, stores the
 * shorts +2, +4, +6 of the entry in the shorts +0, +2, +4 of record number +0 and 0 in +6.
 */
void __cdecl Sim_1001f6c0(void);

#endif
