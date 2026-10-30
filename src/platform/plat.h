/*
 * The platform layer (DECISIONS.md, round 28): every OS service the converted game code needs, besides input
 * (input.h), sound (audio.h) and the menus' screen (display.h), behind plain C types. No Windows (or other OS)
 * type appears here; a window is the opaque pointer the OS gave the game.
 *
 * One backend today, for Windows (plat_win32.c). The architecture must also take a Linux backend later: the
 * functions marked "Linux:" say what that backend will have to do; on Windows they are filled in only as far as
 * Windows needs.
 */
#ifndef HG_PLATFORM_PLAT_H
#define HG_PLATFORM_PLAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---- the game window -------------------------------------------------------------------------------------- */

/* How Plat_WindowShow shows the window (the game's four uses of ShowWindow). */
enum {
    PLAT_SHOW_HIDE = 0,       /* hidden */
    PLAT_SHOW_NOACTIVATE = 1, /* shown as it was, not made the active window */
    PLAT_SHOW_MINIMIZE = 2,   /* minimised */
    PLAT_SHOW_RESTORE = 3     /* shown, active and restored */
};
void Plat_WindowShow(void *window, int how);
/* The window's picture has been redrawn by the game (nothing more to paint for now). */
void Plat_WindowPainted(void *window);
/* What the OS does with a window event the game leaves alone. Windows: DefWindowProc. Linux: nothing, 0. */
int32_t Plat_WindowDefault(void *window, uint32_t message, uint32_t wParam, uint32_t lParam);
/* The OS time (milliseconds) of the window event being handled. */
uint32_t Plat_EventTime(void);
/* Asks the program's event loop to end (the window has been destroyed). */
void Plat_RequestQuit(void);
/* Shows or hides the mouse pointer over the window. */
void Plat_CursorVisible(int visible);

/* ---- time and folders ------------------------------------------------------------------------------------- */

/* Milliseconds since some fixed moment (wraps after 49 days). */
uint32_t Plat_TicksMs(void);
/*
 * The program's current folder into out (size bytes, with the end zero); returns its length, 0 on failure.
 * Windows: "C:\...". Linux: the same call, but the game's paths will need a mapping layer (separators, letter
 * case) that this backend does not have yet.
 */
uint32_t Plat_CurrentDir(char *out, uint32_t size);

/* ---- libraries (the renderer plugins) ---------------------------------------------------------------------- */

/* Loads a library by its file name; 0 when it does not load. Windows: LoadLibraryA. Linux: the plugin's shared
 * object (name mapping still to do). */
void *Plat_LoadLibrary(const char *name);
/* A function of a loaded library by name, or 0. */
void *Plat_LibraryFunction(void *library, const char *name);
void Plat_FreeLibrary(void *library);

#ifdef __cplusplus
}
#endif

#endif
