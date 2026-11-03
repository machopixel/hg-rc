/*
 * Round 31, T1: the texture name table in 1.2's layout, and 1.2's .TEX textures (DECISIONS.md, "Round 31").
 *
 * The table is ours (moved out of the DLL, whose 1.0 table at 0x10a51ac0 has 0x18-byte frames and the luma table
 * right after it): 512 groups of 32 frames of 0x1c bytes, 1.2's layout (1.2 0x10836d20). A frame:
 *   +0x00 name (the 1.0 code copies up to 0x12 bytes), +0x12 word (the value given at registration),
 *   +0x14 dword: 1.2 only clears it; here 1 when the image at +0x18 is a .TEX file (W31_KIND_TEX), else 0,
 *   +0x18 the image (the cached file), or 0.
 *
 * .TEX mode (W31_TexMode): 1.2 loads the world textures as .TEX files (the "1.22" header, a kind word, width +6,
 * height +8, palette count +0xa, that many palette dwords from +0xc, then 8-bit rows; each row mirrored once
 * after loading) and hands the renderer each texture's own palette. Here only with the Vulkan renderer
 * (g_w31Tex, set by the hook layer; the game's 3D mode [0x1017b940] is always on, round 42), as 1.2 sets its
 * flag 0x10105048.
 */
#ifndef HG_SIM_RENDER_W31_H
#define HG_SIM_RENDER_W31_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl
#endif

#define W31_GROUPS 0x200
#define W31_FRAMES 0x20
#define W31_GROUP_SIZE 0x380
#define W31_FRAME_SIZE 0x1c
#define W31_NAME_VALUE 0x12
#define W31_NAME_KIND 0x14
#define W31_NAME_IMAGE 0x18
#define W31_KIND_TEX 1

/*
 * 1.2's texture description for the renderer: 1.0's {width, height, pixels, palette, key} and two dwords more,
 * +0x14 (never written by 1.2; 0 here) and +0x18, set for a .TEX texture in 3D mode.
 */
typedef struct W31Texture {
    int32_t width;
    int32_t height;
    void *pixels;
    void *palette;
    int32_t key;
    int32_t unset14;
    int32_t flag;
} W31Texture;

extern uint8_t g_w31TexNames[W31_GROUPS * W31_GROUP_SIZE];
/* The frame record of a group (1.2 0x10836d20 + group * 0x380 + frame * 0x1c). */
#define W31_NAME(group, frame) \
    (g_w31TexNames + (uint32_t)(int32_t)(group) * W31_GROUP_SIZE + (uint32_t)(int32_t)(frame) * W31_FRAME_SIZE)

/* Set by the hook layer at sim load: the Vulkan renderer is in use (round 31, Diego's pick: 1.2 data with Vulkan). */
extern int g_w31Tex;
/* 1.2's .TEX flag (1.2 0x10105048): the Vulkan renderer (the 3D mode [0x1017b940] is always on, round 42). */
int W31_TexMode(void);
/* The start of the mission load (1.2 0x1012d3f8, timeGetTime in 1.2's sim main): the preload stops 30 s after. */
extern uint32_t g_w31TexStart;

/* Mirrors each row of an 8-bit image (width bytes, height rows) in place (1.2 0x10001b4c / 0x10002391). */
void W31_MirrorRows(uint8_t *pixels, int32_t width, int32_t height);
/*
 * Loads frame's image when it has none: 1.2 loads type "TEX" and mirrors it. Deviation (Diego's pick 1a): when
 * there is no .TEX, the 1.0 type ([0x1014ceb8], .XEL) is loaded instead and the frame's kind stays 0. Returns
 * the image or 0; *loaded tells whether it was loaded now.
 */
uint8_t *W31_LoadImage(uint8_t *frame, int *loaded);

/* The type "TEX" for the cache loader (writable: the loader upper-cases it in place). */
char *W31_TypeTex(void);
/* Texture loads of this sim load (logged per mission by the hook layer): .TEX, the .XEL fallback, none found. */
extern uint32_t g_w31TexLoaded, g_w31TexFallback, g_w31TexMissing;

/*
 * Round 31, T2: the two file textures (sky band [0x1017d0fc] named [0x1017d0d0], cloud layer [0x1017d100] named
 * [0x1017d0b0]) in .TEX mode: 1.2 loads them as .TEX and describes them with their own palette (1.2 0x1008fd87,
 * 0x1008dff3, 0x1008da61). Their descriptions are ours (7 dwords, flag 0); the handles [0x101cfc90] /
 * [0x101cfcb8] point at them.
 */
extern W31Texture g_w31FileTex[2];
/* Fills the description of a file texture from its .TEX image (1.2's layout) and points the handle at it. */
void W31_DescribeFileTex(W31Texture *tex, const uint8_t *image, uint32_t handleAddr);

/*
 * 1.2 0x10002391: the image of the 14-byte texture record index (< 0x200) in .TEX mode. 0 for no record, an empty
 * record (+6 zero, +8 negative), an empty name or no file; else *width / *height and *palette (the .TEX palette,
 * or with the .XEL fallback the game palette 0x10185760) and the pixels.
 */
void *__cdecl W31_GetTexImage(int32_t index, int16_t *width, int16_t *height, void **palette);

#endif
