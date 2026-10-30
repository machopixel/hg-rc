/*
 * Wave 2g: 0x100cb9a4 of HGSIM.DLL.
 */
#ifndef HG_SIM_W2G_100C_H
#define HG_SIM_W2G_100C_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * f = the dword at +0xf0 of the object that the table at 0x109e8b40 holds for
 * the index [0x1017b8c8]. Returns f & 0xff when (f & 0xf00) == 0x200, else -1.
 */
int __cdecl Sim_100cb9a4(void);

#endif
