#include "w28.h"
#include "../../platform/input.h"
#include "../util/access.h"
#include "../unsorted_1005/w2h.h"
#include "../unsorted_1005/w7n.h"
#include "../unsorted_1005/w9c.h"

#define W28_ACTIVE() G32(0x1017b920)        /* 0 while the game window is in the background */
#define W28_QUIT() G32(0x1017b8b8)          /* set when the game should end */
#define W28_MODE_W() GS32(0x1017b918)       /* the screen-mode box, 640x480 in a mission */
#define W28_MODE_H() GS32(0x1017b91c)
#define W28_CLIPPED() G32(0x10179d64)       /* 1 while the mouse is held in the box */
#define W28_RECLIP() G32(0x10179d68)        /* set by the sim when the box must be set up again */

static int g_open, g_openFailed, g_captured;
float g_w28MouseX, g_w28MouseY;             /* the position inside the box, in screen-mode pixels */

void W28_InputReset(void)
{
    g_open = 0;
    g_openFailed = 0;
    g_captured = 0;
    g_w28FocusPaused = 0;
    Input_SetMessageRules(SIM_PTR(int, 0x1017b8b8), SIM_PTR(const int, 0x1017b93c));
}

void W28_InputExit(void)
{
    if (!g_open)
        return;
    Input_MouseCapture(0);
    Input_Close();
    g_open = 0;
}

int __cdecl W28_PumpKeys(void)
{
    InputPumpInfo info;
    InputKey key;
    int n = 0;

    Input_Pump(&info);
    while (Input_NextKey(&key)) {
        uint32_t lParam = (uint32_t)key.scancode << 16 | (uint32_t)key.extended << 24;

        if (key.down) {
            Key_SetState((uint32_t)key.vk, lParam, 1);
        } else {
            Input_QueueKey(key.vk);
            Key_SetState((uint32_t)key.vk, lParam, 0);
        }
        n++;
    }
    return n;
}

// FUNCTION: HGSIM 0x10057afc
int __cdecl Sim_10057afc(void)
{
    W28_PumpKeys();
#ifdef W28I_MUTATE /* the checker's negative check only */
    G32(0x1016686c) = 1;
#else
    G32(0x1016686c) = 0;
#endif
    G32(0x10166868) = 0;
    return 0;
}

// FUNCTION: HGSIM 0x10057f83
int16_t __cdecl Sim_10057f83(void)
{
    uint16_t key;

    Sim_10057aa9(&key);
    if (key == 0 && W28_PumpKeys() != 0)
        Sim_10057aa9(&key);
    return (int16_t)key;
}

// FUNCTION: HGSIM 0x10057b4e
void __cdecl Sim_10057b4e(uint32_t message, uint32_t wParam, uint32_t lParam)
{
    (void)message;
    (void)wParam;
    (void)lParam;
}

// FUNCTION: HGSIM 0x1007d948
int __cdecl Sim_1007d948(void)
{
    Input_MouseCapture(0);
    g_captured = 0;
    W28_CLIPPED() = 0;
    return 0;
}

// FUNCTION: HGSIM 0x1007d961
int __cdecl Sim_1007d961(int unused, int axis)
{
    (void)unused;
    if (W28_ACTIVE() != 0) {
        if (axis == 0)
            g_w28MouseY = (float)(W28_MODE_H() / 2);
        else if (axis == 1)
            g_w28MouseX = (float)(W28_MODE_W() / 2);
    }
    return 0;
}

// FUNCTION: HGSIM 0x1007d9d8
int __cdecl Sim_1007d9d8(int unused, int32_t *pos, uint32_t *buttons)
{
    float dx, dy;
    int32_t x, y;

    if (W28_ACTIVE() == 0) {
        if (W28_CLIPPED() != 0) {
            Input_MouseCapture(0);
            g_captured = 0;
            W28_CLIPPED() = 0;
        }
        *buttons = 0;
        return 0;
    }
    if (W28_CLIPPED() == 0 || W28_RECLIP() != 0) {
        Input_MouseCapture(1);
        g_captured = 1;
        Sim_1007d961(unused, 0);
        Sim_1007d961(unused, 1);
        W28_CLIPPED() = 1;
        W28_RECLIP() = 0;
    }
    if (buttons != 0)
        *buttons = Input_MouseButtons() & 7u;
    if (pos != 0) {
        Input_MouseTake(&dx, &dy);
        g_w28MouseX += dx;
        g_w28MouseY += dy;
        if (g_w28MouseX > (float)(W28_MODE_W() - 1))
            g_w28MouseX = (float)(W28_MODE_W() - 1);
        if (g_w28MouseX < 0.0f)
            g_w28MouseX = 0.0f;
        if (g_w28MouseY > (float)(W28_MODE_H() - 1))
            g_w28MouseY = (float)(W28_MODE_H() - 1);
        if (g_w28MouseY < 0.0f)
            g_w28MouseY = 0.0f;
        x = (int32_t)g_w28MouseX;
        y = (int32_t)g_w28MouseY;
        if (x >= 0 && x < W28_MODE_W() && y >= 0 && y < W28_MODE_H()) {
            pos[0] = (int32_t)(((uint32_t)y * 2u - (uint32_t)W28_MODE_H()) << 16) / W28_MODE_H();
            pos[1] = (int32_t)(((uint32_t)x * 2u - (uint32_t)W28_MODE_W()) << 16) / W28_MODE_W();
        }
    }
    return 0;
}

// FUNCTION: HGSIM 0x10086b99
void __cdecl Sim_10086b99(void)
{
    if (W28_ACTIVE() == 0)
        Input_WaitEvent();
    if (W28_QUIT() != 0)
        return;
    if (!g_open && !g_openFailed && G32(0x1017b8f0) != 0) {
        g_open = Input_Open((void *)(uintptr_t)G32(0x1017b8f0));
        g_openFailed = !g_open;
    }
    W28_PumpKeys();
}
