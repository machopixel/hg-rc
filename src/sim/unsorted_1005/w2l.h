/*
 * Wave 2l: functions around 0x10057a4a..0x1005b89b. Plain C, original cdecl
 * convention.
 */
#ifndef HG_SIM_UNSORTED_1005_W2L_H
#define HG_SIM_UNSORTED_1005_W2L_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Returns 0 (the address is stored in a table of the DLL's data at 0x10143d74). */
int __cdecl Sim_10057a4a(void);
/*
 * 0x100 for kind 2..8, 0xd, 0xe, 0x21..0x25 and 0x28; 0x200 for kind 0x1d..0x20; otherwise `other`.
 */
int __cdecl Sim_1005b89b(int kind, int other);

#endif
