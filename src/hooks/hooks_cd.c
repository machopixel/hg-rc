/*
 * The movies without the CD (round 33). 1.2's shell finds its movies by going through the drive letters
 * (GetLogicalDriveStringsA), keeping the CD-ROM drives (GetDriveTypeA 5) and taking the first whose "X:\video\"
 * exists (_access): its start-up check (0x100fc406), its movie action (0x100fc782) and its set-up (0x1012b869).
 * The movie is then opened by DirectShow from "X:\video\NAME.AVI" (turned to UTF-16 with MultiByteToWideChar,
 * 0x10022d53). When no CD drive holds a video folder and the game folder has one (video\, the disc's 19 AVIs),
 * HGSHELL's import table answers with one more drive letter: a free one, reported as a CD-ROM drive, whose
 * "X:\video\..." paths are turned into the game folder's video\ in _access and MultiByteToWideChar. A real disc
 * with its video folder still wins. Only HGSHELL's imports are changed; nothing outside the process sees the letter.
 */
#include <io.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include "../launcher/debug.h"
#include "hook.h"

static char g_letter;                  /* the answered drive letter, 0 when not in use */
static char g_videoDir[MAX_PATH];      /* the game folder's video folder, without the trailing backslash */
static unsigned g_mapped;

/* "X:\video..." with the answered letter -> the game folder's path into out; 0 when not such a path */
static int MapPath(const char *path, char *out, size_t size)
{
    if (g_letter == 0 || path == NULL || (path[0] & ~0x20) != g_letter || path[1] != ':' || path[2] != '\\' ||
        _strnicmp(path + 3, "video", 5) != 0)
        return 0;
    snprintf(out, size, "%s%s", g_videoDir, path + 8);
    if (g_mapped++ < 20)
        DebugPrintf("no-CD movies: %s -> %s\n", path, out);
    return 1;
}

static DWORD WINAPI StandInGetLogicalDriveStringsA(DWORD size, LPSTR buf)
{
    DWORD n = GetLogicalDriveStringsA(size, buf);

    /* the list is "A:\0B:\0...\0": one more entry before the last terminator */
    if (g_letter != 0 && n != 0 && n + 4 + 1 <= size) {
        buf[n] = g_letter;
        buf[n + 1] = ':';
        buf[n + 2] = '\\';
        buf[n + 3] = '\0';
        buf[n + 4] = '\0';
        n += 4;
    }
    return n;
}

static UINT WINAPI StandInGetDriveTypeA(LPCSTR root)
{
    if (g_letter != 0 && root != NULL && (root[0] & ~0x20) == g_letter && root[1] == ':')
        return DRIVE_CDROM;
    return GetDriveTypeA(root);
}

static int __cdecl StandInAccess(const char *path, int mode)
{
    char mapped[MAX_PATH];

    if (MapPath(path, mapped, sizeof(mapped)))
        return _access(mapped, mode);
    return _access(path, mode);
}

static int WINAPI StandInMultiByteToWideChar(UINT cp, DWORD flags, LPCSTR src, int n, LPWSTR dst, int dsize)
{
    char mapped[MAX_PATH];

    if (n == -1 && MapPath(src, mapped, sizeof(mapped)))
        return MultiByteToWideChar(cp, flags, mapped, -1, dst, dsize);
    return MultiByteToWideChar(cp, flags, src, n, dst, dsize);
}

/* 1 when a CD-ROM drive holds X:\video\ (the shell's own test) */
static int DiscHasVideo(void)
{
    char drives[0x69] = {0}, path[16];
    const char *d;

    GetLogicalDriveStringsA(sizeof(drives), drives);
    for (d = drives; *d != '\0'; d += strlen(d) + 1) {
        if (GetDriveTypeA(d) != DRIVE_CDROM)
            continue;
        snprintf(path, sizeof(path), "%c:\\video\\", d[0]);
        if (_access(path, 0) == 0)
            return 1;
    }
    return 0;
}

/*
 * The menu's focus after its start (round 33, Diego's black menu after a mission). 1.2's shell draws only while its
 * focus flag (+0x1f8) is set: by WM_SETFOCUS / WM_KILLFOCUS in ShellWindowProc and by one GetFocus() test when it
 * starts. Its start routine 0x100301d7 sets the mode, routes our window to it and then takes 10 messages off the
 * queue unhandled, so a focus message waiting then is lost. Once that flush has run, the first PeekMessageA of its
 * own loop gets a WM_SETFOCUS posted in front of it (or the window is given the focus, which sends one), as the
 * window's activation did in 1998.
 */
static uintptr_t g_flushLo, g_flushHi;      /* 1.2 HGSHELL 0x100301d7..0x10030260 at its load address */
static int g_flushSeen, g_flushCalls;

#ifdef _MSC_VER
#include <intrin.h>
#define CD_CALLER() ((uintptr_t)_ReturnAddress())
#else
#define CD_CALLER() ((uintptr_t)__builtin_return_address(0))
#endif

/* the game window: the visible top-level window of this thread (GetActiveWindow is NULL when it is not active) */
static BOOL CALLBACK FindVisible(HWND w, LPARAM out)
{
    if (IsWindowVisible(w)) {
        *(HWND *)out = w;
        return FALSE;
    }
    return TRUE;
}

static HWND GameWindow(void)
{
    HWND w = GetActiveWindow();

    if (w == NULL)
        EnumThreadWindows(GetCurrentThreadId(), FindVisible, (LPARAM)&w);
    return w;
}

static BOOL WINAPI StandInPeekMessageA(LPMSG msg, HWND window, UINT lo, UINT hi, UINT remove)
{
    uintptr_t from = CD_CALLER();

    if (from >= g_flushLo && from < g_flushHi) {
        if (g_flushCalls++ == 0)
            DebugPrintf("shell start: its message flush, focus %p, foreground %p\n", (void *)GetFocus(),
                        (void *)GetForegroundWindow());
        g_flushSeen = 1;
    } else if (g_flushSeen) {
        HWND main = GameWindow();

        g_flushSeen = 0;
        g_flushCalls = 0;
        if (main != NULL && GetFocus() == main) {
            PostMessageA(main, WM_SETFOCUS, 0, 0);
            DebugPrintf("shell start: WM_SETFOCUS posted to %p after its flush\n", (void *)main);
        } else {
            HWND w = main;

            if (w != NULL) {
                SetForegroundWindow(w);
                SetFocus(w);
            }
            DebugPrintf("shell start: focus given to %p after its flush (now %p)\n", (void *)w, (void *)GetFocus());
        }
    }
    return PeekMessageA(msg, window, lo, hi, remove);
}

/* the shell's one start-up focus test (its object's constructor 0x1002fb63: +0x1f8 = GetFocus() == its window),
   logged with what it gets */
static uintptr_t g_ctorLo, g_ctorHi;

static HWND WINAPI StandInGetFocus(void)
{
    uintptr_t from = CD_CALLER();
    HWND f = GetFocus();

    if (from >= g_ctorLo && from < g_ctorHi)
        DebugPrintf("shell start: its focus test GetFocus() = %p (active %p, foreground %p)\n", (void *)f,
                    (void *)GetActiveWindow(), (void *)GetForegroundWindow());
    return f;
}

void HooksCd_OnShellLoaded(HMODULE shell)
{
    char dir[MAX_PATH];
    DWORD used = GetLogicalDrives();
    int n = 0, i;

    g_flushLo = (uintptr_t)shell + 0x301d7;
    g_flushHi = (uintptr_t)shell + 0x30260;
    g_flushSeen = 0;
    g_flushCalls = 0;
    g_ctorLo = (uintptr_t)shell + 0x2fb63;
    g_ctorHi = (uintptr_t)shell + 0x2fed2;
    DebugPrintf("shell start: %d import(s) replaced (its focus test, and its focus after the message flush)\n",
                Hook_PatchImport(shell, "USER32.dll", "PeekMessageA", (void *)StandInPeekMessageA) +
                Hook_PatchImport(shell, "USER32.dll", "GetFocus", (void *)StandInGetFocus));
    g_letter = 0;
    if (GetCurrentDirectoryA(sizeof(dir), dir) == 0)
        return;
    snprintf(g_videoDir, sizeof(g_videoDir), "%s\\video", dir);
    if (_access(g_videoDir, 0) != 0) {
        DebugPrintf("no-CD movies: no %s, the shell looks for its disc\n", g_videoDir);
        return;
    }
    if (DiscHasVideo()) {
        DebugPrintf("no-CD movies: a disc with its video folder is in a drive, used as it is\n");
        return;
    }
    for (i = 'Z'; i >= 'D'; i--)
        if (!(used & (1u << (i - 'A')))) {
            g_letter = (char)i;
            break;
        }
    if (g_letter == 0)
        return;
    n += Hook_PatchImport(shell, "KERNEL32.dll", "GetLogicalDriveStringsA", (void *)StandInGetLogicalDriveStringsA);
    n += Hook_PatchImport(shell, "KERNEL32.dll", "GetDriveTypeA", (void *)StandInGetDriveTypeA);
    n += Hook_PatchImport(shell, "KERNEL32.dll", "MultiByteToWideChar", (void *)StandInMultiByteToWideChar);
    n += Hook_PatchImport(shell, "MSVCRT.dll", "_access", (void *)StandInAccess);
    DebugPrintf("no-CD movies: drive %c: answered as the CD with %s (%d import(s) replaced)\n", g_letter,
                g_videoDir, n);
}
