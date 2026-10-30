/*
 * Round 6, letter e: functions at 0x100529a2..0x1005ada5. Helpers that are not converted yet are called through their
 * original address (SIM_FN) with the prototypes below.
 */
#ifndef HG_SIM_UNSORTED_1005_W6E_H
#define HG_SIM_UNSORTED_1005_W6E_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Not converted: the C library sprintf (destination, format, ...). */
#define W6E_CALL_100fc780 SIM_FN(int(__cdecl *)(char *, const char *, ...), 0x100fc780)
/* Not converted: (object, index 0..3, 0). */
#define W6E_CALL_1005a91a SIM_FN(void(__cdecl *)(void *, int, int), 0x1005a91a)

/*
 * For entry `index` of the table at 0x109e8b40 (0 when index >= [0x1015dc64]): when 0x10052aaf(entry +8) names
 * another entry, *x, *y, *z come from the tables at 0x109b8430/0x450/0x470 (0x74 bytes per +8 value, dword +0x10
 * selects the column) and (*x, 0, *y) is transformed by the matrix of that entry's object +0x48; otherwise they are
 * dwords +0x58, +0x60 and +0x68 of that entry. Returns 1.
 */
int __cdecl Sim_100529a2(uint32_t index, uint32_t *x, uint32_t *y, uint32_t *z);
/*
 * Calls 0x100579ad with the two pointers at [0x10144230]+2 and +6, then fills the record: "keyboard" at +0 and
 * +0x4c, the string at [0x101c4fdc] at +0xc, 0x7d at +0x5c, the tables 0x10165f60 and 0x10143808 at +0x68 and +0x6c,
 * 0 elsewhere. Returns 0.
 */
int __cdecl Sim_1005790a(uint32_t unused, char *rec);
/*
 * Calls 0x1005a91a(obj, i, 0) for i = 0.. while the 0x20-byte entries at [obj+0x88]+0x7c have a non-zero first byte
 * (at most 4; an entry 'g' or 'G' first is skipped); [0x101c42a0] is put back after every call but the first.
 * Returns the number of calls.
 */
int __cdecl Sim_1005ada5(void *obj);

#endif
