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
#include "plat.h"
#include "plat_sdl3.h"

/* The Windows virtual-key codes the sim works in (fixed numbers; windows.h has them on Windows) */
#ifndef VK_RETURN
#define VK_BACK 0x08
#define VK_TAB 0x09
#define VK_RETURN 0x0D
#define VK_SHIFT 0x10
#define VK_CONTROL 0x11
#define VK_MENU 0x12
#define VK_PAUSE 0x13
#define VK_CAPITAL 0x14
#define VK_ESCAPE 0x1B
#define VK_SPACE 0x20
#define VK_PRIOR 0x21
#define VK_NEXT 0x22
#define VK_END 0x23
#define VK_HOME 0x24
#define VK_LEFT 0x25
#define VK_UP 0x26
#define VK_RIGHT 0x27
#define VK_DOWN 0x28
#define VK_SNAPSHOT 0x2C
#define VK_INSERT 0x2D
#define VK_DELETE 0x2E
#define VK_LWIN 0x5B
#define VK_RWIN 0x5C
#define VK_APPS 0x5D
#define VK_NUMPAD0 0x60
#define VK_NUMPAD1 0x61
#define VK_MULTIPLY 0x6A
#define VK_ADD 0x6B
#define VK_SUBTRACT 0x6D
#define VK_DECIMAL 0x6E
#define VK_DIVIDE 0x6F
#define VK_F1 0x70
#define VK_F11 0x7A
#define VK_F12 0x7B
#define VK_NUMLOCK 0x90
#define VK_SCROLL 0x91
#define VK_OEM_1 0xBA
#define VK_OEM_PLUS 0xBB
#define VK_OEM_COMMA 0xBC
#define VK_OEM_MINUS 0xBD
#define VK_OEM_PERIOD 0xBE
#define VK_OEM_2 0xBF
#define VK_OEM_3 0xC0
#define VK_OEM_4 0xDB
#define VK_OEM_5 0xDC
#define VK_OEM_6 0xDD
#define VK_OEM_7 0xDE
#define VK_OEM_102 0xE2
#endif

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
/* the other way, for the key lookups (Plat_KeyChar): a virtual key's first scancode in BuildKeyMap's order */
static SDL_Scancode g_vkScancode[256];
static int g_keyMapBuilt;

static void MapKey(SDL_Scancode usb, int sc, int ext, int vk)
{
    g_keyMap[usb].sc = sc;
    g_keyMap[usb].ext = ext;
    g_keyMap[usb].vk = vk;
    if (vk > 0 && vk < 256 && g_vkScancode[vk] == SDL_SCANCODE_UNKNOWN)
        g_vkScancode[vk] = usb;
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
    g_keyMapBuilt = 1;
}

/* Windows-1252's characters 0x80..0x9F, as Unicode */
static const uint16_t g_cp1252High[32] = {
    0x20AC, 0, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021, 0x02C6, 0x2030, 0x0160, 0x2039, 0x0152, 0, 0x017D, 0,
    0, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014, 0x02DC, 0x2122, 0x0161, 0x203A, 0x0153, 0, 0x017E,
    0x0178};

/*
 * MapVirtualKeyA(key, MAPVK_VK_TO_CHAR) in SDL3 (round P2): the character of the physical key the sim's key code
 * stands for, without modifiers, on the user's keyboard layout, in the Windows code page (1252), the English letters
 * upper case; 0 when the key types none. (The key codes come from input_sdl3.c's table, which follows the US
 * positions: SDL answers for the physical key, as the original game's own Windows key codes did.)
 */
uint32_t Plat_KeyChar(uint32_t key)
{
    SDL_Keycode k;
    int i;

    if (!g_keyMapBuilt)
        BuildKeyMap();
    if (key >= 256 || g_vkScancode[key] == SDL_SCANCODE_UNKNOWN)
        return 0;
    k = SDL_GetKeyFromScancode(g_vkScancode[key], SDL_KMOD_NONE, false);
    switch (k) {                        /* the keypad: Windows gives its characters whatever Num Lock is */
    case SDLK_KP_0: case SDLK_KP_1: case SDLK_KP_2: case SDLK_KP_3: case SDLK_KP_4:
    case SDLK_KP_5: case SDLK_KP_6: case SDLK_KP_7: case SDLK_KP_8: case SDLK_KP_9:
        return (uint32_t)('0' + (k == SDLK_KP_0 ? 0 : k - SDLK_KP_1 + 1));
    case SDLK_KP_MULTIPLY:
        return '*';
    case SDLK_KP_PLUS:
        return '+';
    case SDLK_KP_MINUS:
        return '-';
    case SDLK_KP_PERIOD:
        return '.';
    case SDLK_KP_DIVIDE:
        return '/';
    default:
        break;
    }
    if ((k & SDLK_SCANCODE_MASK) != 0 || k == 0)
        return 0;
    if (k >= 'a' && k <= 'z')
        return (uint32_t)(k - 'a' + 'A');   /* only the English letters come upper case (Windows: 'A', but 0xF1 ñ) */
    if (k < 0x100)
        return (uint32_t)k;                 /* Latin-1 is Windows-1252 from 0xA0 on */
    for (i = 0; i < 32; i++) {
        if (g_cp1252High[i] != 0 && (SDL_Keycode)g_cp1252High[i] == k)
            return (uint32_t)(0x80 + i);
    }
    return 0;
}

/* GetKeyState in SDL3 (round P2), for the modifier keys the sim asks about (Shift, Ctrl, Alt) and the two locks:
   held as Windows answers it (the high byte and bit 7 set: 0xFF80), toggled bit 0 */
int16_t Plat_KeyState(int key)
{
    SDL_Keymod mod = SDL_GetModState();
    int down = 0, toggled = 0;

    switch (key) {
    case VK_SHIFT:
        down = (mod & SDL_KMOD_SHIFT) != 0;
        break;
    case VK_CONTROL:
        down = (mod & SDL_KMOD_CTRL) != 0;
        break;
    case VK_MENU:
        down = (mod & SDL_KMOD_ALT) != 0;
        break;
    case VK_CAPITAL:
        toggled = (mod & SDL_KMOD_CAPS) != 0;
        break;
    case VK_NUMLOCK:
        toggled = (mod & SDL_KMOD_NUM) != 0;
        break;
    default:
        break;
    }
    return (int16_t)((down ? 0xFF80 : 0) | (toggled ? 1 : 0));
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
