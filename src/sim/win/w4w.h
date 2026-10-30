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

/* Volume of the aux device 0x1014c9b4 (auxGetVolume): the low word, or -1 when there is no device (-1) or the call fails. */
int __cdecl Win_AuxGetVolume(void);

/*
 * Sets the volume of the aux device 0x1014c9b4 to min(volume, 0xffff) & 0xffff in both channels.
 * Returns 1 when auxSetVolume succeeded, 0 when it failed or there is no device (-1).
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

/*
 * When the mouse capture flag 0x1017b920 is set: moves the cursor to the middle of the window on one axis
 * (axis 0 = y, axis 1 = x), keeping the other coordinate. The first argument is not used. Returns 0.
 */
int __cdecl Win_CenterCursorAxis(int unused, int axis);

/*
 * Reads the mouse. With the capture flag 0x1017b920 set: confines the cursor to the client rectangle (once, tracked in
 * 0x10179d64/0x10179d68); when `buttons` is not NULL stores bit 0 = VK 1, bit 1 = VK 4, bit 2 = VK 2 pressed; when
 * `pos` is not NULL and the cursor is inside the client area stores pos[0] = y and pos[1] = x scaled to +-0x10000.
 * With the flag clear: releases the clip and stores 0 in *buttons. Returns 0.
 */
int __cdecl Win_PollMouse(int unused, int32_t *pos, uint32_t *buttons);

/*
 * Takes one message from the queue and dispatches it (WaitMessage first when 0x1017b920 is clear). Mouse messages
 * 0x200..0x209 are skipped while 0x1017b93c is 0. A WM_QUIT (0x12) for a window sets 0x1017b8b8 = 1 instead.
 */
void __cdecl Win_PumpMessage(void);

/* Beeps, formats like wsprintf and shows the text in a message box (icon and flags 0x12030). Returns the MessageBox result. */
int __cdecl Win_ErrorBox(const char *format, ...);

#endif
