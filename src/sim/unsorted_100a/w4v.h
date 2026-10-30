/*
 * Round 4, letter v: a function of HGSIM.DLL around 0x100a9000 that allocates a record.
 */
#ifndef HG_SIM_UNSORTED_100A_W4V_H
#define HG_SIM_UNSORTED_100A_W4V_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Sim_1003730b without the entry loop: allocates the record, fills +0x84/+0x88/+0x8c, record[0] = owner, clears the
 * ten dwords from +0x498, sets owner[0x28] = record and owner[0x2c] = 0xc51, then Sim_10072e35(record). Returns 1,
 * 0 when the allocation fails. The first argument is unused.
 */
int __cdecl Sim_100a9017(uint32_t unused, void *owner);

#endif
