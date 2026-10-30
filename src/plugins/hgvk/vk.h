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
} HgvkVertex;

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
/* palette: 256 entries 0x00RRGGBB. keyIndex < 0: no colour key. */
HgvkTexture *Vk_CreateTexture(const void *key, int w, int h, const uint8_t *pixels,
                              const uint32_t *palette, int keyIndex);
void Vk_DropTexture(const void *key);
/* Drops every cached texture at the end of the current frame (the texture palette changed). */
void Vk_RequestDropAllTextures(void);

/* sampler: bit 0 clamp, bit 1 linear filter. */
void Vk_AddDraw(int topology, int blend, int sampler, int keyed, HgvkTexture *tex,
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
