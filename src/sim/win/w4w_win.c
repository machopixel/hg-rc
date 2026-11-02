#include "w4w.h"
#include "w4w_api.h"
#include "../util/access.h"
#include <stdarg.h>
#include "../../platform/music.h"
#include "../../platform/plat.h"

typedef struct {
    int32_t x, y;
} W4wPoint;

// FUNCTION: HGSIM 0x10056210
uint32_t __cdecl Win_JoyGetNumDevs(void)
{
    /* round 27: joystick support removed, no joystick slots (the original asks joyGetNumDevs) */
#ifdef W27J_MUTATE /* the checker's negative check only */
    return 1;
#else
    return 0;
#endif
}

// FUNCTION: HGSIM 0x1009eb85
int __cdecl Win_AuxSetVolume(int volume)
{
    /* 1.2's WAV music volume (1.2 0x1001603f, the music table's slot in place of this function): the music of
     * platform/music.h instead of the CD's aux volume */
    Music_SetVolume(volume < 0 ? 0u : (uint32_t)volume);
    return 1;
}

// FUNCTION: HGSIM 0x1009ebeb
int __cdecl Sim_1009ebeb(void)
{
    return Win_AuxSetVolume((int)G32(0x101321bc));
}

// FUNCTION: HGSIM 0x100b8fdb
uint64_t __cdecl Win_TimeGetTime(void)
{
    return Plat_TimeMs();
}

// FUNCTION: HGSIM 0x1007dbb1
void __cdecl Win_ClientRectToScreen(int32_t *rect, int width, int height)
{
    int32_t client[4];

    /* the original's two ClientToScreen calls (0, 0 and width - 1, height - 1): the client area's place on the
       screen (Plat_WindowRect, SDL3) */
    Plat_WindowRect((void *)(uintptr_t)G32(0x1017b8f0), client);
    rect[0] = client[0];
    rect[1] = client[1];
    rect[2] = client[0] + width - 1;
    rect[3] = client[1] + height - 1;
}

/* Win_ErrorBox (0x100d9fc5: MessageBeep, wvsprintfA, MessageBoxA) is deleted: no caller and no pointer in the DLL
   (DECISIONS.md, "Windows calls: the screen"). */
