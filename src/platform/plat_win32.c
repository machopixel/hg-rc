/*
 * The Windows backend of the platform layer (plat.h): what is left of the window procedure's plumbing (the launcher
 * makes the game window the Windows way), the pointer and the clock the window messages are compared with. The
 * screen services are SDL3's (plat_window.c), which wraps that same window.
 */
#include <windows.h>
#include <mmsystem.h>

#include "plat.h"

void Plat_WindowPainted(void *window)
{
    ValidateRect((HWND)window, NULL);
}

int32_t Plat_WindowDefault(void *window, uint32_t message, uint32_t wParam, uint32_t lParam)
{
    return (int32_t)DefWindowProcA((HWND)window, message, (WPARAM)wParam, (LPARAM)(int32_t)lParam);
}

uint32_t Plat_EventTime(void)
{
    return (uint32_t)GetMessageTime();
}

void Plat_RequestQuit(void)
{
    PostQuitMessage(0);
}

/* Plat_WindowShow and the other screen services: plat_window.c (SDL3). */

/*
 * The pointer as the game had it: Windows' display count (ShowCursor), which hides it over the window whatever has
 * the focus. SDL3's own hiding (SDL_HideCursor) holds only while SDL sees the mouse in the window, and the game drops
 * the mouse messages while the pointer is hidden, so after Alt+Tab Windows showed the window class's arrow again
 * (Diego's play test, 2026-11-02). Back to SDL3 when SDL makes the window (AFTER_DLL.md).
 */
void Plat_CursorVisible(int visible)
{
    /* up to 0 to show, below 0 to hide */
    if (visible) {
        while (ShowCursor(TRUE) < 0) {
        }
    } else {
        while (ShowCursor(FALSE) >= 0) {
        }
    }
}

int Plat_CursorShown(void)
{
    int count = ShowCursor(TRUE);

    ShowCursor(FALSE);
    return count > 0;       /* the count was 0 or more before */
}

int32_t Plat_WindowSend(void *window, uint32_t message, uint32_t wParam, uint32_t lParam)
{
    return (int32_t)SendMessageA((HWND)window, message, (WPARAM)wParam, (LPARAM)(int32_t)lParam);
}

int Plat_DropEvent(void)
{
    MSG msg;

    return PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE) != 0;
}

uint32_t Plat_TimeMs(void)
{
    return (uint32_t)timeGetTime();
}

/* Plat_IniInt / Plat_IniString: plat_ini.c (our own reader, every system). Files: plat_file.c. The coarse clock,
   debug text, folders and libraries: plat_system.c (SDL3); the keys: input_sdl3.c (round P1 / P2). */
