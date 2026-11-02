/*
 * The Windows backend of the platform layer (plat.h). The game window is the sim's own Windows window, so the
 * window calls are the Windows ones the sim made; SDL3 (input_sdl3.c, display_sdl3.c) wraps that same window.
 */
#include <windows.h>

#include "plat.h"

void Plat_WindowShow(void *window, int how)
{
    static const int kShow[] = {SW_HIDE, SW_SHOWNOACTIVATE, SW_MINIMIZE, SW_RESTORE};

    if (how >= 0 && how < (int)(sizeof(kShow) / sizeof(kShow[0])))
        ShowWindow((HWND)window, kShow[how]);
}

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

void Plat_CursorVisible(int visible)
{
    /* the Windows display count: up to 0 to show, below 0 to hide */
    if (visible) {
        while (ShowCursor(TRUE) < 0) {
        }
    } else {
        while (ShowCursor(FALSE) >= 0) {
        }
    }
}

uint32_t Plat_TicksMs(void)
{
    return (uint32_t)GetTickCount();
}

uint32_t Plat_CurrentDir(char *out, uint32_t size)
{
    return (uint32_t)GetCurrentDirectoryA(size, out);
}

/* Plat_IniInt / Plat_IniString: plat_ini.c (our own reader, every system). Files: plat_file.c. */

uint32_t Plat_ProgramPath(char *out, uint32_t size)
{
    DWORD n = GetModuleFileNameA(NULL, out, size);

    return n < size ? (uint32_t)n : 0;
}

void *Plat_LoadLibrary(const char *name)
{
    return (void *)LoadLibraryA(name);
}

void *Plat_LibraryFunction(void *library, const char *name)
{
    return (void *)GetProcAddress((HMODULE)library, name);
}

void Plat_FreeLibrary(void *library)
{
    FreeLibrary((HMODULE)library);
}
