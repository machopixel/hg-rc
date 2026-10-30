/*
 * Wave 2: functions around 0x100c459f..0x100c48fa that call converted
 * helpers. Plain C, original cdecl convention.
 */
#ifndef HG_SIM_UNSORTED_100C_H
#define HG_SIM_UNSORTED_100C_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* 0 when Sim_1003356a(entry) is non-zero and Sim_10033580(entry) differs from the dword at other+4, else 1. */
int __cdecl Sim_100c459f(const unsigned char *entry, const unsigned char *other);
/*
 * Does nothing when obj is null or obj+0x54 is 0. Otherwise calls
 * Sim_100c48c9(obj) and sets the bytes +0x57 and +0x58 of *(obj+0x54):
 * mode 0 gives 1 and 0, mode 1 gives 0 and 1, any other mode gives 0 and 0.
 */
void __cdecl Sim_100c48fa(void *obj, int mode);

#endif
