/*
 * Wave 2g: leaf functions of HGSIM.DLL from 0x100b3547..0x100b65b2.
 */
#ifndef HG_SIM_W2G_100B_H
#define HG_SIM_W2G_100B_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Copies six dwords: 0x10150634, 0x1015064c, 0x10150660, 0x10150674, 0x10150688, 0x1015069c = those at 0x101c47bc, 0x101c45e4, 0x101c4858, 0x101c4f90, 0x101c4758, 0x101c4fd8. */
void __cdecl Sim_100b3547(void);
/*
 * When [0x1017d20c] < [0x1017d210] (signed): stores the pair (a, b) as the
 * entry [0x1017d20c] of the table of 8-byte entries at [0x1017d214], counts
 * [0x1017d20c] up and returns 1; otherwise returns 0.
 */
int __cdecl Sim_100b367e(int a, int b);
/* Returns 1. */
int __cdecl Sim_100b6590(void);
/* Does nothing. */
void __cdecl Sim_100b65b2(void);

#endif
