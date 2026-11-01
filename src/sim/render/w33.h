#pragma once
#include <stdint.h>

/*
 * Round 33: 1.2's Direct3D renderer (1.2's dx5draw.dll, plugin id 1) at the plugin boundary. The sim sends 1.0's
 * numbering, which our Vulkan renderer keeps; 1.2's renderers number the render modes and states differently
 * (DECISIONS.md, round 31): the D3D plugin's Render, RenderNoClip and SetState slots are wrapped to translate.
 */
void W33_WrapD3D12(void);

/* 1.2 [0x1014b954]: the 3DFX / PowerVR mode (round 33): vertex colours from the coloured light */
extern int32_t g_w33Voodoo;
