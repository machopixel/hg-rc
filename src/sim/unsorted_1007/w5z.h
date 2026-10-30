/*
 * Round 5 (letter z): small functions of HGSIM.DLL at 0x10074c3d and 0x100788ba.
 * Calls of functions that are not converted yet go through their original
 * address (SIM_FN); the others call the converted C functions by name.
 */
#ifndef HG_SIM_UNSORTED_1007_W5Z_H
#define HG_SIM_UNSORTED_1007_W5Z_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Asks the original 0x100766ca(key): for 0 calls Sim_10074bfa(key) and returns 1; for 1 calls Sim_100751e4(key)
 * and returns 0; any other answer returns 0.
 */
int __cdecl Sim_10074c3d(int32_t key);

/* When the signed word at obj+4 and the dword at obj+0x30 are not 0: sets obj+0xf8 from dword 0x109e89d8 and calls the original 0x1002c8cb(dword at obj+0xec, 0, 0, 0). */
void __cdecl Sim_100788ba(void *obj);

#endif
