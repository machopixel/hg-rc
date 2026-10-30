/* Round 3 (letter r): a user of the timer module (core.h). */
#ifndef HG_SIM_CORE_W3R_H
#define HG_SIM_CORE_W3R_H

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Restarts the timer whose number is stored at 0x101354dc, then loops until it has counted
 * 0x5b (91) steps. Only returns because another thread of the game steps the timer.
 */
void __cdecl Sim_1002ab74(void);

#endif
