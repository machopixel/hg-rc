/*
 * Round 4, letter v: a function of HGSIM.DLL around 0x10072000 that allocates a record.
 */
#ifndef HG_SIM_UNSORTED_1007_W4V_H
#define HG_SIM_UNSORTED_1007_W4V_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Like Sim_1003730b (record of Sim_10072ff2() bytes, owner[0x28] = record, owner[0x2c] = 0xc51, record[0] = owner,
 * the ten dwords from +0x498 cleared) but the record's buffers are set up by Sim_10072e35. Returns 1, 0 when the
 * allocation fails. The first argument is unused.
 */
int __cdecl Sim_10072d88(uint32_t unused, void *owner);

#endif
