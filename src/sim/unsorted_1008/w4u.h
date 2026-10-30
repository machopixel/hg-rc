/* Round 4 (letter u), original 0x1008b900. */
#ifndef HG_SIM_UNSORTED_1008_W4U_H
#define HG_SIM_UNSORTED_1008_W4U_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* When dword 0x1015ecd0 is 0: Timer_Pause(0x80, 0) and Sim_1001f4d3 (resumes the CD music). */
void __cdecl Sim_1008b900(void);

#endif
