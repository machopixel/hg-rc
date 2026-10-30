#include "w2n.h"

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
    name = SIM_PTR(unsigned char, 0x10a51ac0) + *(int16_t *)rec * 0x300 + *(int16_t *)(rec + 2) * 0x18;
    if (*(signed char *)name < 1)
        return NULL;
    image = *(const unsigned char **)(name + 0x14);
    if (image == NULL) {
        image = RENDER_LOAD(name, *SIM_PTR(void *, 0x1014ceb8));
        if (image == NULL)
            return NULL;
        *(const unsigned char **)(name + 0x14) = image;
    }
    *width = *(const int16_t *)image;
    *height = *(const int16_t *)(image + 2);
    return (void *)(image + 4);
}

/* The part all three builders share: 1/z and the colour bytes of one vertex. */
static void FinishVertex(unsigned char *v)
{
    *(float *)(v + 0xc) = F32(0x10111000) / *(float *)(v + 8);
    v[0x14] = (unsigned char)U32(0x10143fdc);
    v[0x15] = (unsigned char)U32(0x10143fe0);
    v[0x16] = (unsigned char)U32(0x10143fe4);
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
    RenderTexture tex;
    RenderTexture *handle;
    int16_t width, height;
    void *pixels;
    unsigned char *v;
    int i;

    if (count < 3)
        return 0;
    index += 0x100;
    pixels = Render_GetTextureImage(index, &width, &height);
    if (pixels == NULL)
        return 0;
    ReloadLumaIfPending();
    v = verts;
    for (i = 0; i < count; i++, v += 0x24) {
        FinishVertex(v);
        if (U32(0x1017b940) != 0)
            *(uint32_t *)(v + 0x10) = (*(uint32_t *)(v + 0x10) << 3) | 7;
        else
            *(uint32_t *)(v + 0x10) = *(uint32_t *)(v + 0x10) << 3;
    }
    tex.width = width;
    tex.height = height;
    tex.pixels = pixels;
    tex.palette = SIM_PTR(void, 0x10185760);
    handle = &tex;
    RENDER_CALL(2, RENDER_PLUGIN, verts, count, &handle);
    return 1;
}

// FUNCTION: HGSIM 0x10006744
int __cdecl Render_SubmitVerts_10006744(int index, int count, unsigned char *verts, int flag)
{
    RenderTexture tex;
    RenderTexture *handle;
    int16_t width, height;
    void *pixels;
    unsigned char *v;
    int i;

    if (count < 3)
        return 0;
    index += 0x100;
    pixels = Render_GetTextureImage(index, &width, &height);
    if (pixels == NULL)
        return 0;
    ReloadLumaIfPending();
    v = verts;
    for (i = 0; i < count; i++, v += 0x24) {
        FinishVertex(v);
        v[0x17] = 0xff;
        if (U32(0x1017b940) != 0)
            *(uint32_t *)(v + 0x10) = (*(uint32_t *)(v + 0x10) << 3) | 7;
        else
            *(uint32_t *)(v + 0x10) = *(uint32_t *)(v + 0x10) << 3;
    }
    tex.width = width;
    tex.height = height;
    tex.pixels = pixels;
    tex.palette = SIM_PTR(void, 0x10185760);
    handle = &tex;
    if (flag != 0) {
        RENDER_CALL(1, RENDER_PLUGIN, verts, count, &handle);
    } else if (U32(0x1017b940) != 0) {
        tex.key = 0xff; /* only set on this path; the others leave it unset */
        RENDER_CALL(6, RENDER_PLUGIN, verts, count, &handle);
    } else {
        RENDER_CALL(2, RENDER_PLUGIN, verts, count, &handle);
    }
    return 1;
}

// FUNCTION: HGSIM 0x100068d9
int __cdecl Render_SubmitVerts_100068d9(int index, int count, unsigned char *verts)
{
    RenderTexture tex;
    RenderTexture *handle;
    int16_t width, height;
    void *pixels;
    unsigned char *v;
    int i;

    if (count < 3)
        return 0;
    pixels = Render_GetTextureImage(index, &width, &height);
    if (pixels == NULL)
        return 0;
    ReloadLumaIfPending();
    v = verts;
    for (i = 0; i < count; i++, v += 0x24) {
        FinishVertex(v);
        if (U32(0x1017b940) != 0)
            *(uint32_t *)(v + 0x10) = 0xff;
        else
            *(uint32_t *)(v + 0x10) = 0x80;
        v[0x17] = 0xc8;
    }
    tex.key = 0xff;
    tex.width = width;
    tex.height = height;
    tex.pixels = pixels;
    tex.palette = SIM_PTR(void, 0x10185760);
    handle = &tex;
    if (U32(0x1017b940) != 0)
        RENDER_CALL(6, RENDER_PLUGIN, verts, count, &handle);
    else
        RENDER_CALL(2, RENDER_PLUGIN, verts, count, &handle);
    return 1;
}
