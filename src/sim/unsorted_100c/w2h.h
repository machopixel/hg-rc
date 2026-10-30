/*
 * Wave 2 functions of HGSIM.DLL around 0x100c6000..0x100cd000 (letter h).
 * Names carry the original address because the code alone does not say what
 * the fields mean.
 */
#ifndef HG_SIM_UNSORTED_100c_W2H_H
#define HG_SIM_UNSORTED_100c_W2H_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Does nothing. */
void __cdecl Sim_100c6fb8(void);
/* Sets bit 12 of the dword +0xf0 of the object 0x109e8b40[[0x1017b8c8]]. */
void __cdecl Sim_100ca96a(void);
/* Returns 0x18. */
int __cdecl Sim_100cc96c(void);

#endif
