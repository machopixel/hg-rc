#include "w33.h"
#include "../fx/w32.h"
#include "../util/access.h"

typedef int32_t(__cdecl *W33RenderFn)(int32_t mode, void *surf, void *verts, int32_t count, void *tex);
typedef int32_t(__cdecl *W33StateFn)(uint32_t code, uint32_t value);

int32_t g_w33Voodoo;

static W33RenderFn s_w33Render, s_w33NoClip;
static W33StateFn s_w33State;

/* 1.2's renderers take 21 modes: 1.0's 0..4 unchanged, 1.0's 5..18 as 7..20 (1.2's 5 and 6 draw nothing) */
static int32_t W33_Mode(int32_t mode)
{
    return mode >= 5 && mode <= 18 ? mode + 2 : mode;
}

static int32_t __cdecl W33_Render(int32_t mode, void *surf, void *verts, int32_t count, void *tex)
{
    return s_w33Render(W33_Mode(mode), surf, verts, count, tex);
}

static int32_t __cdecl W33_RenderNoClip(int32_t mode, void *surf, void *verts, int32_t count, void *tex)
{
    return s_w33NoClip(W33_Mode(mode), surf, verts, count, tex);
}

/* 1.2's state codes are 1.0's minus 1; our own painter state (round 32) is for the Vulkan renderer only: 1.2's
   Direct3D renderer has no depth buffer and draws everything in the game's order */
static int32_t __cdecl W33_SetState(uint32_t code, uint32_t value)
{
    if (code == W32_STATE_PAINTER)
        return 0;
    return s_w33State(code - 1, value);
}

void W33_WrapD3D12(void)
{
    if (G32(0x109e3d30) == 0 || G32(0x109e3d58) == 0 || G32(0x109e3d40) == 0)
        return;
    s_w33Render = (W33RenderFn)(uintptr_t)G32(0x109e3d30);
    s_w33NoClip = (W33RenderFn)(uintptr_t)G32(0x109e3d58);
    s_w33State = (W33StateFn)(uintptr_t)G32(0x109e3d40);
    G32(0x109e3d30) = (uint32_t)(uintptr_t)W33_Render;
    G32(0x109e3d58) = (uint32_t)(uintptr_t)W33_RenderNoClip;
    G32(0x109e3d40) = (uint32_t)(uintptr_t)W33_SetState;
}
