/*
 * Round 28: the sim's platform functions on the platform layer (DECISIONS.md, "Round 28"). The game logic of each
 * is the original's; every OS service goes through src/platform (input.h, plat.h), never a Windows call. Installed
 * with the SDL3 platform (Hooks_SdlPlatform); without it the original Windows code runs.
 */
#ifndef HG_SIM_PLATFORM_W28_H
#define HG_SIM_PLATFORM_W28_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#define __stdcall
#endif

/* A new sim load: the input state starts again (the DLL is loaded afresh for every mission). */
void W28_InputReset(void);
/* The sim has returned: the mouse is let go and the input backend closed. */
void W28_InputExit(void);

/*
 * Hands the key events the input backend has collected to the sim's key handling, as the sim's Windows key
 * message handler 0x10057b4e did: a press sets the key state (Key_SetState(vk, lParam, 1)), a release queues the
 * typed key (Input_QueueKey) and clears the state; lParam carries the PC scancode in bits 16..23 and the extended
 * flag in bit 24. Pumps the backend first. Returns the number of key events handed on.
 */
int __cdecl W28_PumpKeys(void);

/* Keyboard device slot 5 (table 0x10143d70): pumps the keys, then empties the typed-key queue (write index
 * 0x1016686c, read index 0x10166868). Returns 0. The original peeks one key message and runs 0x10057b4e on it. */
int __cdecl Sim_10057afc(void);
/* The next typed key (Sim_10057aa9), or 0; when there was none, the keys are pumped and, if any came, read
 * again. The original peeks one key message for that. */
int16_t __cdecl Sim_10057f83(void);
/* The sim's Windows key message handler: with the platform layer the keys come from W28_PumpKeys, so a key
 * message that still reaches it (the window procedure) is left alone. */
void __cdecl Sim_10057b4e(uint32_t message, uint32_t wParam, uint32_t lParam);

/* Mouse device slot 3 (table 0x10149cc0): the mouse is let go (the original: ClipCursor(0)), clip flag 0x10179d64
 * cleared. Returns 0. */
int __cdecl Sim_1007d948(void);
/* Mouse device slot 4: with the game window active ([0x1017b920]) one axis of the mouse position goes back to the
 * middle of the screen-mode box ([0x1017b918] x [0x1017b91c]): axis 0 the vertical, 1 the horizontal. Returns 0. */
int __cdecl Sim_1007d961(int unused, int axis);
/*
 * Mouse device slot 5: the buttons (bit 0 left, 1 middle, 2 right) and the position, 16.16 fixed point from -1 to
 * +1 around the middle of the box (pos[0] vertical, pos[1] horizontal). With the window inactive the mouse is let
 * go, the clip flag cleared and the buttons 0. When the box is not set up ([0x10179d64] 0) or must be again
 * ([0x10179d68]) the mouse is captured and centred. The original kept the Windows cursor in the box and read it;
 * here the backend's movement is added to a position kept inside the box. Returns 0.
 */
int __cdecl Sim_1007d9d8(int unused, int32_t *pos, uint32_t *buttons);

/*
 * The sim's message loop step: waits for an OS event while the window is inactive, nothing more after the quit
 * flag [0x1017b8b8] is set; on the first call with a game window ([0x1017b8f0]) the input backend is opened around
 * it with the game's message rules (quit flag, cursor shown [0x1017b93c]); then pumps the keys.
 */
void __cdecl Sim_10086b99(void);

/*
 * The game window's event handler (the window procedure, stdcall). The OS events the game handles, by their
 * Windows numbers (the Windows backend delivers them; another backend calls the same handling for its events):
 *   0x100..0x108 keys: left alone, the keys come from W28_PumpKeys (the original: 0x10057b4e); 0.
 *   0x0f paint: with the window shown ([0x1017b924]) in renderer mode ([0x1015e9bc] 2) the display driver's
 *        refresh (driver [0x1015e68c] +0x18) and Plat_WindowPainted; 0. Otherwise the OS default.
 *   0x02 destroyed: Plat_RequestQuit; 0.
 *   0x1c activated / deactivated (wParam 1 / 0), unless the quit flag [0x1017b8b8] is set (then the OS default):
 *        [0x1017b928] = [0x1017b920] = wParam. Activated: the key state is cleared (Sim_10057f53); for a mode
 *        that fits 640x480 ([0x1015e9c4] <= 640, [0x1015e9c8] <= 480) without [0x1017b940]: after a minimise by
 *        the game ([0x1015e9c0] 1) the screen mode is set again (0x10037f71, flag cleared) and the window
 *        restored, else restored, or shown without activating for an exclusive display driver ([[0x1015e688]] 1);
 *        bigger modes restored. Then the sound resumes ([0x109e3d5c], with [0x1017b934]). Deactivated: the sound
 *        pauses ([0x109e3d60]); for a fitting mode the window is minimised (hidden for an exclusive driver),
 *        except for a windowed driver ([[0x1015e688]] 0) not minimised before: minimised, flag set, mode set
 *        (0x10037f71); bigger modes minimised. Both: the palette is set again (driver [0x1015e688] +0x14 with
 *        (0, 0x100, 0x1015e6a0, 1)). 0.
 *   0x10 close: quit flag 1, [0x1017b8bc] += 2; 0.
 *   0xa0 mouse over the frame: the pointer is shown (flag [0x1017b93c]) in renderer mode; then the OS default.
 *   0x46 position changing: while the position lock [0x1017b938] holds, the new position (lParam +8..+0x14) is
 *        set to the kept one (0x109e3780..); the lock ends once the event time is past [0x1015dbc8] (with the
 *        wrap of [0x1015da2c]); 0. Without the lock the OS default.
 *   0x200 mouse move: the pointer is hidden when shown (unless [0x1017b92c] and Sim_100766ca(5) is not 1); 0.
 *   0x30f palette wanted: the palette is set again as above, 1; 0 without a driver.
 * Anything else: the OS default.
 */
/*
 * StartSupanim, the loading animation (flag: the long wait, 0x3c, else 2): the animation files are found from the
 * current folder's drive letter ("%c:\launch\supanm6.shp", or netmech6 in a network game; [0x1017d5dc] and
 * [0x1017d5e0] are cleared first, so the other names are never used) but then named "LAUNCH_H" and "TERR0001"
 * (Str_Copy over the built paths, as the original). LAUNCH_H is loaded (Sim_100ba7d5) into [0x1017d5d0], its
 * palette set through the display driver, the 640x480 picture box centred on the screen ([0x10199440],
 * [0x10199444]), and with the display buffer ready (driver +0x20 answering 0) the first frame drawn; TERR0001, when
 * it loads, becomes the second layer ([0x1017d5d4], [0x1017d5d8] frames). An exclusive display driver
 * ([[0x1015e688]] 1) shows it once. Otherwise the original started a Windows timer (250 ms, a second thread) that
 * ran the tick 0x100b8834; here no timer: W28_AnimPoll runs the tick on the game's own thread (deliberate change,
 * DECISIONS.md round 28). [0x10199448] = 10 (the timer period the original picks on a PC), [0x1017d5cc] (the timer)
 * stays 0.
 */
void __cdecl Sim_100b82a0(int longWait);
/* The loading animation's tick (the original's timer callback, stdcall with 5 unused arguments): with the
 * animation loaded ([0x1015e538], [0x1017d5d0]) and the display buffer ready, the next frame is drawn and shown. */
void __stdcall Sim_100b8834(uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t e);
/* Runs the loading animation's tick when 250 ms have passed since the last one; called by the file loader
 * (Sim_100ba7d5) between files. Nothing unless Sim_100b82a0 started the animation. */
void __cdecl W28_AnimPoll(void);

/*
 * SystemError, the game's error report (code, then an optional printf format and its arguments): the text
 * "#%02X: " + the code's line in the SystemError section of the text file (Ini_GetValue), then ": " and the
 * formatted details, goes to 0x1017d618; [0x1017de18] = code. By the code (the original's table): a warning
 * (Sim_100bc535), the game shut down (Sim_100bc451) and the fatal box (Sim_100bc47e), the fatal box alone, or
 * nothing more. With g_w28ErrorTrace set, it is told the code and format first (the hook layer's trace.log line).
 */
void __cdecl Sim_100bc280(uint32_t code, const char *format, ...);
extern void(__cdecl *g_w28ErrorTrace)(uint32_t code, const char *format);

/*
 * The file cache's clock choice (deliberate change, round 28): the original's CPU check stores the processor's
 * cpuid vendor words (0x10199194, 0x1019917c, 0x10199190); the cache start takes the cycle counter for
 * "GenuineIntel", else the millisecond clock, only to stamp cache entries (+0x10/+0x14, never read back). Here the
 * vendor words are always those of an AMD processor (Diego's PC) and the result 1; the cycle counter returns
 * Plat_TicksMs (no rdtsc).
 */
int __cdecl Sim_100b8f30(void);
uint64_t __cdecl Sim_100b8fb1(void);

/*
 * The CD drive search (deliberate change, round 28): the original returns the cached letter [0x1017cc38] when set;
 * otherwise it lists the drives (GetLogicalDriveStringsA), but its CD-ROM test and its search for the movie file
 * do nothing (wrong import slots: FindClose on a path), so with any drive at all it sets the drive count
 * [0x1017cc3c] to 1 and returns 1 (its 0x69-byte list is never freed). Windows always has a drive, so that is its
 * answer; here the same answer without asking the OS: [0x1017cc3c] = 1, result 1.
 */
uint32_t __cdecl Sim_10099510(void);

/*
 * LoadPlugin(path), the renderer plugin's load: the library (Plat_LoadLibrary) must have GetFuncDesc saying
 * "RASTER"; when a plugin was asked for ([0x1015e578]) it must be that one (the type [0x1015e5bc], or the file name
 * [0x1015e57c]) or it is freed; CheckFunc (with [0x1015e59c] or 0) and Sim_10036820 must accept it. Then its type
 * goes to [0x109e3d20] and [0x1015e5bc], its 16 entry points to 0x109e3d24.., the name to [0x1015e57c], and it
 * replaces the plugin loaded before ([0x1015e568], freed). The original with LoadLibraryA / GetProcAddress /
 * FreeLibrary, here through plat.h. With g_w28Trace set, the checks are reported (trace.log).
 */
void __cdecl Sim_1003650d(const char *path);
/* PluginChoice(kind, name, index): "RASTER" asks for a renderer by name ([0x1015e57c]) or type ([0x1015e5bc]),
 * "SOUND" a sound plugin ([0x1015e5c4] / [0x1015e604]), "DRIVER" sets the CheckFunc argument [0x1015e59c]. */
void __cdecl Sim_1003695b(const char *kind, const char *name, uint32_t index);
extern void(__cdecl *g_w28Trace)(const char *format, ...);

int32_t __stdcall Sim_100867a2(uint32_t window, uint32_t message, uint32_t wParam, uint32_t lParam);

#endif
