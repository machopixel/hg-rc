/*
 * Input through SDL3 (docs/input.md; DECISIONS.md, round 28). With the SDL3 platform (Hooks_SdlPlatform, the
 * default in converted runs) the sim's message loop, keyboard and mouse functions are the converted ones of
 * src/sim/platform/w28_input.c (hooked by conv_w28.inc), which use the platform input layer (platform/input.h):
 * SDL3 runs the message loop around the game's window, the key events go to the sim's own key handling, the mouse
 * is captured and its movement kept in the game's screen-mode box. This file only starts and ends that per sim
 * load and logs it.
 */
#include <windows.h>

#include "../launcher/debug.h"
#include "../platform/input.h"
#include "../sim/platform/w28.h"
#include "../sim/unsorted_100b/w8y.h"
#include "hook.h"

static int g_active;

void HooksInput_OnSimLoaded(HMODULE sim)
{
    (void)sim;
    g_active = Hooks_SdlPlatform();
    g_w8yLoadPoll = g_active ? W28_AnimPoll : NULL;
    if (!g_active)
        return;
    W28_InputReset();
    DebugPrintf("HGSIM.DLL: SDL3 platform on (message loop, keyboard, mouse)\n");
}

/* Called when the sim has returned, before its DLL is unloaded. */
void HooksInput_OnSimExit(void)
{
    InputStats stats;

    if (!g_active || !Input_IsOpen())
        return;
    Input_GetStats(&stats);
    DebugPrintf("input: sim exit: %u key down, %u key up, %u mouse motion, %u mouse button\n", stats.keyDown,
                stats.keyUp, stats.mouseMotion, stats.mouseButton);
    W28_InputExit();
}
