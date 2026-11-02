/* Round 4 (letter u), original 0x1001f335..0x1001f4d3: switches the CD music on and off (see win/w4u.h). */
#ifndef HG_SIM_UNSORTED_1001_W4U_H
#define HG_SIM_UNSORTED_1001_W4U_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Sim_1001f335 / Sim_1001f367 flip the "paused" flag dword 0x1015d1bc to 1 / to 0, but only when it is
 * the other value. In both cases, when dword 0x101321a8 is not -1 and bit 8 of dword 0x101321c0 is set,
 * they call Cd_TogglePause first.
 * No CD (the music folder, DECISIONS.md "dropped features"): 1.2's WAV music versions, Music_Pause(1) / Music_Pause(0) in place of
 * Cd_TogglePause (platform/music.h; the music folder, no CD).
 */
void __cdecl Sim_1001f335(void);
void __cdecl Sim_1001f367(void);
/* Cd_StopWait when dword 0x101321a8 is not -1. No CD: Music_Stop then, and the pause flag
 * 0x1015d1bc cleared with it (1.2 0x10015f97). */
void __cdecl Sim_1001f399(void);
/* Calls Sim_1001f367. */
void __cdecl Sim_1001f4d3(void);

#endif
