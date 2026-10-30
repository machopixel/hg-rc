/*
 * Wave 2 (letter i) additions to unsorted_100d: linked nodes, a rect middle, a constant table and the record selection around 0x100d1000..0x100d8300.
 * Plain C, original cdecl convention. Names carry the original address
 * because the code alone does not say what the data is for.
 */
#ifndef HG_SIM_UNSORTED_100d_W2I_H
#define HG_SIM_UNSORTED_100d_W2I_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* ---- w2i_chain.c ---- */
/*
 * Nodes are linked by the dwords at +4 and +8, and the dword at +0x14 of a node
 * is an owner pointer. Both functions find `first` (the end of the +4 links
 * from chain) and `last` (the end of the +8 links from chain) and set the +0x14
 * of the nodes from first along the +8 links.
 *
 * Sim_100d16e4: sets them to owner; first[4] = owner[0x10]; owner[0x10] = last;
 * then owner[0xc] = first when that was 0, else first[4][8] = first.
 * Returns last.
 */
void *__cdecl Sim_100d16e4(void *owner, void *chain);
/*
 * Sim_100d1834: sets them to owner[0x14]; last[8] = owner; first[4] = owner[4];
 * owner[4] = last; then first[4][8] = first, or owner[0x14][0xc] = first when
 * first[4] is 0 and owner[0x14] is not. Returns first.
 */
void *__cdecl Sim_100d1834(void *owner, void *chain);
/* *out1 = (dword 0x2c + dword 0x30) / 2 + dword 0x4c of obj; *out2 = (0x34 + 0x38) / 2 + 0x50. */
void __cdecl Sim_100d339e(void *obj, int *out1, int *out2);

/* ---- w2i_table.c ---- */
/*
 * Copies the 5 dwords at src into each of the 11 slots of 0x14 bytes from
 * 0x10185400, overwrites 32 dwords of the slots with constants, and clears the
 * byte at 0x10185620 + 16 * i for i < 20.
 */
void __cdecl Sim_100d3142(const uint32_t *src);

/* ---- w2i_select.c ---- */
/*
 * A block has a count at +0x4f8 (the functions do nothing for 0 or less), a
 * selected record number at +0x4fc and a pointer at +0x84 to 8 records of 0x128
 * bytes. A record has dwords at +0 (-1 = unused), +4 (index into the 0xd6-byte
 * table at 0x10145508, negative = none), +8 (-1 = none) and +0xc (compared
 * between records). Bit 2 of the table entry's dword at +8 gates a record.
 */
/* For each i < 8 with a flag set in the dwords at 0x101853c0 (cleared here): sets the record's +8 to 2, +0x2c from its table entry, +0x10 to 0 and +0x18/+0x1c. */
void __cdecl Sim_100d63fa(void *block);
/* Looks round the other 7 record numbers after the selected one for a record with the same +0xc and a gate bit; selects it, else calls Sim_100d5f40 when flag is non-zero. */
void __cdecl Sim_100d5cc1(void *block, int flag);
/* Advances the selected record number (mod 8) until its record is in use, has a table entry and passes the gate; ends early on [0x10179ccc] with [[block]+4] == [0x1017b8c8]. */
void __cdecl Sim_100d5de5(void *block);
/* Starting with the +0xc value after the selected record's, finds the first i whose record has that +0xc value and the gate bit, and selects it. */
void __cdecl Sim_100d5f40(void *block);
/* Sim_100d5f40 of the block (dword +0x28) of the table 0x109e8b40 entry [0x1017b8c8]. */
void __cdecl Sim_100d63d4(void);
void __cdecl Sim_100d82f1(void); /* does nothing */

#endif
