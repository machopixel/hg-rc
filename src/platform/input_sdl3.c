/*
 * SDL3 backend of the platform input layer (input.h).
 *
 * SDL3 wraps the game's own window instead of creating one. For such a window
 * SDL hands every message on to the original window procedure after looking
 * at it ("assume it's going to handle messages" in SDL_windowsevents.c), so the
 * game keeps handling its window exactly as before while SDL sees the input.
 */
#include <string.h>
#include <windows.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_system.h>

#include "input.h"
#include "plat_sdl3.h"

static SDL_Window *g_window;
static int g_sdlStarted;
static InputStats g_stats;
static float g_mouseDx, g_mouseDy;
static int g_wantRelative;
static int *g_quitFlag;
static const int *g_cursorShown;

/*
 * SDL scancodes (USB usage numbers) to what the sim wants: the PC scancode, the
 * extended-key flag and the Windows virtual key code, as a Windows keyboard
 * message would carry them.
 */
typedef struct KeyMap {
    int sc, ext, vk;
} KeyMap;

static KeyMap g_keyMap[SDL_SCANCODE_COUNT];

static void MapKey(SDL_Scancode usb, int sc, int ext, int vk)
{
    g_keyMap[usb].sc = sc;
    g_keyMap[usb].ext = ext;
    g_keyMap[usb].vk = vk;
}

static void BuildKeyMap(void)
{
    static const unsigned char letters[26] = {0x1E, 0x30, 0x2E, 0x20, 0x12, 0x21, 0x22, 0x23, 0x17,
                                              0x24, 0x25, 0x26, 0x32, 0x31, 0x18, 0x19, 0x10, 0x13,
                                              0x1F, 0x14, 0x16, 0x2F, 0x11, 0x2D, 0x15, 0x2C};
    int i;

    for (i = 0; i < 26; i++)
        MapKey((SDL_Scancode)(SDL_SCANCODE_A + i), letters[i], 0, 'A' + i);
    for (i = 0; i < 9; i++)
        MapKey((SDL_Scancode)(SDL_SCANCODE_1 + i), 0x02 + i, 0, '1' + i);
    MapKey(SDL_SCANCODE_0, 0x0B, 0, '0');
    MapKey(SDL_SCANCODE_RETURN, 0x1C, 0, VK_RETURN);
    MapKey(SDL_SCANCODE_ESCAPE, 0x01, 0, VK_ESCAPE);
    MapKey(SDL_SCANCODE_BACKSPACE, 0x0E, 0, VK_BACK);
    MapKey(SDL_SCANCODE_TAB, 0x0F, 0, VK_TAB);
    MapKey(SDL_SCANCODE_SPACE, 0x39, 0, VK_SPACE);
    MapKey(SDL_SCANCODE_MINUS, 0x0C, 0, VK_OEM_MINUS);
    MapKey(SDL_SCANCODE_EQUALS, 0x0D, 0, VK_OEM_PLUS);
    MapKey(SDL_SCANCODE_LEFTBRACKET, 0x1A, 0, VK_OEM_4);
    MapKey(SDL_SCANCODE_RIGHTBRACKET, 0x1B, 0, VK_OEM_6);
    MapKey(SDL_SCANCODE_BACKSLASH, 0x2B, 0, VK_OEM_5);
    MapKey(SDL_SCANCODE_NONUSHASH, 0x2B, 0, VK_OEM_5);
    MapKey(SDL_SCANCODE_SEMICOLON, 0x27, 0, VK_OEM_1);
    MapKey(SDL_SCANCODE_APOSTROPHE, 0x28, 0, VK_OEM_7);
    MapKey(SDL_SCANCODE_GRAVE, 0x29, 0, VK_OEM_3);
    MapKey(SDL_SCANCODE_COMMA, 0x33, 0, VK_OEM_COMMA);
    MapKey(SDL_SCANCODE_PERIOD, 0x34, 0, VK_OEM_PERIOD);
    MapKey(SDL_SCANCODE_SLASH, 0x35, 0, VK_OEM_2);
    MapKey(SDL_SCANCODE_NONUSBACKSLASH, 0x56, 0, VK_OEM_102);
    MapKey(SDL_SCANCODE_CAPSLOCK, 0x3A, 0, VK_CAPITAL);
    for (i = 0; i < 10; i++)
        MapKey((SDL_Scancode)(SDL_SCANCODE_F1 + i), 0x3B + i, 0, VK_F1 + i);
    MapKey(SDL_SCANCODE_F11, 0x57, 0, VK_F11);
    MapKey(SDL_SCANCODE_F12, 0x58, 0, VK_F12);
    MapKey(SDL_SCANCODE_PRINTSCREEN, 0x37, 1, VK_SNAPSHOT);
    MapKey(SDL_SCANCODE_SCROLLLOCK, 0x46, 0, VK_SCROLL);
    MapKey(SDL_SCANCODE_PAUSE, 0x45, 0, VK_PAUSE);
    MapKey(SDL_SCANCODE_INSERT, 0x52, 1, VK_INSERT);
    MapKey(SDL_SCANCODE_HOME, 0x47, 1, VK_HOME);
    MapKey(SDL_SCANCODE_PAGEUP, 0x49, 1, VK_PRIOR);
    MapKey(SDL_SCANCODE_DELETE, 0x53, 1, VK_DELETE);
    MapKey(SDL_SCANCODE_END, 0x4F, 1, VK_END);
    MapKey(SDL_SCANCODE_PAGEDOWN, 0x51, 1, VK_NEXT);
    MapKey(SDL_SCANCODE_RIGHT, 0x4D, 1, VK_RIGHT);
    MapKey(SDL_SCANCODE_LEFT, 0x4B, 1, VK_LEFT);
    MapKey(SDL_SCANCODE_DOWN, 0x50, 1, VK_DOWN);
    MapKey(SDL_SCANCODE_UP, 0x48, 1, VK_UP);
    MapKey(SDL_SCANCODE_NUMLOCKCLEAR, 0x45, 0, VK_NUMLOCK);
    MapKey(SDL_SCANCODE_KP_DIVIDE, 0x35, 1, VK_DIVIDE);
    MapKey(SDL_SCANCODE_KP_MULTIPLY, 0x37, 0, VK_MULTIPLY);
    MapKey(SDL_SCANCODE_KP_MINUS, 0x4A, 0, VK_SUBTRACT);
    MapKey(SDL_SCANCODE_KP_PLUS, 0x4E, 0, VK_ADD);
    MapKey(SDL_SCANCODE_KP_ENTER, 0x1C, 1, VK_RETURN);
    {
        static const unsigned char pad[9] = {0x4F, 0x50, 0x51, 0x4B, 0x4C, 0x4D, 0x47, 0x48, 0x49};

        for (i = 0; i < 9; i++)
            MapKey((SDL_Scancode)(SDL_SCANCODE_KP_1 + i), pad[i], 0, VK_NUMPAD1 + i);
    }
    MapKey(SDL_SCANCODE_KP_0, 0x52, 0, VK_NUMPAD0);
    MapKey(SDL_SCANCODE_KP_PERIOD, 0x53, 0, VK_DECIMAL);
    MapKey(SDL_SCANCODE_APPLICATION, 0x5D, 1, VK_APPS);
    MapKey(SDL_SCANCODE_LCTRL, 0x1D, 0, VK_CONTROL);
    MapKey(SDL_SCANCODE_LSHIFT, 0x2A, 0, VK_SHIFT);
    MapKey(SDL_SCANCODE_LALT, 0x38, 0, VK_MENU);
    MapKey(SDL_SCANCODE_LGUI, 0x5B, 1, VK_LWIN);
    MapKey(SDL_SCANCODE_RCTRL, 0x1D, 1, VK_CONTROL);
    MapKey(SDL_SCANCODE_RSHIFT, 0x36, 0, VK_SHIFT);
    MapKey(SDL_SCANCODE_RALT, 0x38, 1, VK_MENU);
    MapKey(SDL_SCANCODE_RGUI, 0x5C, 1, VK_RWIN);
}

#define KEY_QUEUE_SIZE 256
static InputKey g_keyQueue[KEY_QUEUE_SIZE];
static unsigned g_keyHead, g_keyTail;

static void QueueKey(const SDL_KeyboardEvent *event)
{
    const KeyMap *map;
    InputKey *key;

    if ((unsigned)event->scancode >= SDL_SCANCODE_COUNT)
        return;
    map = &g_keyMap[event->scancode];
    if (map->sc == 0)
        return;
    key = &g_keyQueue[g_keyTail % KEY_QUEUE_SIZE];
    key->down = event->down;
    key->repeat = event->repeat;
    key->scancode = map->sc;
    key->extended = map->ext;
    key->vk = map->vk;
    g_keyTail++;
    if (g_keyTail - g_keyHead > KEY_QUEUE_SIZE)
        g_keyHead = g_keyTail - KEY_QUEUE_SIZE; /* drop the oldest */
}

int Input_NextKey(InputKey *key)
{
    if (g_keyHead == g_keyTail)
        return 0;
    *key = g_keyQueue[g_keyHead % KEY_QUEUE_SIZE];
    g_keyHead++;
    return 1;
}

/* The game's rules for a window message: 0 drops it (see Input_SetMessageRules). */
static int MessageAllowed(const MSG *msg)
{
    if (msg->message == WM_QUIT && msg->hwnd == NULL) {
        if (g_quitFlag != NULL)
            *g_quitFlag = 1;
        return 0;
    }
    if (msg->message >= WM_MOUSEMOVE && msg->message <= WM_MBUTTONDBLCLK && g_cursorShown != NULL &&
        *g_cursorShown == 0)
        return 0;
    return 1;
}

static bool SDLCALL MessageHook(void *userdata, MSG *msg)
{
    (void)userdata;
    return MessageAllowed(msg) != 0;
}

void Input_SetMessageRules(int *quitFlag, const int *cursorShown)
{
    g_quitFlag = quitFlag;
    g_cursorShown = cursorShown;
}

void Input_WaitEvent(void)
{
    WaitMessage();
}

int Input_Open(void *nativeWindow)
{
    if (g_window != NULL)
        return 1;
    if (nativeWindow == NULL)
        return 0;
    /* The game ends the program through its own window procedure. */
    SDL_SetHint(SDL_HINT_QUIT_ON_LAST_WINDOW_CLOSE, "0");
    /* Mouse movement with the Windows pointer speed and acceleration, as the cursor had. */
    SDL_SetHint(SDL_HINT_MOUSE_RELATIVE_SYSTEM_SCALE, "1");
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS))
        return 0;
    g_sdlStarted = 1;
    /* the one wrap of the game's window, shared with the screen services (plat_window.c) */
    g_window = PlatSdl_Window(nativeWindow);
    if (g_window == NULL) {
        SDL_Quit();
        g_sdlStarted = 0;
        return 0;
    }
    SDL_SetWindowsMessageHook(MessageHook, NULL);
    BuildKeyMap();
    g_keyHead = g_keyTail = 0;
    g_mouseDx = g_mouseDy = 0.0f;
    g_wantRelative = 0;
    memset(&g_stats, 0, sizeof(g_stats));
    return 1;
}

void Input_Close(void)
{
    if (g_window != NULL)
        SDL_SetWindowRelativeMouseMode(g_window, false);
    SDL_SetWindowsMessageHook(NULL, NULL);
    if (g_window != NULL) {
        PlatSdl_Release();
        g_window = NULL;
    }
    if (g_sdlStarted) {
        SDL_Quit();
        g_sdlStarted = 0;
    }
}

int Input_IsOpen(void)
{
    return g_window != NULL;
}

void Input_Pump(InputPumpInfo *info)
{
    SDL_Event event;

    if (info != NULL)
        info->quitRequested = 0;
    if (g_window == NULL) {
        /* no SDL3: one message of the OS's own loop, by the same rules */
        MSG msg;

        if (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE) && MessageAllowed(&msg)) {
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
        return;
    }
    SDL_PumpEvents();
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_EVENT_KEY_DOWN:
            g_stats.keyDown++;
            QueueKey(&event.key);
            break;
        case SDL_EVENT_KEY_UP:
            g_stats.keyUp++;
            QueueKey(&event.key);
            break;
        case SDL_EVENT_MOUSE_MOTION:
            g_stats.mouseMotion++;
            if (g_wantRelative) {
                g_mouseDx += event.motion.xrel;
                g_mouseDy += event.motion.yrel;
            }
            break;
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
        case SDL_EVENT_MOUSE_BUTTON_UP:
            g_stats.mouseButton++;
            break;
        case SDL_EVENT_QUIT:
            if (info != NULL)
                info->quitRequested = 1;
            break;
        default:
            break;
        }
    }
}

void Input_MouseCapture(int on)
{
    g_wantRelative = on != 0;
    g_mouseDx = g_mouseDy = 0.0f;
    if (g_window != NULL)
        SDL_SetWindowRelativeMouseMode(g_window, g_wantRelative);
}

void Input_MouseTake(float *dx, float *dy)
{
    *dx = g_mouseDx;
    *dy = g_mouseDy;
    g_mouseDx = g_mouseDy = 0.0f;
}

unsigned Input_MouseButtons(void)
{
    /* The same left / middle / right bit order the sim uses. */
    return (unsigned)SDL_GetGlobalMouseState(NULL, NULL) & 7u;
}

void Input_GetStats(InputStats *stats)
{
    *stats = g_stats;
}
