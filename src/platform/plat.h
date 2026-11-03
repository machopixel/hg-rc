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

/* How Plat_WindowShow shows the window (the game's uses of ShowWindow). */
enum {
    PLAT_SHOW_HIDE = 0,       /* hidden */
    PLAT_SHOW_NOACTIVATE = 1, /* shown as it was, not made the active window */
    PLAT_SHOW_MINIMIZE = 2,   /* minimised */
    PLAT_SHOW_RESTORE = 3,    /* shown, active and restored */
    PLAT_SHOW_DEFAULT = 4     /* shown (SW_SHOWDEFAULT) */
};
/*
 * The window's screen services on SDL3, the same on every system (plat_window.c; DECISIONS.md, "Windows calls: the
 * screen"): `window` is the OS window the game was given, which SDL3 wraps once.
 */
void Plat_WindowShow(void *window, int how);
/* The main screen's (desktop's) size, in the units windows are placed in. */
void Plat_ScreenSize(int *width, int *height);
/* fullscreen != 0: borderless over the whole screen, without a display mode change; else a bordered window whose
 * client area is width x height at x, y on the screen. */
void Plat_WindowPlace(void *window, int fullscreen, int x, int y, int width, int height);
int Plat_WindowIsFullscreen(void *window);
/* The client size a window gets when Alt+Enter leaves full screen: the largest whole multiple of 640 x 480 that
 * fits the screen. */
void Plat_WindowedSize(int *width, int *height);
/* The client area on the screen: left, top, right, bottom. */
void Plat_WindowRect(void *window, int32_t *rect);
/* A message box with Windows MessageBoxA's type bits (buttons: type & 0xf; icon 0x10 error, 0x30 warning) and its
 * answers (IDOK 1, IDCANCEL 2, IDABORT 3, IDRETRY 4, IDIGNORE 5, IDYES 6, IDNO 7); 0 when none can be shown. */
int Plat_MessageBox(void *window, const char *text, const char *caption, uint32_t type);
/* A fine clock: its count now and its counts per second. */
uint64_t Plat_CounterNow(void);
uint64_t Plat_CounterRate(void);
/* The window's picture has been redrawn by the game (nothing more to paint for now). */
void Plat_WindowPainted(void *window);
/* What the OS does with a window event the game leaves alone. Windows: DefWindowProc. Linux: nothing, 0. */
int32_t Plat_WindowDefault(void *window, uint32_t message, uint32_t wParam, uint32_t lParam);
/* The OS time (milliseconds) of the window event being handled. */
uint32_t Plat_EventTime(void);
/* Asks the program's event loop to end (the window has been destroyed). */
void Plat_RequestQuit(void);
/* Shows or hides the mouse pointer over the window; whether it is shown (plat_win32.c: Windows' display count, as
 * long as the launcher's Windows window is the game window). */
void Plat_CursorVisible(int visible);
int Plat_CursorShown(void);
/* Windows launcher plumbing (plat_win32.c; goes with the launcher's own window, AFTER_DLL.md): a message to the
 * window procedure (the launcher's routing messages), and discarding one waiting window event (nonzero when there
 * was one: PeekMessageA with PM_REMOVE). */
int32_t Plat_WindowSend(void *window, uint32_t message, uint32_t wParam, uint32_t lParam);
int Plat_DropEvent(void);

/* ---- time and folders ------------------------------------------------------------------------------------- */

/* Milliseconds since some fixed moment (wraps after 49 days): SDL_GetTicks, 1 ms steps (plat_system.c). */
uint32_t Plat_TicksMs(void);
/* Milliseconds since some fixed moment at the multimedia timer's finer step (Windows: timeGetTime, as the sim's own
 * timing read; Linux / Mac: SDL_GetTicks, still to do). */
uint32_t Plat_TimeMs(void);
/*
 * The program's current folder into out (size bytes, with the end zero), no separator at the end; returns its
 * length, the size needed when out is too small, 0 on failure. SDL_GetCurrentDirectory (plat_system.c): Windows
 * "C:\..."; Linux / Mac "C:" in front of the folder ("C:/home/..."), since the game takes the first character as a
 * drive letter; the file layer drops "X:" again.
 */
uint32_t Plat_CurrentDir(char *out, uint32_t size);

/* ---- settings files (.ini: [section] key=value) --------------------------------------------------------- */

/*
 * The whole number of key in section of the settings file path, or def when there is none: a leading decimal
 * number with an optional sign. Our own reader with Windows' GetPrivateProfileIntA rules (plat_ini.c), every system.
 */
uint32_t Plat_IniInt(const char *path, const char *section, const char *key, uint32_t def);
/*
 * The text of key in section of path into out (size bytes, with the end zero), or def when there is none;
 * returns the length copied (GetPrivateProfileStringA's rules, plat_ini.c).
 */
uint32_t Plat_IniString(const char *path, const char *section, const char *key, const char *def, char *out,
                        uint32_t size);

/* ---- files (C runtime replacement, files: DECISIONS.md) ------------------------------------------------------ */

/*
 * The file services the game's C runtime (the converted copies of the original's, src/sim/unsorted_100f and
 * unsorted_1010) used to take from Windows, in the same shape as the Windows calls they replace, so those copies
 * keep every rule of their own (text mode, Ctrl-Z, append, the error numbers). One implementation for every system
 * (plat_file.c): standard C files, always in binary (exact bytes), and SDL3 for listing folders. Names are the
 * game's: "\" or "/", a drive letter ("C:") left out except on Windows, and on other systems a name that does not
 * exist as written is looked up ignoring upper / lower case, folder by folder.
 * A handle is never 0 or PLAT_FILE_INVALID. Errors are Windows error numbers (2 not found, 3 no folder, 5 denied,
 * 80 exists, 131 negative seek), read with Plat_FileLastError.
 */
#define PLAT_FILE_INVALID 0xffffffffu

/* CreateFileA: access 0x80000000 read / 0x40000000 write; disposition 1 create new, 2 create always, 3 open
 * existing, 4 open always, 5 truncate existing. share, security and flags are accepted and not used. */
uint32_t Plat_FileCreate(const char *name, uint32_t access, uint32_t share, void *security, uint32_t disposition,
                         uint32_t flags, uint32_t tmpl);
/* ReadFile / WriteFile: nonzero on success, *done the bytes moved (overlapped must be 0). */
int32_t Plat_FileRead(uint32_t handle, void *data, uint32_t size, uint32_t *done, void *overlapped);
int32_t Plat_FileWrite(uint32_t handle, const void *data, uint32_t size, uint32_t *done, void *overlapped);
/* SetFilePointer: method 0 start, 1 current, 2 end; the new position, or PLAT_FILE_INVALID (high must be 0). */
uint32_t Plat_FileSeek(uint32_t handle, int32_t distance, int32_t *high, uint32_t method);
/* SetEndOfFile at the current position, FlushFileBuffers, CloseHandle: nonzero on success. */
int32_t Plat_FileTruncate(uint32_t handle);
int32_t Plat_FileFlush(uint32_t handle);
int32_t Plat_FileClose(uint32_t handle);
/* GetFileType: 1 a disk file (every handle of ours), 0 unknown. */
uint32_t Plat_FileType(uint32_t handle);
/* DeleteFileA: nonzero on success. */
int32_t Plat_FileDelete(const char *name);
/* GetLastError after one of these. */
uint32_t Plat_FileLastError(void);
/*
 * FindFirstFileA / FindNextFileA / FindClose over a pattern ("folder\*.ext", "?" and "*"): data is filled as
 * Windows' WIN32_FIND_DATAA (attributes +0, creation / access / write times +4 / +0xc / +0x14 as FILETIME, size high
 * / low +0x1c / +0x20, name +0x2c, 260 bytes, short name +0x130 left empty). Letter case of the pattern does not
 * matter. Attributes: 0x10 a folder, 0x20 a file (0x21 when it cannot be written).
 */
uint32_t Plat_FileFindFirst(const char *pattern, void *data);
int32_t Plat_FileFindNext(uint32_t find, void *data);
int32_t Plat_FileFindClose(uint32_t find);

/* ---- memory and time (plat_system.c, standard C and SDL3) -------------------------------------------------- */

/* A memory block that knows its size (GlobalAlloc(GMEM_FIXED) / GlobalSize / GlobalFree): NULL when out of memory. */
void *Plat_BlockAlloc(uint32_t size);
uint32_t Plat_BlockSize(const void *block);
void Plat_BlockFree(void *block);
/* Fills a MEMORYSTATUS (0x20 bytes, its length already set) as GlobalMemoryStatus: the RAM, capped at 4 GB - 1. */
void Plat_MemoryStatus(void *status);
/* Fills a SYSTEMTIME (8 words) with the time in UTC, as GetSystemTime. */
void Plat_SystemTime(void *systemTime);
/* A file time (FILETIME: 100 ns steps since 1601, UTC) as local time in a SYSTEMTIME (8 words), as
 * FileTimeToLocalFileTime followed by FileTimeToSystemTime; 0 when it cannot be converted. */
int Plat_FileTimeToLocal(const uint32_t *fileTime, void *systemTime);

/* ---- program ---------------------------------------------------------------------------------------------- */

/* The program's folder and "HG" into out (size bytes, with the end zero); its length, 0 when unknown
 * (SDL_GetBasePath, plat_system.c; only the assert box shows it). */
uint32_t Plat_ProgramPath(char *out, uint32_t size);
/* The C runtime's assert box (C runtime replacement, the rest): text and caption, Abort / Retry / Ignore (SDL3). */
enum { PLAT_ASSERT_ABORT = 3, PLAT_ASSERT_RETRY = 4, PLAT_ASSERT_IGNORE = 5 };
int Plat_AssertBox(const char *text, const char *caption);
/* Stops in the debugger (the original's DebugBreak after "Retry"); without one the program ends there. */
void Plat_DebugBreak(void);
/* A line of debug text: SDL's log (Windows: also the attached debugger; Linux / Mac: the console). */
void Plat_DebugText(const char *text);

/* ---- keyboard ---------------------------------------------------------------------------------------------- */

/* The character a Windows virtual-key code types without modifiers on the user's keyboard layout, as
 * MapVirtualKeyA type 2 gives it (English letters upper case, Windows-1252), 0 for none: SDL3's key map (input_sdl3.c). */
uint32_t Plat_KeyChar(uint32_t key);
/* Shift / Ctrl / Alt / Caps Lock / Num Lock's state as GetKeyState gives it (held 0xFF80, toggled bit 0), from
 * SDL3's modifier state (input_sdl3.c). */
int16_t Plat_KeyState(int key);

/* ---- libraries (the renderer plugins) ---------------------------------------------------------------------- */

/* Loads a library by its file name; 0 when it does not load. A name whose file part is a built-in library's
 * (Plat_AddBuiltinLibrary: the Vulkan renderer "HGVK.DLL") gives that one, from the program itself. Otherwise
 * SDL_LoadObject (plat_system.c). */
void *Plat_LoadLibrary(const char *name);
/* A library built into the program, under its file name (letter case aside): `function` answers its functions
 * by name (NULL for none). */
void Plat_AddBuiltinLibrary(const char *name, void *(*function)(const char *name));
/* A function of a loaded library by name, or 0. */
void *Plat_LibraryFunction(void *library, const char *name);
void Plat_FreeLibrary(void *library);

#ifdef __cplusplus
}
#endif

#endif
