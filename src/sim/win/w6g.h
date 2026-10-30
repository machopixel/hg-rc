/*
 * Round 6, letter g: two more CD audio functions (see w4u.h for the data of the CD module and the MCI
 * message numbers).
 */
#ifndef HG_SIM_WIN_W6G_H
#define HG_SIM_WIN_W6G_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#define __stdcall
#endif

/* Patched by src/hooks/hooks_sim.c (HookedCdAudioInit): always called through its original address. */
#define W6G_CD_INIT SIM_FN(int(__cdecl *)(void), 0x1009dfa0)

/* mciSendCommandA (import slot 0x10ac5850): device id, message, flags, parameter block. */
typedef uint32_t(__stdcall *W6gMciSendCommandFn)(uint32_t, uint32_t, uint32_t, void *);

/*
 * Clears the dwords 0x1017d060 and 0x1017d064; returns 0 when 0x1009dfa0() answers non-zero. Otherwise sets the open
 * flag (0x1017d08c), stores Cd_GetStatus() in 0x1017d078 and, by that status: 1 (not ready) clears the open flag and
 * returns 0; 2 and 3 load the track table (Cd_LoadTracks on 0x1017d080); 4 also reads the position into 0x1017d068.
 * Then sets the volume from dword 0x101321bc (Win_AuxSetVolume) and returns 1.
 */
int __cdecl Cd_Open(void);

/* MCI_SET with MCI_SET_DOOR_OPEN (0x100) on the device 0x1014c9b0; when that fails (non-zero) Cd_Shutdown. */
void __cdecl Cd_OpenDoor(void);

#endif
