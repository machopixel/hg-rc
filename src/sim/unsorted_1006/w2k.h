/* Wave 2 (letter k): one initialiser of HGSIM.DLL at 0x1006271a. */
#ifndef HG_SIM_UNSORTED_1006_W2K_H
#define HG_SIM_UNSORTED_1006_W2K_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Clears the dwords 0x10165be0 and 0x10165be4 and, in each of the ten 0x18-byte records from 0x10165be8, the dwords at +0, +4 and +0x10. */
void __cdecl Sim_1006271a(void);

#endif
