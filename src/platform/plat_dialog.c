/* Dialogs of the platform layer (plat.h): the C runtime's assert box on SDL3, the same on every system. */
#include <SDL3/SDL.h>
#include "plat.h"

int Plat_AssertBox(const char *text, const char *caption)
{
    static const SDL_MessageBoxButtonData buttons[3] = {
        {0, PLAT_ASSERT_ABORT, "Abort"},
        {0, PLAT_ASSERT_RETRY, "Retry"},
        {SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT | SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, PLAT_ASSERT_IGNORE, "Ignore"},
    };
    SDL_MessageBoxData box;
    int pressed = PLAT_ASSERT_ABORT;

    SDL_zero(box);
    box.flags = SDL_MESSAGEBOX_ERROR | SDL_MESSAGEBOX_BUTTONS_LEFT_TO_RIGHT;
    box.title = caption;
    box.message = text;
    box.numbuttons = 3;
    box.buttons = buttons;
    if (!SDL_ShowMessageBox(&box, &pressed))
        return PLAT_ASSERT_ABORT;   /* no box could be shown: as Abort */
    return pressed;
}

void Plat_DebugBreak(void)
{
    SDL_TriggerBreakpoint();
}
