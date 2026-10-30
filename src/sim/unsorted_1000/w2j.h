/*
 * Wave 2, letter j: leaf functions of HGSIM.DLL around 0x10006000..0x10010000.
 * Plain C, original cdecl convention. Names carry the original address where
 * the code alone does not say what the data is for.
 */
#ifndef HG_SIM_UNSORTED_1000_W2J_H
#define HG_SIM_UNSORTED_1000_W2J_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Steps the 0x200 animation records (0xe bytes each, at 0x10ac1b00) to the
 * time in dword 0x1015dc78. A record is used when its signed word +8 is >= 0
 * and its words +4 (period) and +6 (mode) are non-zero. Each whole period that
 * has passed since the time at +0xa advances the frame (word +2) by one over
 * 32 frames, restarting at 0 when frame `frame` of animation (word +0) has a
 * zero first byte in the table at 0x10a51ac0 (0x300 bytes per animation, 0x18
 * per frame). Mode 2 clears itself when the frame wraps back below its old one.
 */
void __cdecl Sim_10006a2f(void);
/* Stores value in dword 0x1015d09c and returns the previous one. */
uint32_t __cdecl Sim_100089aa(uint32_t value);
/*
 * obj+0x30 points at a block with a count (word +6) and the offset of an array
 * of 0x28-byte entries (dword +8); the entry after the last one is filled in
 * (word +0 = value, dword +4 = (offset - entry) & 0xffff, dword +0x24 = obj,
 * word +2 and dword +8 cleared), the count grows by one and the entry is
 * returned. 0 when obj+0x30 is 0.
 */
void *__cdecl Sim_100091f7(void *obj, unsigned short value, uint32_t offset);
/* The word at +2, zero-extended. */
uint32_t __cdecl Sim_1000943e(const void *obj);
/*
 * Entries of 5 dwords start at +0x18 of obj, 0x14 bytes apart; dword +0xc is a
 * count of them. Returns 0 for a null obj, otherwise 1. Sim_10017892() - 1 is
 * the number of slots of the current group (n). When n is not 0 the entries 5
 * and 7 are copied behind the first n+1, the count becomes n+3, and the entry
 * i+1 gets 1 in its first dword for each i < n with Sim_10015c61(i + 1) a record
 * whose word +0x1a6 is 0xc. If n is 0, or every one of them was marked, entry
 * 0 is copied from +0xb8, entry 1 from +0xa4 and the count becomes 2.
 * The first argument is not used.
 */
int __cdecl Sim_1000fd00(uint32_t unused, void *obj);
/*
 * 1 when `slot` is at most Sim_10017892() - 1 (or is 4) and the record that
 * Sim_10015c61(slot) finds exists and its word +0x1a6 is not 0xc; otherwise 0.
 */
int __cdecl Sim_1000fe60(int slot);
/*
 * Same entry layout as Sim_1000fd00 (obj+8 is the slot number it tests with
 * Sim_1000fe60). Returns 0 for a null obj, else 1. When Sim_10017892() - 1 is
 * 0 entry 0 is copied from +0xb8, entry 1 from +0xa4 and the count becomes 2.
 * Otherwise, when Sim_1000fe60(obj[8]) is 0, the entries `count` and
 * `count - 1` are copied to entries 0 and 1 and the count becomes 2.
 * The first argument is not used.
 */
int __cdecl Sim_1000ff5b(uint32_t unused, void *obj);
/*
 * Clears dwords +0x54 and +0x2d4 of the block at 0x1015d0a4 selected by obj[4],
 * sets obj[4] = -1, calls Sim_1002828e(signed word obj[0x1c], dword 0x1014ceb4)
 * and clears obj[8]. Does nothing when obj is null or obj[8] is 0.
 */
void __cdecl Sim_1000f029(void *obj);

#endif
