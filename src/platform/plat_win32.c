/*
 * The Windows backend of the platform layer (plat.h): what is left of the window procedure's plumbing (the launcher
 * makes the game window the Windows way), the clocks, libraries and keys. The screen services are SDL3's
 * (plat_window.c), which wraps that same window.
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

uint32_t Plat_TicksMs(void)
{
    return (uint32_t)GetTickCount();
}

uint32_t Plat_TimeMs(void)
{
    return (uint32_t)timeGetTime();
}

void Plat_DebugText(const char *text)
{
    OutputDebugStringA(text);
}

uint32_t Plat_KeyChar(uint32_t key)
{
    return (uint32_t)MapVirtualKeyA(key, 2);
}

int16_t Plat_KeyState(int key)
{
    return (int16_t)GetKeyState(key);
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

/* the libraries built into the program (the Vulkan renderer); a handle of ours is the address of its entry */
typedef struct {
    const char *name;
    void *(*function)(const char *name);
} PlatBuiltin;

static PlatBuiltin g_builtins[4];
static int g_builtinCount;

void Plat_AddBuiltinLibrary(const char *name, void *(*function)(const char *name))
{
    if (g_builtinCount < (int)(sizeof(g_builtins) / sizeof(g_builtins[0]))) {
        g_builtins[g_builtinCount].name = name;
        g_builtins[g_builtinCount].function = function;
        g_builtinCount++;
    }
}

static PlatBuiltin *Builtin(const void *handle)
{
    int i;

    for (i = 0; i < g_builtinCount; i++) {
        if (handle == &g_builtins[i])
            return &g_builtins[i];
    }
    return NULL;
}

void *Plat_LoadLibrary(const char *name)
{
    const char *file = name;
    const char *p;
    int i;

    for (p = name; *p != '\0'; p++) {
        if (*p == '\\' || *p == '/' || *p == ':')
            file = p + 1;
    }
    for (i = 0; i < g_builtinCount; i++) {
        if (_stricmp(file, g_builtins[i].name) == 0)
            return &g_builtins[i];
    }
    return (void *)LoadLibraryA(name);
}

void *Plat_LibraryFunction(void *library, const char *name)
{
    PlatBuiltin *b = Builtin(library);

    if (b != NULL)
        return b->function(name);
    return (void *)GetProcAddress((HMODULE)library, name);
}

void Plat_FreeLibrary(void *library)
{
    if (Builtin(library) == NULL)
        FreeLibrary((HMODULE)library);
}
