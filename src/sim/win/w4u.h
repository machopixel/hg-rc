/*
 * Round 4 (letter u), Windows wrappers.
 *
 * CD audio (original 0x1009dec0..0x1009eaa2): the game plays its music from the audio CD through the
 * Windows MCI (mciSendCommandA, import slot 0x10ac5850). The message numbers sent are the MCI_* numbers
 * (0x804 MCI_CLOSE, 0x806 MCI_PLAY, 0x808 MCI_STOP, 0x809 MCI_PAUSE, 0x80d MCI_SET, 0x814 MCI_STATUS,
 * 0x855 MCI_RESUME). Data of the module:
 *   dword 0x1014c9b0  MCI device id (-1 when closed)
 *   dword 0x1017d08c  non-zero while the CD is open
 *   0x1017d080        track range: dword first track, dword last track, dword pointer to a table of the
 *                     position of each track (first..last) and of the end (Cd_LoadTracks)
 *   0x1017d068        position saved when paused: track, minute, second, frame
 *   dwords 0x1017d090, 0x1017d094   from/to positions of the last Cd_PlayTrack
 *   dword 0x1017d078  last status read by Cd_UpdateStatus
 * Status codes of Cd_GetStatus: 1 open/not ready, 2 stopped, 3 playing, 4 paused, 5 unknown or error.
 *
 * Display (original 0x1007eda5..0x1008400d): the game's window and its device-independent bitmap.
 */
#ifndef HG_SIM_WIN_W4U_H
#define HG_SIM_WIN_W4U_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Index of the first CD audio aux device (auxGetDevCaps technology 1), else of the first aux-in device (2), else -1; -1 on any error. */
int __cdecl Aux_FindCdDevice(void);

/* MCI_CLOSE on the device, then the device id becomes -1. Returns 0. */
int __cdecl Cd_Close(void);
/* MCI_PAUSE, MCI_RESUME, MCI_STOP: return the MCI result (0 = success). */
uint32_t __cdecl Cd_Pause(void);
uint32_t __cdecl Cd_Resume(void);
uint32_t __cdecl Cd_Stop(void);
/* MCI_PLAY from position `from`, up to `to` when `to` is not 0. Returns the MCI result. */
uint32_t __cdecl Cd_Play(uint32_t from, uint32_t to);
/* When open: frees the track table of the range at 0x1017d080, closes the device and clears the open flag. */
void __cdecl Cd_Shutdown(void);
/* MCI_STATUS (mode) as a status code (see above). Returns 5 when the CD is not open; when the query fails it calls Cd_Shutdown first and returns 5. */
int __cdecl Cd_GetStatus(void);
/* Stores Cd_GetStatus() in dword 0x1017d078 and returns it. */
int __cdecl Cd_UpdateStatus(void);
/* Stopped: plays the last range (0x1017d090..0x1017d094); playing: saves the position (0x1017d068) and pauses; paused: resumes. */
void __cdecl Cd_TogglePause(void);
/* Stopped: plays from the start of the first table entry; playing: only saves the position (0x1017d068); paused: resumes. */
void __cdecl Sim_1009e441(void);
/* While open: stops, again and again, until the status is no longer 3 (playing). */
void __cdecl Cd_StopWait(void);
/* Skip one track back / forward (Cd_Skip with -1 / +1 on the range at 0x1017d080), only while open. */
void __cdecl Cd_PrevTrack(void);
void __cdecl Cd_NextTrack(void);
/* out[4] = track, minute, second, frame: zeros when not playing or paused, the live position when playing, the saved one (0x1017d068) when paused. */
void __cdecl Cd_GetPosition(uint32_t *out);
/* MCI_STATUS position split into the four bytes of the result; out stays unchanged when the query fails. Only while open. */
void __cdecl Cd_ReadPosition(uint32_t *out);
/* Playing or paused only: jumps to the track `delta` away from the current one when it lies inside range; stopping or pausing as the status was. */
void __cdecl Cd_Skip(const uint32_t *range, int delta);
/* Plays track number `track` (remembers its from/to positions in 0x1017d090/94), when it is inside the range at 0x1017d080. */
void __cdecl Cd_PlayTrack(int track);
/* Plays the position range from..to (only when to > from, unsigned), playing or paused only. */
void __cdecl Cd_PlayRange(uint32_t from, uint32_t to);
/* Reads the number of tracks and the position of each into a new table: range[0] = 1, range[1] = count, range[2] = table. Returns 0, or 1 when the CD is not open or the table cannot be allocated. */
int __cdecl Cd_LoadTracks(uint32_t *range);
/* Frees the track table range[2] (when not NULL) and clears the pointer. */
void __cdecl Cd_FreeTracks(uint32_t *range);

/* The names of the three DisplayDibWindow functions are the ones the DLL prints in their error messages ("DisplayDibWindowEnd err" ...). */
/* Closes the window's bitmap objects, hides it (ShowWindow 0), sends the window message 0x402 and clears the display variables; returns 0. */
int __cdecl DisplayDibWindowEnd(void);
/* Sends the window message 0x4a with a 3-dword block {0x240401, dword 0x109e3ce8 or 0, dword 0x1015e9a0}; returns the result (also in dword 0x10372d8c). */
int __cdecl DisplayDibWindowDraw(void);
/* Copies 256 RGB triples to 0x109b8c00, converts them with Sim_1004b804 into 0x1015e6a0 and the palette block 0x109e38e8 (value << 2), sends window message 0x4a. Returns -1 for a NULL pointer or a failed send, else 0. */
int __cdecl DisplayDibWindowSetFmt(const uint8_t *rgb);
/* Creates the window's DIB section, or sets its colour table when it exists. Returns 1, or 0 when the DIB section cannot be created. */
int __cdecl Sim_1008375a(void);
/* SetDIBitsToDevice of the rectangle (a, b)..(c, d) of the bitmap at dword 0x1015e9a0 (the message "GDI BitBlt Rect err" on failure); returns -1 unless all lines (dword 0x109e3d18) were drawn. */
int __cdecl Gdi_BitBltRect(int a, int b, int c, int d);
/* Copies `count` RGB triples to palette entries start.. (0x1015e6a0), rebuilds the GDI palette from all 256 (value << 2). Returns 1, or 0 for bad arguments. */
int __cdecl Gdi_SetPalette(int start, int count, const uint8_t *rgb);
/* Gdi_SetPalette(a, b, c) when the flag 0x1017b7d8 is set; 0 on success, -1 otherwise. The fourth argument is not used. */
int __cdecl Sim_1008400d(int a, int b, const uint8_t *c, int d);

#endif
