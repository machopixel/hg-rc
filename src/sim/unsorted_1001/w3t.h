/*
 * Round 3 (letter t) functions of HGSIM.DLL in 0x1001xxxx. Plain C, original
 * cdecl convention. Names carry the original address because the code alone
 * does not say what the data is for.
 */
#ifndef HG_SIM_UNSORTED_1001_W3T_H
#define HG_SIM_UNSORTED_1001_W3T_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Sets up obj from a record of the table at the pointer in 0x10130cd4 (0x1c
 * bytes each, number in the signed byte obj+0x1b9) and the byte obj+0x1b8
 * (0, 1 or 2) and calls Sim_100d60b6(*(obj+0x28), 0) when the record's dword
 * +8 (stored at obj+0x1c2) is not zero.
 */
void __cdecl Sim_1001139e(void *obj);

/*
 * Float from a 16-bit key: group 0x100 of the key reads the float at
 * 0x109a8650 + 0x54 * (key & 0xff), group 0x200 gives 100.0, group 0x400 gives
 * 10000.0, any other group 100.0. A value below 100.0 (or NaN) becomes 100.0.
 * The float is returned in st(0).
 */
float __cdecl Sim_10014061(int16_t key);

/*
 * The float obj+0xe8 (obj+0x110 when obj+4 is the player index 0x1017b8c8 and
 * the owner obj+0x28 has a nonzero dword at +0x50c) minus the float obj+0x68,
 * moved into -180..180 by one step of 360. Returned in st(0).
 */
float __cdecl Sim_10015266(const void *obj);

/*
 * Interpolates in a table of keyed vectors. table[0] is the count n, then n
 * entries of 16 bytes {int key, float x, y, z} start at table+4. t is reduced
 * modulo period; an exact key copies its vector to out, otherwise the vector is
 * blended between the entries around t (the table wraps around: the entry
 * before the first is the last one, the one after the last is the first with
 * period added to its key). Returns 1 without writing for a null out or n < 1.
 */
int __cdecl Sim_1001be08(int t, int period, const int32_t *table, float *out);

/*
 * Loads the cached file `name` (cache loader 0x10003413, with the string
 * constant at 0x10131944) into the slot `obj` (0x320 bytes): the name goes to
 * obj+4, the image pointer to obj+0x14 and obj+0x18, then the image is parsed
 * in place: its dword +0xc is a count of blocks of {0x20 bytes whose first
 * string is padded to at least 8 characters with the string at 0x10131948,
 * a count and 16-byte entries, a count and 20-byte entries}, pointers to the
 * blocks go to obj+0x1c + 0xc * i (and +0x20, +0x24); obj+0x31c gets the pointer
 * that follows them. Sets obj+0 to 1 and increments the counter at 0x1015d18c.
 * Returns 1 for a null argument or when the file cannot be loaded, else 0.
 */
int __cdecl Sim_1001b705(const char *name, void *obj);

/* Loads the file into the first of the 0x80 slots (0x320 bytes from 0x10a1eaa0) whose dword +0 is 0; returns the slot or NULL. */
void *__cdecl Sim_1001b880(const char *name);

#endif
