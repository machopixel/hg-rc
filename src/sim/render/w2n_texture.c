#include "w2n.h"
#include "w31.h"
#include "w33.h"

#define F32(addr) (*SIM_PTR(float, addr))
#define U32(addr) (*SIM_PTR(uint32_t, addr))
#define RENDER_PLUGIN SIM_PTR(void, 0x109e3dc0)
#define RENDER_CALL RENDER_SLOT(int(__cdecl *)(int, void *, void *, int, void *), 0x109e3d30)

/* The (cached) image behind a 14-byte texture record or one of the two fixed images. */
// FUNCTION: HGSIM 0x100072ff
void *__cdecl Render_GetTextureImage(int index, int16_t *width, int16_t *height)
{
    const unsigned char *image = NULL;
    unsigned char *rec;
    unsigned char *name;

    if (index >= 0x200)
        return NULL;
    if (U32(0x1015cfec) != 0 && (*SIM_PTR(int32_t, 0x1012da38) == index - 0x100 || index < 0x100)) {
        if (U32(0x1015cffc) == 0)
            U32(0x1015cffc) = (uint32_t)(uintptr_t)RENDER_LOAD(SIM_PTR(void, 0x1012dbdc), *SIM_PTR(void *, 0x1014ceb8));
        if (U32(0x1015d000) == 0)
            U32(0x1015d000) = (uint32_t)(uintptr_t)RENDER_LOAD(SIM_PTR(void, 0x1012dbe8), *SIM_PTR(void *, 0x1014ceb8));
        if (U32(0x1015cffc) != 0 && U32(0x1015d000) != 0) {
            image = (const unsigned char *)(uintptr_t)(index < 0x100 ? U32(0x1015cffc) : U32(0x1015d000));
            *width = *(const int16_t *)image;
            *height = *(const int16_t *)(image + 2);
            U32(0x1015cff8) = 0;
            return (void *)(image + 4);
        }
    }
    rec = SIM_PTR(unsigned char, 0x10ac1b00) + index * 14;
    if (*(uint16_t *)(rec + 6) == 0 || *(int16_t *)(rec + 8) < 0)
        return NULL;
    name = W31_NAME(*(int16_t *)rec, *(int16_t *)(rec + 2));
    if (*(signed char *)name < 1)
        return NULL;
    image = *(const unsigned char **)(name + W31_NAME_IMAGE);
    if (image == NULL) {
        image = RENDER_LOAD(name, *SIM_PTR(void *, 0x1014ceb8));
        if (image == NULL)
            return NULL;
        *(const unsigned char **)(name + W31_NAME_IMAGE) = image;
    }
    *width = *(const int16_t *)image;
    *height = *(const int16_t *)(image + 2);
    return (void *)(image + 4);
}

/*
 * Round 31: the image of a submitter. In .TEX mode 1.2's getter (W31_GetTexImage) with the texture's own palette
 * (1.2 0x100010d9 / 0x100012c5 / 0x10001513); otherwise 1.0's with the game palette.
 */
static void *TexImage(int index, int16_t *width, int16_t *height, void **palette)
{
    if (W31_TexMode())
        return W31_GetTexImage(index, width, height, palette);
    *palette = SIM_PTR(void, 0x10185760);
    return Render_GetTextureImage(index, width, height);
}

/*
 * The part all three builders share: 1/z and the colour bytes of one vertex. 1.2 (0x100010d9 / 0x100012c5 /
 * 0x10001513): in its 3DFX / PowerVR mode (g_w33Voodoo, 1.2 [0x1014b954]) the colour is the vertex's specular colour
 * (the coloured light of Sim_10058c5e, put there by the queued polygon) times the channel factors / 256; the first
 * two builders then also make the vertex opaque (round 33).
 */
static void FinishVertex(unsigned char *v, int opaque)
{
    uint32_t spec;

    *(float *)(v + 0xc) = F32(0x10111000) / *(float *)(v + 8);
    if (g_w33Voodoo == 0) {
        v[0x14] = (unsigned char)U32(0x10143fdc);
        v[0x15] = (unsigned char)U32(0x10143fe0);
        v[0x16] = (unsigned char)U32(0x10143fe4);
        return;
    }
    spec = *(const uint32_t *)(v + 0x18);
    v[0x14] = (unsigned char)(U32(0x10143fdc) * (spec >> 16 & 0xff) >> 8);
    v[0x15] = (unsigned char)(U32(0x10143fe0) * (spec >> 8 & 0xff) >> 8);
#ifdef W2NV_MUTATE /* the checker's negative check only: blue from green */
    v[0x16] = (unsigned char)(U32(0x10143fe4) * (spec >> 8 & 0xff) >> 8);
#else
    v[0x16] = (unsigned char)(U32(0x10143fe4) * (spec & 0xff) >> 8);
#endif
    if (opaque)
        v[0x17] = 0xff;
}

/* Runs the luma-table reload the first call after [0x1012da34] was set. */
static void ReloadLumaIfPending(void)
{
    if (U32(0x1012da34) != 0) {
        U32(0x1012da34) = 0;
        Render_LoadLumaTable();
    }
}

// FUNCTION: HGSIM 0x10006609
int __cdecl Render_SubmitVerts_10006609(int index, int count, unsigned char *verts)
{
    W31Texture tex;
    W31Texture *handle;
    int16_t width, height;
    void *pixels, *palette;
    unsigned char *v;
    int i;

    if (count < 3)
        return 0;
    index += 0x100;
    pixels = TexImage(index, &width, &height, &palette);
    if (pixels == NULL)
        return 0;
    ReloadLumaIfPending();
    v = verts;
    for (i = 0; i < count; i++, v += 0x24) {
        FinishVertex(v, 1);
        *(uint32_t *)(v + 0x10) = (*(uint32_t *)(v + 0x10) << 3) | 7;
    }
    tex.width = width;
    tex.height = height;
    tex.pixels = pixels;
    tex.palette = palette;
    tex.unset14 = 0;
    tex.flag = 0;
    handle = &tex;
    RENDER_CALL(2, RENDER_PLUGIN, verts, count, &handle);
    return 1;
}

// FUNCTION: HGSIM 0x10006744
int __cdecl Render_SubmitVerts_10006744(int index, int count, unsigned char *verts, int flag)
{
    W31Texture tex;
    W31Texture *handle;
    int16_t width, height;
    void *pixels, *palette;
    unsigned char *v;
    int i;

    if (count < 3)
        return 0;
    index += 0x100;
    pixels = TexImage(index, &width, &height, &palette);
    if (pixels == NULL)
        return 0;
    ReloadLumaIfPending();
    v = verts;
    for (i = 0; i < count; i++, v += 0x24) {
        FinishVertex(v, 1);
        /* 1.2 (0x100012c5) keeps the caller's alpha (0x40 / 0x80 / [0x101cfc8c]); it sets 0xff only for its
           3DFX / PowerVR vertex colours, which our renderers never take (round 31: 1.2 with Vulkan) */
        if (!g_w31Tex)
            v[0x17] = 0xff;
        *(uint32_t *)(v + 0x10) = (*(uint32_t *)(v + 0x10) << 3) | 7;
    }
    tex.width = width;
    tex.height = height;
    tex.pixels = pixels;
    tex.palette = palette;
    tex.unset14 = 0;
    tex.flag = 0;
    handle = &tex;
    if (flag != 0) {
        RENDER_CALL(1, RENDER_PLUGIN, verts, count, &handle);
    } else {
        tex.key = 0xff; /* only set on this path; the other leaves it unset */
        tex.flag = W31_TexMode(); /* 1.2 (0x100012c5): a .TEX texture in 3D mode */
        /* 1.2 asks for mode 8: its renderers renumbered the modes, its 8 is 1.0's 6 (DECISIONS.md, round 31) */
        RENDER_CALL(6, RENDER_PLUGIN, verts, count, &handle);
    }
    return 1;
}

// FUNCTION: HGSIM 0x100068d9
int __cdecl Render_SubmitVerts_100068d9(int index, int count, unsigned char *verts)
{
    W31Texture tex;
    W31Texture *handle;
    int16_t width, height;
    void *pixels, *palette;
    unsigned char *v;
    int i;

    if (count < 3)
        return 0;
    pixels = TexImage(index, &width, &height, &palette);
    if (pixels == NULL)
        return 0;
    ReloadLumaIfPending();
    v = verts;
    for (i = 0; i < count; i++, v += 0x24) {
        FinishVertex(v, 0);
        *(uint32_t *)(v + 0x10) = 0xff;
        v[0x17] = 0xc8;
    }
    tex.key = 0xff;
    tex.width = width;
    tex.height = height;
    tex.pixels = pixels;
    tex.palette = palette;
    tex.unset14 = 0;
    tex.flag = W31_TexMode(); /* 1.2: a .TEX texture in 3D mode */
    handle = &tex;
    /* 1.2 asks for mode 8: its renderers renumbered the modes, its 8 is 1.0's 6 (DECISIONS.md, round 31) */
    RENDER_CALL(6, RENDER_PLUGIN, verts, count, &handle);
    return 1;
}
