#include "w4w.h"
#include "w4w_api.h"
#include "../util/access.h"
#include <stdarg.h>
#include "../../platform/music.h"

#define AUX_DEVICE G32(0x1014c9b4)

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

// FUNCTION: HGSIM 0x1009eb52
int __cdecl Win_AuxGetVolume(void)
{
    uint32_t volume;

    if (AUX_DEVICE == 0xffffffffu)
        return -1;
    if (SIM_API(W4wAuxGetFn, W4W_IAT_AUXGETVOLUME)(AUX_DEVICE, &volume) != 0)
        return -1;
    return (int)(volume & 0xffff);
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
    return SIM_API(W4wU32Fn, W4W_IAT_TIMEGETTIME)();
}

// FUNCTION: HGSIM 0x1007dbb1
void __cdecl Win_ClientRectToScreen(int32_t *rect, int width, int height)
{
    W4wPoint pt;

    pt.x = 0;
    pt.y = 0;
    SIM_API(W4wBool2Fn, W4W_IAT_CLIENTTOSCREEN)((void *)(uintptr_t)G32(0x1017b8f0), &pt);
    rect[0] = pt.x;
    rect[1] = pt.y;
    pt.x = width - 1;
    pt.y = height - 1;
    SIM_API(W4wBool2Fn, W4W_IAT_CLIENTTOSCREEN)((void *)(uintptr_t)G32(0x1017b8f0), &pt);
    rect[2] = pt.x;
    rect[3] = pt.y;
}

// FUNCTION: HGSIM 0x1007d961
int __cdecl Win_CenterCursorAxis(int unused, int axis)
{
    W4wPoint pt;

    (void)unused;
    if (G32(0x1017b920) != 0) {
        SIM_API(W4wBoolPtrFn, W4W_IAT_GETCURSORPOS)(&pt);
        SIM_API(W4wBool2Fn, W4W_IAT_SCREENTOCLIENT)((void *)(uintptr_t)G32(0x1017b8f0), &pt);
        if (axis == 0)
            pt.y = GS32(0x1017b91c) / 2;
        else if (axis == 1)
            pt.x = GS32(0x1017b918) / 2;
        SIM_API(W4wBool2Fn, W4W_IAT_CLIENTTOSCREEN)((void *)(uintptr_t)G32(0x1017b8f0), &pt);
        SIM_API(W4wInt2Fn, W4W_IAT_SETCURSORPOS)(pt.x, pt.y);
    }
    return 0;
}

// FUNCTION: HGSIM 0x1007d9d8
int __cdecl Win_PollMouse(int unused, int32_t *pos, uint32_t *buttons)
{
    W4wPoint pt;

    if (G32(0x1017b920) == 0) {
        if (G32(0x10179d64) != 0) {
            SIM_API(W4wBoolPtrFn, W4W_IAT_CLIPCURSOR)(0);
            G32(0x10179d64) = 0;
        }
        *buttons = 0;
        return 0;
    }
    if (G32(0x10179d64) == 0 || G32(0x10179d68) != 0) {
        Win_ClientRectToScreen((int32_t *)GBYTES(0x10373040), GS32(0x1017b918), GS32(0x1017b91c));
        SIM_API(W4wBoolPtrFn, W4W_IAT_CLIPCURSOR)(GBYTES(0x10373040));
        Win_CenterCursorAxis(unused, 0);
        Win_CenterCursorAxis(unused, 1);
        G32(0x10179d64) = 1;
        G32(0x10179d68) = 0;
    }
    if (buttons != 0) {
        W4wIntFn key = SIM_API(W4wIntFn, W4W_IAT_GETASYNCKEYSTATE);
        int16_t left = (int16_t)((key(1) & 0x8000) != 0);
        int16_t right = (int16_t)((key(2) & 0x8000) != 0);
        int16_t middle = (int16_t)((key(4) & 0x8000) != 0);

        *buttons = (uint32_t)((int32_t)right << 2 | (int32_t)middle << 1 | (int32_t)left);
    }
    if (pos != 0) {
        if (SIM_API(W4wBoolPtrFn, W4W_IAT_GETCURSORPOS)(&pt) != 0) {
            SIM_API(W4wBool2Fn, W4W_IAT_SCREENTOCLIENT)((void *)(uintptr_t)G32(0x1017b8f0), &pt);
            if (pt.x >= 0 && pt.x < GS32(0x1017b918) && pt.y >= 0 && pt.y < GS32(0x1017b91c)) {
                pos[0] = (int32_t)((((uint32_t)pt.y * 2u) - G32(0x1017b91c)) << 16) / GS32(0x1017b91c);
                pos[1] = (int32_t)((((uint32_t)pt.x * 2u) - G32(0x1017b918)) << 16) / GS32(0x1017b918);
            }
        }
    }
    return 0;
}

// FUNCTION: HGSIM 0x10086b99
void __cdecl Win_PumpMessage(void)
{
    uint32_t msg[7];

    if (G32(0x1017b920) == 0)
        SIM_API(W4wPtrFn, W4W_IAT_WAITMESSAGE)();
    if (G32(0x1017b8b8) != 0)
        return;
    if (SIM_API(W4wPeekMessageFn, W4W_IAT_PEEKMESSAGE)(msg, 0, 0, 0, 1) == 0)
        return;
    while (G32(0x1017b93c) == 0 && msg[1] >= 0x200 && msg[1] <= 0x209) {
        if (SIM_API(W4wPeekMessageFn, W4W_IAT_PEEKMESSAGE)(msg, 0, 0, 0, 1) == 0)
            break;
    }
    if (msg[0] != 0 && msg[1] == 0x12) {
        G32(0x1017b8b8) = 1;
    } else {
        SIM_API(W4wBoolPtrFn, W4W_IAT_TRANSLATEMESSAGE)(msg);
        SIM_API(W4wBoolPtrFn, W4W_IAT_DISPATCHMESSAGE)(msg);
    }
}

// FUNCTION: HGSIM 0x100d9fc5
int __cdecl Win_ErrorBox(const char *format, ...)
{
    char text[256];
    va_list args;

    SIM_API(W4wIntFn, W4W_IAT_MESSAGEBEEP)(0x30);
    va_start(args, format);
    SIM_API(W4wWvsprintfFn, W4W_IAT_WVSPRINTF)(text, format, (void *)args);
    va_end(args);
    return SIM_API(W4wMessageBoxFn, W4W_IAT_MESSAGEBOX)(
        SIM_API(W4wPtrFn, W4W_IAT_GETFOCUS)(), text, (const char *)GBYTES(0x10152460), 0x12030);
}
