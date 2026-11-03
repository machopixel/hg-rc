/*
 * Round 5, letter c: functions at 0x1003a052..0x10041247 that call helpers which are not converted yet.
 * Those helpers are called through their original address (SIM_FN) with the prototypes below.
 */
#ifndef HG_SIM_UNSORTED_1003_W5C_H
#define HG_SIM_UNSORTED_1003_W5C_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Not converted: takes the address of the record at 0x109e369c. */
#define SIM_CALL_1003a548 SIM_FN(void(__cdecl *)(void *), 0x1003a548)
/* Not converted: (dword of record +0x28, dword of the message at +3, 0). */
#define SIM_CALL_10038913 SIM_FN(void(__cdecl *)(uint32_t, uint32_t, uint32_t), 0x10038913)
/* Not converted: takes the dword at record +0x48. */
#define SIM_CALL_100aa799 SIM_FN(void(__cdecl *)(uint32_t), 0x100aa799)

/* Not converted: no arguments. */
#define SIM_CALL_1008a5e3 SIM_FN(void(__cdecl *)(void), 0x1008a5e3)
/* Not converted: (address of the 0xf0-byte block at 0x109e3e00, 0xf0). */
#define SIM_CALL_100eb070 SIM_FN(void(__cdecl *)(void *, int), 0x100eb070)

/*
 * When the dword at 0x109e369c is non-zero hands that record to 0x1003a548. Then, if the table at [0x1015d0a4] exists,
 * clears its dword slots (count at +4, slots from +0x54), frees it with HeapFree and returns 1; otherwise returns 0.
 */
int __cdecl Sim_1003a052(void);
/* Message with a slot index in byte +2: unless that slot's record is missing or refers to the current dword [0x1017b8c8] (at +4, or at +0xc when +0x14 is 3) calls 0x10038913 and 0x100aa799. */
void __cdecl Sim_10041247(const uint8_t *msg);

/*
 * Shutdown sequence: 0x1008a5e3(), Sim_100a87f0(), the function stored at 0x109e3d2c (a renderer plugin slot) when set;
 * then frees the block [0x1015e534] with HeapFree
 * (if set), clears [0x1015e538] and calls Sim_10037c6c().
 */
void __cdecl Sim_100354ff(void);

#endif
