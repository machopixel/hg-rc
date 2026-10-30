/* Wave 2e, original 0x100a7da3. */
#ifndef HG_SIM_UNSORTED_100A_W2E_H
#define HG_SIM_UNSORTED_100A_W2E_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * The record at the dword 0x101c65e6 + index * 0x14a (a pointer) has a ushort count at +0xc
 * and entries of 0x18 bytes from +0xe. Returns the number of the first entry whose first four
 * bytes equal those of the string key, or 0xFFFF when none (the upper 16 bits of eax are 0).
 */
uint32_t __cdecl Sim_100a7da3(int index, const char *key);

#endif
