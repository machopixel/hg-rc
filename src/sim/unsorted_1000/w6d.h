/*
 * Round 6, letter d: functions in 0x1000799a..0x1000ea40 that call helpers which are not converted yet (called
 * through their original address with SIM_FN, prototypes below) besides converted ones.
 */
#ifndef HG_SIM_UNSORTED_1000_W6D_H
#define HG_SIM_UNSORTED_1000_W6D_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Patched by another hook file (0x100ef727) and not converted (0x100e86a1); seven dwords each. */
#define W6D_CALL_100ef727 SIM_FN(int(__cdecl *)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t), 0x100ef727)
#define W6D_CALL_100e86a1 SIM_FN(int(__cdecl *)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t), 0x100e86a1)
/* Not converted: one pointer, returns a byte count that Sim_100097d3 adds. */
#define W6D_CALL_1002ff6e SIM_FN(uint32_t(__cdecl *)(void *), 0x1002ff6e)
/* Not converted: two dwords. */
#define W6D_CALL_10039708 SIM_FN(int(__cdecl *)(uint32_t, uint32_t), 0x10039708)
/* Not converted: twelve dwords (the third is the name index from Sim_1000df9c). */
#define W6D_CALL_1000e737 SIM_FN(int(__cdecl *)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t), 0x1000e737)

/*
 * 256 calls in a 16 x 16 grid, two per entry (rows y and y + 1): 0x100ef727(0x109e3e00, x, y + j, x + 3, y + j, 0,
 * the word i of the table 0x101659e0). x starts
 * at 10 and grows by 4, y starts at 10 and grows by 2 after every 16 entries.
 */
void __cdecl Sim_1000799a(void);
/*
 * Byte count: 0xb4 (+0xa4 when obj[0x28] is set), then for every block of the list at obj[0x2c] (next at +0xc):
 * 0x18 + 0x58 per entry (word count at +4) + 0x28 + the word at +2 of every 0x28-byte record (word count at +6,
 * records at the block + the dword at +8); plus 0x1002ff6e(obj).
 */
uint32_t __cdecl Sim_100097d3(void *obj);
/*
 * When [0x1015d0a4] is set: for every i below its dword +4 whose dword +0x2d4 + 4 * i equals Sim_1000df9c(name),
 * calls 0x10039708(its dword +0x94 + 8 * i, value).
 */
void __cdecl Sim_1000e9c9(const char *name, uint32_t value);
/*
 * 0 when [0x1015d0a4] is null, else 0x1000e737(a, b, Sim_1000df9c(name), 0, c, [0x101321b4], d, Sim_1000edce(),
 * -1, e & 0xffff, 2, f) when its dword +0x10 is set, or with the last two 0, 0; returns what 0x1000e737 returns.
 * Sim_1000df9c is called first in both cases.
 */
int __cdecl Sim_1000ea40(uint32_t a, uint32_t b, const char *name, uint32_t c, uint32_t d, uint32_t e, uint32_t f);

/*
 * Picks the best of the 0x128-byte records of the block obj[0x28] (the selection obj[0x28][0x4fc] steps with
 * Sim_100d5cc1(block, 1) until it is back where it started or all [0x4f8] were tried): every record that
 * Sim_1000b9f1 accepts gets a score from its dword +0x20, the bits 0x40 and 1 of its table entry (Sim_1000c749),
 * its kind (Sim_1000c7b4 against Sim_1000c627(obj)), the entry's dword +0x78 and Sim_1000c813, Sim_1000c680 and bit
 * 0x400 of obj[0xf0]; the last record with the highest score is selected. For a selection of kind 0x16 / 0x14 /
 * 0x10 the float *obj[0x54] grows by 1.875 / 3.75 / 0 (the floats 0x1011106c, 0x10111070, 0x10111060), and *flag
 * becomes 1 when (+0x86 & 1) is set in the entry of the last accepted record or of the selected one (*flag is
 * cleared first). Without a pick the selection is put back. Returns 1 when a record was picked.
 */
int __cdecl Sim_1000bba7(void *obj, uint32_t *flag);
/*
 * 0 for a null obj. For the selected record (+0x4fc) of the block obj[0x28] and its table entry: 1 when
 * Sim_1000a9bf(obj, entry) is 0 or 3, when Sim_1000c627(obj) differs from Sim_1000c7b4(the record's kind), when the
 * record's dword +0x20 is <= 0, or when Sim_1000ce13(obj, records, record, [0x4f8]) is non-zero; else 0.
 */
int __cdecl Sim_1000ceb7(void *obj);

#endif
