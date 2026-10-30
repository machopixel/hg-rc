#include "w2m.h"

#define LOCK_DISPLAY RENDER_SLOT(unsigned char *(__cdecl *)(int *), 0x109e3d48)
#define UNLOCK_DISPLAY RENDER_SLOT(void(__cdecl *)(void), 0x109e3d4c)
#define REFRESH_DISPLAY RENDER_SLOT(void(__cdecl *)(void), 0x109e3d50)
#define DISPLAY_LOCKED SIM_PTR(int32_t, 0x1017b984)

// FUNCTION: HGSIM 0x100877c3
int __cdecl PluginDisp_UnlockRefresh(void)
{
    if (*DISPLAY_LOCKED != 0)
        UNLOCK_DISPLAY();
    REFRESH_DISPLAY();
    return 0;
}

// FUNCTION: HGSIM 0x100877df
int __cdecl PluginDisp_Refresh_100877df(void)
{
    REFRESH_DISPLAY();
    return 0;
}

// FUNCTION: HGSIM 0x100877ec
int __cdecl PluginDisp_Refresh_100877ec(void)
{
    REFRESH_DISPLAY();
    return 0;
}

// FUNCTION: HGSIM 0x100877f9
int __cdecl PluginDisp_Refresh_100877f9(void)
{
    REFRESH_DISPLAY();
    return 0;
}

// FUNCTION: HGSIM 0x10087806
int __cdecl PluginDisp_Refresh_10087806(void)
{
    REFRESH_DISPLAY();
    return 0;
}

// FUNCTION: HGSIM 0x10087813
int __cdecl PluginDisp_Refresh_10087813(void)
{
    REFRESH_DISPLAY();
    return 0;
}

// FUNCTION: HGSIM 0x10087820
int __cdecl PluginDisp_FillRect(int x0, int y0, int x1, int y1, int color)
{
    uint32_t width = (uint32_t)x1 - (uint32_t)x0 + 1;
    uint32_t height = (uint32_t)y1 - (uint32_t)y0 + 1;
    int stride;
    unsigned char *row = LOCK_DISPLAY(&stride);
    int r, c;

    row += (uint32_t)stride * height + width;
    for (r = 0; r < (int)height; r++) {
        for (c = 0; c < (int)width; c++)
            ((uint16_t *)row)[c] = (uint16_t)color;
        row += stride;
    }
    UNLOCK_DISPLAY();
    return 0;
}

// FUNCTION: HGSIM 0x10087a0c
int __cdecl PluginDisp_Lock(void)
{
    int stride;
    unsigned char *pixels;

    *DISPLAY_LOCKED = 1;
    pixels = LOCK_DISPLAY(&stride);
    (*SIM_PTR(uint32_t *, 0x1015e698))[0] = (uint32_t)(size_t)pixels;
    (*SIM_PTR(uint32_t *, 0x1015e698))[1] = (uint32_t)stride - 1;
    return 0;
}
