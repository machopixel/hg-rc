/*
 * The game window's screen services of the platform layer (plat.h) on SDL3, the same on every system: the screen
 * size, placing the window (bordered or full screen), showing it, the mouse pointer, message boxes and the fine
 * clock. SDL3 wraps the game's own window once (PlatSdl_Window); input_sdl3.c uses that same wrap.
 */
#include <SDL3/SDL.h>

#include "plat.h"
#include "plat_sdl3.h"

static SDL_Window *g_wrap;
static void *g_wrapNative;
static int g_video;

int PlatSdl_Video(void)
{
    if (!g_video || !SDL_WasInit(SDL_INIT_VIDEO)) {
        /* the game ends the program through its own window procedure */
        SDL_SetHint(SDL_HINT_QUIT_ON_LAST_WINDOW_CLOSE, "0");
        g_video = SDL_InitSubSystem(SDL_INIT_VIDEO | SDL_INIT_EVENTS) ? 1 : 0;
        g_wrap = NULL;          /* SDL was ended (SDL_Quit) since: any wrap went with it */
        g_wrapNative = NULL;
    }
    return g_video;
}

SDL_Window *PlatSdl_Window(void *native)
{
    SDL_PropertiesID props;

    if (native == NULL || !PlatSdl_Video())
        return NULL;
    if (g_wrap != NULL && g_wrapNative == native)
        return g_wrap;
    PlatSdl_Release();
    props = SDL_CreateProperties();
#ifdef _WIN32
    SDL_SetPointerProperty(props, SDL_PROP_WINDOW_CREATE_WIN32_HWND_POINTER, native);
    g_wrap = SDL_CreateWindowWithProperties(props);
#else
    g_wrap = (SDL_Window *)native;     /* other systems: the game's window is an SDL3 window already */
#endif
    SDL_DestroyProperties(props);
    g_wrapNative = g_wrap != NULL ? native : NULL;
    return g_wrap;
}

void PlatSdl_Release(void)
{
#ifdef _WIN32
    if (g_wrap != NULL && SDL_WasInit(SDL_INIT_VIDEO))
        SDL_DestroyWindow(g_wrap);      /* a wrapped window: its window procedure goes back to the game */
#endif
    g_wrap = NULL;
    g_wrapNative = NULL;
}

void Plat_ScreenSize(int *width, int *height)
{
    SDL_Rect r;

    *width = 640;
    *height = 480;
    if (PlatSdl_Video() && SDL_GetDisplayBounds(SDL_GetPrimaryDisplay(), &r)) {
        *width = r.w;
        *height = r.h;
    }
}

void Plat_WindowPlace(void *window, int fullscreen, int x, int y, int width, int height)
{
    SDL_Window *w = PlatSdl_Window(window);

    if (w == NULL)
        return;
    if (fullscreen) {
        SDL_SetWindowFullscreenMode(w, NULL);  /* borderless over the desktop, no display mode change */
        SDL_SetWindowFullscreen(w, true);
    } else {
        SDL_SetWindowFullscreen(w, false);
        SDL_SetWindowBordered(w, true);
        SDL_SetWindowSize(w, width, height);
        SDL_SetWindowPosition(w, x, y);
    }
    SDL_SyncWindow(w);
}

int Plat_WindowIsFullscreen(void *window)
{
    SDL_Window *w = PlatSdl_Window(window);

    return w != NULL && (SDL_GetWindowFlags(w) & SDL_WINDOW_FULLSCREEN) != 0;
}

void Plat_WindowedSize(int *width, int *height)
{
    SDL_Rect r;
    int k = 1;

    if (PlatSdl_Video() && SDL_GetDisplayUsableBounds(SDL_GetPrimaryDisplay(), &r)) {
        /* the largest whole multiple of 640 x 480 that fits, with room for the title bar */
        k = r.w / 640;
        if ((r.h - 64) / 480 < k)
            k = (r.h - 64) / 480;
        if (k < 1)
            k = 1;
    }
    *width = 640 * k;
    *height = 480 * k;
}

void Plat_WindowRect(void *window, int32_t *rect)
{
    SDL_Window *w = PlatSdl_Window(window);
    int x = 0, y = 0, cw = 0, ch = 0;

    if (w != NULL) {
        SDL_GetWindowPosition(w, &x, &y);
        SDL_GetWindowSize(w, &cw, &ch);
    }
    rect[0] = x;
    rect[1] = y;
    rect[2] = x + cw;
    rect[3] = y + ch;
}

void Plat_WindowShow(void *window, int how)
{
    SDL_Window *w = PlatSdl_Window(window);

    if (w == NULL)
        return;
    switch (how) {
    case PLAT_SHOW_HIDE:
        SDL_HideWindow(w);
        break;
    case PLAT_SHOW_NOACTIVATE:
        SDL_SetHint(SDL_HINT_WINDOW_ACTIVATE_WHEN_SHOWN, "0");
        SDL_ShowWindow(w);
        SDL_SetHint(SDL_HINT_WINDOW_ACTIVATE_WHEN_SHOWN, "1");
        break;
    case PLAT_SHOW_MINIMIZE:
        SDL_MinimizeWindow(w);
        break;
    case PLAT_SHOW_RESTORE:
        SDL_ShowWindow(w);
        SDL_RestoreWindow(w);
        SDL_RaiseWindow(w);
        break;
    default:            /* PLAT_SHOW_DEFAULT */
        SDL_ShowWindow(w);
        break;
    }
}

/* Plat_CursorVisible / Plat_CursorShown: plat_win32.c while the launcher's Windows window is the game window (the
   pointer belongs to that window's messages; DECISIONS.md, "Windows calls: the screen"). */

int Plat_MessageBox(void *window, const char *text, const char *caption, uint32_t type)
{
    /* Windows' button sets (type & 0xf) and the answers they give (IDOK 1, IDCANCEL 2, IDABORT 3, IDRETRY 4,
       IDIGNORE 5, IDYES 6, IDNO 7) */
    static const struct {
        int count;
        int ids[3];
        const char *names[3];
    } kSets[6] = {
        {1, {1}, {"OK"}},
        {2, {1, 2}, {"OK", "Cancel"}},
        {3, {3, 4, 5}, {"Abort", "Retry", "Ignore"}},
        {3, {6, 7, 2}, {"Yes", "No", "Cancel"}},
        {2, {6, 7}, {"Yes", "No"}},
        {2, {4, 2}, {"Retry", "Cancel"}},
    };
    SDL_MessageBoxButtonData buttons[3];
    SDL_MessageBoxData box;
    unsigned set = type & 0xf, icon = type & 0xf0;
    int pressed, i, n;

    if (set > 5)
        set = 0;
    n = kSets[set].count;
    for (i = 0; i < n; i++) {
        buttons[i].flags = 0;
        buttons[i].buttonID = kSets[set].ids[i];
        buttons[i].text = kSets[set].names[i];
    }
    buttons[0].flags |= SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT;
    buttons[n - 1].flags |= SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT;
    SDL_zero(box);
    box.flags = (icon == 0x10 ? SDL_MESSAGEBOX_ERROR : icon == 0x30 ? SDL_MESSAGEBOX_WARNING
                                                                    : SDL_MESSAGEBOX_INFORMATION) |
                SDL_MESSAGEBOX_BUTTONS_LEFT_TO_RIGHT;
    box.window = PlatSdl_Window(window);
    box.title = caption != NULL ? caption : "";
    box.message = text != NULL ? text : "";
    box.numbuttons = n;
    box.buttons = buttons;
    pressed = buttons[n - 1].buttonID;
    if (!SDL_ShowMessageBox(&box, &pressed))
        return 0;       /* as MessageBoxA when no box can be shown */
    return pressed;
}

uint64_t Plat_CounterNow(void)
{
    return SDL_GetPerformanceCounter();
}

uint64_t Plat_CounterRate(void)
{
    return SDL_GetPerformanceFrequency();
}
