/*
 * Round 7, letter l: functions in 0x100071fb..0x1000cfe2. Helpers that are not converted (or that another
 * round-7 list converts) are called through their original address with SIM_FN; prototypes below.
 * Floating point follows the original x87 code with the game's control word (53-bit precision): every
 * intermediate is a double, and only a store to a float rounds to float (see util/w3o.h for the FCOM idioms).
 */
#ifndef HG_SIM_UNSORTED_1000_W7L_H
#define HG_SIM_UNSORTED_1000_W7L_H

#include <stdint.h>
#include "../simmem.h"
#include "../libm/hg_math.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* The C library sprintf of the DLL (0x100fc780). */
#define W7L_SPRINTF_1000 SIM_FN(int(__cdecl *)(char *, const char *, ...), 0x100fc780)
/* The C library sqrt of the DLL (0x100fca04), on a double; the result comes back in st(0). */
#define W7L_SQRT_1000 hg_sqrt
/* Not converted: table lookup of one float, returns a float value in st(0) (0x10084766). */
#define W7L_CALL_10084766_1000 SIM_FN(double(__cdecl *)(float), 0x10084766)
/* Not converted: table lookup of two floats, returns a float value in st(0) (0x1008486e). */
#define W7L_CALL_1008486e_1000 SIM_FN(double(__cdecl *)(float, float), 0x1008486e)
/* The routine whose address 0x10007c9e hands to Sim_1000951c (the jump stub 0x100021b7 of 0x10007f49). */
#define W7L_FN_100021b7 SIM_FN(void(__cdecl *)(void *), 0x100021b7)

/*
 * For the 0x200 x 0x20 entries of 0x18 bytes at 0x10a51ac0 (0x300 bytes per row): an entry with a dword +0x14
 * gets Sim_100bb2b0(entry, [0x1014ceb8]) and +0x14 = 0; every entry gets byte +0 = 0 and word +0x10 = 0. Then
 * Sim_100bb2b0(0x10a3ae80, [0x1014cf0c]) and the 0x200 records of 0xe bytes at 0x10ac1b00 become
 * (word 0xffff, 0, 0, 0, 0xfffe, dword 0xffffffff).
 */
void __cdecl Sim_100071fb(void);

/*
 * Only when [0x10a3ade8] or [0x1015d038] is set: clears the dwords 0x1015d010, 0x1015d018, 0x1015d00c,
 * 0x1015d014, calls Sim_10087c86, then Sim_1000951c(list, 0x100021b7) for every list of the null-ended table
 * 0x1036f2a0 and for [0x1017c8d0]. With [0x10a3ade8] set it prints the counters on the mono screen: the two
 * title lines and (with [0x1015d038]) Sim_1007df6c of the summary text once ([0x1015d03c] 0, then 1), then the
 * four counters (format "%4.4d").
 */
void __cdecl Sim_10007c9e(void);

/*
 * For the model at obj+0x2c (word +4 = count of 0x58-byte points from +0x18, coordinates at +0xc/+0x10/+0x14):
 * the centre of the bounding box of the points (seeded with the last point) goes to the floats
 * obj+0x44..0x4c and is copied to obj+0x38..0x40; obj+0x50 = sqrt of the largest squared distance of a point
 * from that centre, divided by the float at 0x1011100c when the byte obj+0xa0 is 'w' or 'W', or by the double
 * at 0x10111010 when the text at obj+0xa0 contains "ENBP".
 */
void __cdecl Sim_100089c6(void *obj);

/*
 * *a and *b = 10.0, or (when the dword +0xf0 of obj[0] has bit 0x200) the float from 0x1008486e(the float
 * +0x53c of the block +0x28 of record PTRTAB(that dword & 0xff), the float +0xd8 of obj[0]) limited as in the
 * code (floats 0x10111048..0x10111050: 10.0 or 3.0 outside). Both are then scaled by the float 0x10111054 when
 * dword [obj[0]] is 1 and obj+0x4fc equals obj+0x5fb, or else by the float 0x10111058 when
 * Sim_1000c749(obj, obj+0x4fc, 1) is non-zero.
 */
void __cdecl Sim_1000ae6e(void *obj, float *a, float *b);

/*
 * Builds the table of 0xa8-byte records at 0x10a3a600 (count +1 in [0x1015d0a0] per record) from the 0x2e
 * entries of 0xd6 bytes at 0x10145508 that have bit 0x40 in their dword +0x86: dword +0 = entry number, 20
 * angle steps at +0x58 and 20 values at +8 computed with the lookup 0x10084766 from the entry's dword +0x38,
 * floats +0x3c and the constants at 0x1013abc0, 0x10111088..0x10111094 (entries with +0x10 == 0x45 use other
 * steps and an extra offset and scale).
 */
void __cdecl Sim_1000c894(void);

/*
 * When the word obj+0x1aa has 0x200 in bits 8..11: 0x1008486e(the float +0x5c of record PTRTAB(low byte of
 * that word) minus the y coordinate given by Sim_100a9cfb(the pointer +0x494 of block obj+0x28, or obj+0x48
 * when that is null), the float obj+0xd4); otherwise 0. Returned as a float in st(0).
 */
float __cdecl Sim_1000cfe2(void *obj);

#endif
