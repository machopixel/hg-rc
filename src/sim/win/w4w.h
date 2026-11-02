/*
 * Round 4, letter w: small functions of HGSIM.DLL that call Windows through
 * the DLL's import slots: joystick count, the aux (CD audio) volume, the
 * millisecond timer, the mouse and cursor, the message pump and an error box.
 */
#ifndef HG_SIM_WIN_W4W_H
#define HG_SIM_WIN_W4W_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* The joystick driver's count (table 0x101426f0 +0): 0, joystick support removed in round 27 (the original returns
 * joyGetNumDevs()). */
uint32_t __cdecl Win_JoyGetNumDevs(void);


/*
 * Sets the volume of the aux device 0x1014c9b4 to min(volume, 0xffff) & 0xffff in both channels.
 * Returns 1 when auxSetVolume succeeded, 0 when it failed or there is no device (-1).
 * No CD (the music folder, DECISIONS.md "dropped features"): 1.2's WAV music volume (1.2 0x1001603f), Music_SetVolume(volume) and 1.
 */
int __cdecl Win_AuxSetVolume(int volume);

/* Win_AuxSetVolume(dword 0x101321bc). */
int __cdecl Sim_1009ebeb(void);

/* timeGetTime(), as the 64-bit value with a zero top half that the original returns in edx:eax. */
uint64_t __cdecl Win_TimeGetTime(void);

/*
 * Gets the client rectangle in screen coordinates: rect[0..1] = (0,0) and rect[2..3] = (width-1, height-1)
 * converted with ClientToScreen for the window 0x1017b8f0.
 */
void __cdecl Win_ClientRectToScreen(int32_t *rect, int width, int height);




/* Beeps, formats like wsprintf and shows the text in a message box (icon and flags 0x12030). Returns the MessageBox result. */

#endif
