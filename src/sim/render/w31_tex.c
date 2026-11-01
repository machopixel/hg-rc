#include "w31.h"
#include "w2m.h"
#include "w33.h"
#include "../util/access.h"

uint8_t g_w31TexNames[W31_GROUPS * W31_GROUP_SIZE];
int g_w31Tex;
uint32_t g_w31TexStart;
uint32_t g_w31TexLoaded, g_w31TexFallback, g_w31TexMissing;
W31Texture g_w31FileTex[2];

/* writable: the cache loader upper-cases the type in place */
static char g_w31TypeTex[4] = "TEX";

char *W31_TypeTex(void)
{
    return g_w31TypeTex;
}

void W31_DescribeFileTex(W31Texture *tex, const uint8_t *image, uint32_t handleAddr)
{
    tex->width = S16(image, 6);
    tex->height = S16(image, 8);
    tex->pixels = (void *)(image + 0xc + S16(image, 0xa) * 4);
    tex->palette = (void *)(image + 0xc);
#ifdef W31S_MUTATE /* the checker's negative check only: the palette one entry late */
    tex->palette = (void *)(image + 0x10);
#endif
    tex->key = 0;
    tex->unset14 = 0;
    tex->flag = 0;
    G32(handleAddr) = (uint32_t)(uintptr_t)tex;
}

/* 1.2's .TEX flag 0x10105048: set by its SimMain for the 3D mode or its 3DFX / PowerVR mode (round 33) */
int W31_TexMode(void)
{
#ifdef W31V_MUTATE /* the checker's negative check only: the 3D mode alone */
    return g_w31Tex != 0 && G32(0x1017b940) != 0;
#else
    return g_w31Tex != 0 && (G32(0x1017b940) != 0 || g_w33Voodoo != 0);
#endif
}

void W31_MirrorRows(uint8_t *pixels, int32_t width, int32_t height)
{
    int32_t half = width >> 1;
    int32_t row, i;

#ifdef W31T_MUTATE /* the checker's negative check only: the last row is left alone */
    height--;
#endif
    for (row = 0; row < height; row++) {
        uint8_t *p = pixels + row * width;
        uint8_t *q = p + width - 1;

        for (i = 0; i < half; i++, p++, q--) {
            uint8_t t = *p;

            *p = *q;
            *q = t;
        }
    }
}

uint8_t *W31_LoadImage(uint8_t *frame, int *loaded)
{
    uint8_t *image = (uint8_t *)(uintptr_t)U32(frame, W31_NAME_IMAGE);

    *loaded = 0;
    if (image != 0)
        return image;
    image = (uint8_t *)RENDER_LOAD(frame, g_w31TypeTex);
    if (image != 0) {
        g_w31TexLoaded++;
        U32(frame, W31_NAME_KIND) = W31_KIND_TEX;
        W31_MirrorRows(image + 0xc + S16(image, 0xa) * 4, S16(image, 6), S16(image, 8));
    } else {
        /* Diego's pick 1a: no .TEX, the 1.0 image */
        image = (uint8_t *)RENDER_LOAD(frame, (void *)(uintptr_t)G32(0x1014ceb8));
        if (image == 0) {
            g_w31TexMissing++;
            return 0;
        }
        g_w31TexFallback++;
        U32(frame, W31_NAME_KIND) = 0;
    }
    U32(frame, W31_NAME_IMAGE) = (uint32_t)(uintptr_t)image;
    *loaded = 1;
    return image;
}

// FUNCTION: HGSIM12 0x10002391
void *__cdecl W31_GetTexImage(int32_t index, int16_t *width, int16_t *height, void **palette)
{
    const uint8_t *rec;
    uint8_t *frame, *image;
    int loaded;

    if (index >= 0x200)
        return 0;
    rec = SIM_PTR(uint8_t, 0x10ac1b00) + index * 14;
    if (U16(rec, 6) == 0 || S16(rec, 8) < 0)
        return 0;
    frame = W31_NAME(S16(rec, 0), S16(rec, 2));
    if ((int8_t)frame[0] < 1)
        return 0;
    image = W31_LoadImage(frame, &loaded);
    if (image == 0)
        return 0;
    if (U32(frame, W31_NAME_KIND) != W31_KIND_TEX) {
        *width = S16(image, 0);
        *height = S16(image, 2);
        *palette = SIM_PTR(void, 0x10185760);
        return image + 4;
    }
    *width = S16(image, 6);
    *height = S16(image, 8);
    *palette = image + 0xc;
    return image + 0xc + S16(image, 0xa) * 4;
}
