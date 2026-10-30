/*
 * Round 3 (letter o): 0x100556c2.
 */
#ifndef HG_SIM_UNSORTED_1005_W3O_H
#define HG_SIM_UNSORTED_1005_W3O_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif


/*
 * For i from [0x10164bc0] - 1 down to 0: when the dword at 0x1015f868 + 4 * i is non-zero, calls the function
 * stored at +0xc of the structure pointed to by the dword at 0x1015fd20 + 4 * i, with the record at
 * 0x1015f880 + 0x74 * i as argument.
 */
void __cdecl Sim_100556c2(void);

#endif
