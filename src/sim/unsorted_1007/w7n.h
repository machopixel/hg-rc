/*
 * Round 7, letter n: functions at 0x10072a6b..0x1007e570. Helpers that are not converted yet (or that another
 * hook file patches) are called through their original address with SIM_FN and the prototypes below.
 */
#ifndef HG_SIM_UNSORTED_1007_W7N_H
#define HG_SIM_UNSORTED_1007_W7N_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Patched by hooks_sim.c (system error report): (code, format, ...). */
#define W7N7_CALL_100bc280 SIM_FN(int(__cdecl *)(int, const char *, ...), 0x100bc280)
/* Not converted: the C library sprintf (destination, format, ...). */
#define W7N7_CALL_100fc780 SIM_FN(int(__cdecl *)(char *, const char *, ...), 0x100fc780)
/* Not converted: (entry, index) for the entries of the table 0x10149958 (see Sim_1007520b). */
#define W7N7_CALL_10074cea SIM_FN(int(__cdecl *)(void *, int), 0x10074cea)
/* Not converted: (dword +0xc of a list element of 0x1015dc14), see Sim_10075585. */
#define W7N7_CALL_10075667 SIM_FN(int(__cdecl *)(uint32_t), 0x10075667)
/* Not converted: (object), see Sim_10078950. */
#define W7N7_CALL_100782a9 SIM_FN(void(__cdecl *)(void *), 0x100782a9)
/* Not converted (in the w7m list): (a, four dwords of the 0x54-byte entry), see Sim_10079b68. */
#define W7N7_CALL_100527fa SIM_FN(int(__cdecl *)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t), 0x100527fa)

/*
 * Per-frame step of the state at obj+0x4f0: 5 = starts a 0x389-tick wait ([0x1015dc78]) once, then mode 4 of
 * Sim_100448cd while it runs and [0x10179cd4] = 1 after it; 4 = once ([0x10179d04]) Sim_10061b62, [0x10179cd4] = 1
 * and mode 1; 2 = the "AFXHDWPS" requests of the flags [obj]+0x54 bytes +0x27/+0x28/+0x2c and [0x10179ccc] /
 * [0x10179cbc]. Then [0x10179cd8] = -1 sets state 3, and 1 (state 3 and the word +0x560 bits 4/8 allowing it)
 * shows the text [0x101c464c] with Sim_1007df6c, sets state 0 and clears bit 0x10 of the word [obj]+0x1c.
 */
void __cdecl Sim_10072a6b(uint8_t *obj);
/*
 * For the block of the current entry of 0x109e8b40: Sim_1002ba28(block +0x613), Sim_1002b92d, then for each of
 * the 14 entries of the table 0x10149958 that has +0 and +0x18 set: stores 0x109e3e20 through +0x18 and that value
 * through +0; 0x10074cea(entry, index); then the check functions (+0x14) of the block +0x80 and of its
 * +0xc sub-blocks (pointers at +0x28 + 0x14 * i) are called with (entry, block); an entry for which any check
 * gives 0 (or that has no block) is removed from the table.
 */
void __cdecl Sim_1007520b(void);
/*
 * Walks the list at 0x1015dc14 (next at +0x14) bringing each element's state +4 to the wanted state +8
 * (1: Sim_10075063 must succeed, 0: Sim_10075124); at the first element that is in state 1 calls 0x10075667 with
 * +0xc (unless bit 2 of [+0xc]+4 is set and [0x109e3dac] is non-zero), which clears the wanted state on failure,
 * and stops.
 */
void __cdecl Sim_10075585(void);
/*
 * For an object with word +4 set and dword +0x30 not 0, 1 or 2 that has a pair +0xf4: calls 0x100782a9(obj),
 * with the 20-byte entry of 0x10185400 at index +0xc and the 20 bytes at [+0xec] replaced by the rectangle of
 * Sim_1008f867(0, pair) for the call when there is one (the pair is first passed to Sim_1008f630 unless +0xf8 is
 * 1). Then +0xf8 = [0x109e89d8].
 */
void __cdecl Sim_10078950(uint8_t *obj);
/*
 * For each group i < [0x104737e0] with a name table (0x1017ee40, names at +0x70, 0x97 bytes apart): marks every
 * record j (0x13f bytes at 0x10473800 + 0x4fc3a * i, count at +0) in state 0, 1 or 7 (byte +0x3a) whose name is
 * `name` (case-blind) with state 7. Returns the bits 1 << j and (1 << i) << 16 of the marked records (shift counts
 * taken modulo 32 as the processor does); 0 for an empty name.
 */
uint32_t __cdecl Sim_1007950a(const char *name);
/*
 * For each record of the 0x400 of each group (see Sim_1007950a) in state 7 named `name` whose mask (dword +0x3c)
 * shares a bit with `bits` and that holds fewer than 0x28 values (byte +0x128): appends the low word of value to
 * its word array at +0x129 and calls Sim_1007cbb9.
 */
void __cdecl Sim_10079664(const char *name, uint32_t bits, uint32_t value);
/*
 * Stores [0x1015dc78] / 0xb5 in [0x1037305c], +4 and +0xa1 of obj after calling 0x100527fa(a, ...) with the four
 * dwords +4, +8, +0xc, +0 of the 0x54-byte entry 0x109a8654 at index byte +0x129 (when byte +0x128 is non-zero,
 * byte +0x12a is 1 and the index is below [0x1017e5a8]; else four zeros). Returns 1 when the entry was used.
 */
int __cdecl Sim_10079b68(uint8_t *obj, uint32_t a);
/*
 * The project file entry (20 bytes at 0x10179d78: name, next at +0x10) named `name` (case-blind) in the hash table
 * at 0x1017ad18 (0x65 chains; hash: each byte | 0x20 added, then the low word rotated left by one); when there is
 * none and `create` is set a new one is added ([0x1017afb0] in use, "Too many project file entries" reported from
 * 200 on); else 0.
 */
uint8_t *__cdecl ProjFile_FindEntry(const char *name, int create);

#endif
