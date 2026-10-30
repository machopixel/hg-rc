/*
 * Round 7, letter q: functions of HGSIM.DLL at 0x100c81ed..0x100ccb0d.
 *
 * Entities are the pointers of the table 0x109e8b40 (PTRTAB), the current one is index [0x1017b8c8]; an
 * entity's position is the three floats at +0x58, +0x5c, +0x60. The relative-position functions below copy a
 * target position, subtract the entity position (one float subtraction each, exact in single precision, so the
 * C matches the x87 original bit for bit) and hand the three differences to 0x100cbc6b, which answers four floats
 * through pointers.
 *
 * Callees that are not converted are called through their original address (SIM_FN):
 *   0x100c7deb (object)
 *   0x100cbc6b (dx, dy, dz as float bits, four float out pointers)
 *   0x100eccf3 (table, character) -> value
 */
#ifndef HG_SIM_UNSORTED_100C_W7Q_H
#define HG_SIM_UNSORTED_100C_W7Q_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

typedef void(__cdecl *W7qObjectFn)(void *obj);
typedef void(__cdecl *W7qRelativeFn)(uint32_t dx, uint32_t dy, uint32_t dz, uint32_t *a, uint32_t *b, uint32_t *c,
                                     uint32_t *d);
typedef uint32_t(__cdecl *W7qGlyphFn)(void *table, uint32_t c);

#define W7Q_CALL_100c7deb SIM_FN(W7qObjectFn, 0x100c7deb)
#define W7Q_CALL_100cbc6b SIM_FN(W7qRelativeFn, 0x100cbc6b)
#define W7Q_CALL_100eccf3 SIM_FN(W7qGlyphFn, 0x100eccf3)

/* ---- w7q_objects.c ---- */
/*
 * For an object with a non-zero word +4 while the dword 0x10151cf8 is set, and a pair pointer at +0xf4: resets
 * the pair (Sim_1008f630) unless the dword +0xf8 is 1, asks Sim_1008f867(0, pair) for a five-dword record and,
 * when there is one, runs 0x100c7deb(obj) with that record copied into the dwords at 0x1018548c and into the
 * record [obj+0xec] (both are put back afterwards); without a record 0x100c7deb runs as is. Then stores the
 * dword 0x109e89d8 in +0xf8 (also when the pair pointer is 0).
 */
void __cdecl Sim_100c81ed(void *obj);
/*
 * Like Sim_100c81ed with Sim_1008f867(1, pair); the pair is reset unless +0xf8 is 0, 3 or 4, and 0x100c7deb
 * only runs when there is a record.
 */
void __cdecl Sim_100c82e8(void *obj);
/*
 * Sums 0x100eccf3(table, byte) (or Sim_100f2cec while the dword 0x101668e8 is set) over the bytes of the string
 * s. While 0x101668e8 is set a byte that starts a two-byte character (0x81..0xff except 0x85, 0x86, 0xeb, 0xec,
 * 0xa0..0xdf and 0xef..0xf9) also skips the byte after it.
 */
int __cdecl Sim_100ccb0d(const char *s, void *table);

/* ---- w7q_relative.c ---- */
/*
 * For the entry of the table 0x109a8640 (0x54 bytes) chosen by the low byte of the current entity's dword
 * +0x118: 0 when its dword +0 is 0. Otherwise refreshes its position (+0x18..+0x20) from the object at +4
 * (Sim_100a9cfb) when there is one, and stores the position, the four answers of 0x100cbc6b and the entity's
 * +0x118 in the dwords 0x1015e4c0..0x1015e4dc. When the third answer is below the entry's float +0x10 (or not
 * comparable), the entity's block (+0x28) has no 1 at +0x50c and the entry's word +0x24 lacks 0x20: sets that
 * bit, sets bit [entity+8] in the word +0x26, calls Sim_100cbdb8(entity, 1, 0) and Sim_1000ebfb with the name at
 * 0x10151d34. Returns 1.
 */
int __cdecl Sim_100ca996(void);
/*
 * Checks entry `index` of the table 0x109a8640 for entity `who` (-1: index below 0, -2: not below the count
 * 0x1017e5a8, -9: flags 0xe, -3: empty entry, -4: flag 0x10000 without bit 0x40 in the word +0x24, -6: owner
 * +8 is not who|0x200 for entries with bit 1, or flag 0x100 for the others, or +0xc differs from the entity's +8).
 * Then works out the relative position as Sim_100ca996 and stores position and answers in the entity's dwords
 * +0xd4..+0xec and +0xfc..+0x114. Returns 1.
 */
int __cdecl Sim_100cab7b(int who, int index, uint32_t flags);
/*
 * The same for entry `index` of the table 0x109e9ba0 (0x50 bytes, count 0x1015dc68, word flags +0), whose
 * position comes from Sim_100b53f0 of its +8 (which Sim_100b5386 and Sim_100b53bb must accept, else -8).
 * Flags 0x23 give -9, 0x20000/0x40000 require Sim_10052c02(index) to be 0/1 (else -9); the current entity
 * gets -5 for entries it may not choose, and -7 when the entry lacks 0x1000 and the second answer is above the
 * float 0x1012bc48. On success stores into the entity's dwords +0xd4..+0xec and returns 1.
 */
int __cdecl Sim_100cb299(int who, int index, uint32_t flags);

#endif
