/*
 * Wave 2e, original 0x10033c24..0x1003ac07. The first group writes to the
 * text screen at physical address 0xB0000 (the monochrome adapter, 80 columns
 * by 25 rows of character/attribute byte pairs) which the debug output of the
 * DLL uses; every use is guarded by the dword at 0x10a3ade8. These functions
 * touch that address directly, so they must only run where it is mapped.
 */
#ifndef HG_SIM_UNSORTED_1003_W2E_H
#define HG_SIM_UNSORTED_1003_W2E_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Writes the characters of s at (col, row) of the text screen with the attribute
 * byte attr (low 8 bits). Does nothing for a column outside 0..79, a row outside
 * 0..24 or an empty string; at most 80 characters are written and a long string
 * continues into the next row's cells.
 */
void __cdecl Mono_PutText(const char *s, short col, short row, short attr);
/* Fills the whole screen (0xFA0 bytes) with 0x20. */
void __cdecl Mono_Clear(void);

/* Clears 0x3524 bytes at 0x109e4cc0 (the entry table counted by [0x109e4cc0]); returns 1. */
int __cdecl Sim_10033c24(void);
/* Copies at most 0x100 bytes of the string to 0x10139e58 (strncpy rules); returns the destination. */
char *__cdecl Sim_100348bd(const char *s);
/* Removes everything up to and including the last backslash of the string, in place. */
void __cdecl Path_StripDirectory(char *path);

/*
 * obj has the pointer at +0x84 to eight 0x128-byte slots and the pointer at +0x8c to
 * 0x32 entries of 0x24 bytes; rec is a block with a 0x128-byte slot image at +0xc and
 * 0x24-byte entries from +0x134. Does nothing unless a slot has dword +4 == -1; then
 * copies rec's image into that slot and fills the free entries of obj from rec.
 */
void __cdecl Sim_10038ad5(void *obj, void *rec);
/* Clears the two 0x40-byte blocks at 0x109e36e0 and 0x109e3720; when [[0x1015d0a4]+0xc] is set, fills in constants. */
void __cdecl Sim_10039725(void);
/*
 * Adds a record to the table of 0x7e-byte entries at 0x109b9e20 (count [0x1015ea80], at
 * most 0x465) or, when a5 is not 0, fills one more name of entry a6. a5 is the
 * index (0..3) of the 8-character name the string goes into. Returns the entry
 * index or -1.
 */
int __cdecl Sim_1003aa7d(const char *name, uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4,
                         int a5, int a6, unsigned short a7, uint32_t a8);
/* Resets all 0x465 entries of that table, sets the count to 0 and the flag [0x1015ea84] to 1. */
void __cdecl Sim_1003ac07(void);

#endif
