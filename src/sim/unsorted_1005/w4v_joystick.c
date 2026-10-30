#include "w4v.h"
#include "../util/access.h"
#include "w3s.h"
#include "../unsorted_100f/w2e.h"

/* JOYINFOEX of the Windows multimedia API (0x34 bytes). */
typedef struct {
    uint32_t dwSize, dwFlags, dwXpos, dwYpos, dwZpos, dwRpos, dwUpos, dwVpos;
    uint32_t dwButtons, dwButtonNumber, dwPOV, dwReserved1, dwReserved2;
} W4vJoyInfoEx;

typedef uint32_t(__stdcall *W4vJoyGetPosExFn)(uint32_t id, W4vJoyInfoEx *info);

#define IAT_JOYGETPOSEX 0x10ac585cu

/* The double the original pushes as two dwords (low dword first). */
static double Scale(uint32_t lo, uint32_t hi)
{
    union {
        uint32_t u[2];
        double d;
    } v;

    v.u[0] = lo;
    v.u[1] = hi;
    return v.d;
}

// FUNCTION: HGSIM 0x10056dfa
uint32_t __cdecl Sim_10056dfa(const uint32_t *cfg, uint32_t *axes, uint32_t *buttons)
{
    W4vJoyInfoEx info;
    int n;
    int i;

    Mem_Fill(&info, 0, sizeof info);
    info.dwSize = sizeof info;
    if (buttons == 0 || axes == 0)
        return 1;
    for (i = 0; i < 6; i++)
        axes[i] = 0;
    buttons[1] = 0;
    buttons[0] = 0;
    if (G32(0x1017b920) == 0)
        return 0;
    info.dwFlags = cfg[1];
    if (SIM_API(W4vJoyGetPosExFn, IAT_JOYGETPOSEX)(cfg[0], &info) == 0) {
        buttons[0] = info.dwButtons;
        if ((cfg[1] & 0x40) != 0) {
            switch (info.dwPOV) {
            case 0:
                buttons[cfg[0x1a]] = buttons[cfg[0x1a]] | cfg[0x1e];
                break;
            case 9000:
                buttons[cfg[0x1b]] = buttons[cfg[0x1b]] | cfg[0x1f];
                break;
            case 18000:
                buttons[cfg[0x1c]] = buttons[cfg[0x1c]] | cfg[0x20];
                break;
            case 27000:
                buttons[cfg[0x1d]] = buttons[cfg[0x1d]] | cfg[0x21];
                break;
            }
        }
        axes[0] = (uint32_t)Sim_100570fb((int)info.dwYpos, (int)cfg[9], (int)cfg[3], Scale(cfg[0x10], cfg[0x11]));
        axes[1] = (uint32_t)Sim_100570fb((int)info.dwXpos, (int)cfg[8], (int)cfg[2], Scale(cfg[0xe], cfg[0xf]));
        n = 2;
        if ((cfg[1] & 4) != 0) {
            axes[2] = (uint32_t)Sim_100570fb((int)info.dwZpos, (int)cfg[10], (int)cfg[4], Scale(cfg[0x12], cfg[0x13]));
            n = 3;
        }
        if ((cfg[1] & 8) != 0) {
            axes[n] = (uint32_t)Sim_100570fb((int)info.dwRpos, (int)cfg[0xb], (int)cfg[5], Scale(cfg[0x14], cfg[0x15]));
            n++;
        }
        if ((cfg[1] & 0x10) != 0) {
            axes[n] = (uint32_t)Sim_100570fb((int)info.dwUpos, (int)cfg[0xc], (int)cfg[6], Scale(cfg[0x16], cfg[0x17]));
            n++;
        }
        if ((cfg[1] & 0x20) != 0)
            axes[n] = (uint32_t)Sim_100570fb((int)info.dwVpos, (int)cfg[0xd], (int)cfg[7], Scale(cfg[0x18], cfg[0x19]));
    }
    return 0;
}
