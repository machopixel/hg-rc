/*
 * Vulkan back end of HGVK.DLL. Everything the game hands the plugin arrives
 * here as triangle/line/point lists in 640x480 screen pixels.
 */
#ifndef HGVK_VK_H
#define HGVK_VK_H

#include <stdint.h>
#include <windows.h>

/* Same layout as the vertex input of the pipelines in vk.c. */
typedef struct HgvkVertex {
    float x, y;    /* screen pixels, 640x480 */
    float rhw;     /* 1 / camera depth; <= 0: screen space, farthest */
    uint32_t rgba; /* bytes R, G, B, A in memory order */
    float u, v;
    float iz;      /* 1 / camera depth for the depth buffer; <= 0: farthest */
    float fog;     /* round 33: D3D's vertex fog factor, the specular alpha byte +0x1b / 255 (1: no fog); not yet
                      read by the shaders */
} HgvkVertex;

/* Round 33: the fog FirstDevice asks for (flags bit 2), as 1.2's dx5draw takes it: FOGENABLE, FOGCOLOR (0xAARRGGBB
   from the sim, alpha unused by D3D's fog). Kept for the shader step; nothing draws with it yet. */
void Vk_SetFog(int enabled, uint32_t colour);

enum { HGVK_TRIANGLES, HGVK_LINES, HGVK_POINTS };

typedef struct HgvkTexture HgvkTexture;

/* Supplied by hgvk.c. */
void HgvkLog(const char *fmt, ...);

/*
 * Returns 0 on success. On failure everything is released again.
 * depthBuffer: use a W-buffer (see hgvk.c, Render) when the GPU has one.
 */
int Vk_Init(HWND hwnd, int depthBuffer);
void Vk_Shutdown(void);
int Vk_IsReady(void);

HgvkTexture *Vk_FindTexture(const void *key);
/*
 * palette: 256 entries 0x00RRGGBB. keyIndex < 0: no colour key. keyIndex HGVK_KEY_ALPHA: the palette's top byte
 * is a transparency, as 1.2's renderer reads it for a texture whose description has the flag +0x18 (its 16-bit
 * ARGB textures): alpha (15 - top 4 bits) * 17, no colour key.
 */
#define HGVK_KEY_ALPHA (-2)
HgvkTexture *Vk_CreateTexture(const void *key, int w, int h, const uint8_t *pixels,
                              const uint32_t *palette, int keyIndex);
/* Whether the texture was made with HGVK_KEY_ALPHA. */
int Vk_TextureHasAlpha(const HgvkTexture *t);
void Vk_DropTexture(const void *key);
/* Drops every cached texture at the end of the current frame (the texture palette changed). */
void Vk_RequestDropAllTextures(void);

/* sampler: bit 0 clamp, bit 1 linear filter. */
/* painter != 0: drawn without depth test or write, in submission order (1.2's particles, round 32) */
/* Clears the depth buffer at this point of the frame's draws (1.2's render state 0xc, our 0xd; 3dfxgear: a
   depth-only grBufferClear). */
void Vk_ClearDepth(void);
void Vk_AddDraw(int topology, int blend, int sampler, int keyed, int painter, HgvkTexture *tex,
                const HgvkVertex *verts, int count);

/*
 * Draws everything added since the last call, then the 2D overlays, and presents.
 * hud: 640x480 RGB565, rows hudStrideBytes apart, 0x0000 is transparent; what the
 * sim drew itself. hud2: the full-resolution layer (hud2.c) of hud2W x hud2H
 * premultiplied RGBA pixels covering the 4:3 picture, drawn over hud; NULL for none.
 */
void Vk_Present(const uint16_t *hud, int hudStrideBytes, const uint32_t *hud2, int hud2W,
                int hud2H);

#endif
