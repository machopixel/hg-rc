/*
 * Wave 2g: functions of HGSIM.DLL from 0x10041e61..0x1004f1df. Names carry the
 * original address because the code alone does not say what the objects are.
 */
#ifndef HG_SIM_W2G_1004_H
#define HG_SIM_W2G_1004_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Does nothing unless [0x1015ec58] is set. Otherwise: clears bits 0x4800 of
 * the word at +0x1c of the object the table at 0x109e8b40 gives for the index
 * [0x1017b8c8]; `mode` = the byte at +0x1c of the 0x1e-byte record
 * Sim_1003dbc0(index) of the table at root[0x126] (root = [0x1015d004]); for each of
 * the root[0x12] records (word at +0x1a = object index) sets the dword at
 * 0x109b7d54 + 0x6c * object[2] to 0 or 1 (by comparing the record's +0x1c
 * byte with mode when the byte at [0x1017b8c4] + 0xa is non-zero, otherwise
 * by object[1] == index); then for each of the [0x1015dc64] objects clears
 * +0x160 and +0x164, clears +0x18 when +0x14 is 3 and sets +0x168 to
 * root + 0x138 or root + 0x137; finally stores object[2] of the current object
 * in [0x1013bb40], sets the byte at [0x1017b8c4] + 8 when root[0x137] or
 * root[0x138] is non-zero, and sets [0x1015ec54] = 1.
 */
void __cdecl Sim_10041e61(void);
/* The group of a code (low byte): 0..8 -> 0, 10..13 -> 1, 14..17 -> 2, 18..21 -> 3, anything else -> 4. */
int __cdecl Sim_1004f1df(int code);

#endif
