/*
 * Platform input layer. The game never talks to SDL3 (or any OS API) for
 * keyboard, mouse or pads: the hook layer calls this interface and passes the
 * results on to the sim's own input tables (see docs/input.md).
 *
 * One backend implements it today, input_sdl3.c. A gamepad is one more device
 * in the same interface (Input_PollPad, added when it is wanted); a console
 * port provides another backend file with the same functions.
 */
#ifndef HG_PLATFORM_INPUT_H
#define HG_PLATFORM_INPUT_H

#ifdef __cplusplus
extern "C" {
#endif

/* What happened during one Input_Pump call. */
typedef struct InputPumpInfo {
    int quitRequested; /* the platform asked the program to end */
} InputPumpInfo;

/*
 * Starts the backend around the game's existing window (an HWND on Windows).
 * Safe to call again once it succeeded. Returns 1 on success, 0 on failure.
 */
int Input_Open(void *nativeWindow);
void Input_Close(void);
int Input_IsOpen(void);

/*
 * Runs the platform's message loop once. Everything the OS has queued for the
 * program is delivered, as the game's own loop did before. Without the backend
 * open (Input_Open failed or was not called) it handles one OS message by the
 * rules of Input_SetMessageRules, as the game's own loop did.
 */
void Input_Pump(InputPumpInfo *info);

/*
 * The game's two rules for the window messages the OS delivers (DECISIONS.md, round 28): while *cursorShown is 0
 * mouse messages are dropped, and the OS asking the program to end sets *quitFlag to 1. Both pointers stay valid
 * while the backend is open. They apply to Input_Pump with or without the backend open (without it, Input_Pump
 * runs the OS's own message loop once).
 */
void Input_SetMessageRules(int *quitFlag, const int *cursorShown);

/* Sleeps until the OS has something for the program (an event or a window message). */
void Input_WaitEvent(void);

/*
 * A key press or release in the form the sim expects: the PC scancode (1 to
 * 0x58), whether it is an extended key (arrows, right Ctrl/Alt, keypad Enter
 * and so on) and the Windows virtual key code. Key repeats are flagged.
 */
typedef struct InputKey {
    int down;
    int repeat;
    int scancode;
    int extended;
    int vk;
} InputKey;

/* Takes the oldest key event the last Input_Pump calls collected. Returns 0 when none. */
int Input_NextKey(InputKey *key);

/*
 * Mouse. While captured the pointer is hidden and held in the window, and
 * only its movement is reported (scaled by the system pointer speed settings,
 * so it feels like the Windows cursor did). Buttons: bit 0 left, bit 1 middle,
 * bit 2 right.
 */
void Input_MouseCapture(int on);
void Input_MouseTake(float *dx, float *dy); /* movement since the last call, then reset */
unsigned Input_MouseButtons(void);

/* Counters for the log: events the backend has seen since Input_Open. */
typedef struct InputStats {
    unsigned keyDown, keyUp, mouseMotion, mouseButton;
} InputStats;
void Input_GetStats(InputStats *stats);

#ifdef __cplusplus
}
#endif

#endif
