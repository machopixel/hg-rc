#include "w2m.h"
#include "w31.h"
#include "../../platform/plat.h"
#include "../util/access.h"

#define LUMA_CACHE SIM_PTR(unsigned char *, 0x1015cff0)
#define LUMA_TABLE SIM_PTR(unsigned char, 0x10ab1ae0)

// FUNCTION: HGSIM 0x1000656f
void __cdecl Render_LoadLumaTable(void)
{
    int row, col;

    if (*LUMA_CACHE == NULL)
        *LUMA_CACHE = RENDER_LOAD(SIM_PTR(void, 0x10a3ae80), *SIM_PTR(void *, 0x1014cf0c));
    for (row = 0; row < 32; row++)
        for (col = 0; col < 256; col++)
            LUMA_TABLE[((31 - row) << 8) + col] = (*LUMA_CACHE)[(row << 8) + col];
    RENDER_SLOT(void(__cdecl *)(void *), 0x109e3d24)(LUMA_TABLE);
}

/*
 * Round 31: 1.2's preload (0x10001b4c) in .TEX mode: the 1.0 loops, each image loaded as .TEX (W31_LoadImage) and
 * offered with its own palette and the descriptor's flag +0x18 set; the whole preload stops once 30 s have passed
 * since the mission load began (g_w31TexStart; checked after every 14-byte record).
 */
static void Render_PreloadTexTextures(void)
{
    int width = 0x100;
    int stop = 0;
    int pass, group, i, k;

    for (pass = 0; pass < 2 && !stop; pass++) {
        for (group = 0; group < 8 && !stop; group++) {
            for (i = 0; i < 0x200 && !stop; i++) {
                const unsigned char *rec = SIM_PTR(unsigned char, 0x10ac1b00) + i * 14;

                if ((*(const int16_t *)(rec + 8) & 1) != 0) {
                    int names = *(const int16_t *)rec;
                    int done = 0;

                    for (k = 0; k < 0x20 && !done; k++) {
                        uint8_t *name = W31_NAME(names, k);
                        uint8_t *image;
                        W31Texture tex;
                        W31Texture *handle;
                        int loaded;

                        if (name[0] == 0) {
                            done = 1;
                            continue;
                        }
                        if ((name[0] == 'W' && name[1] == 'X') != (pass == 1)) {
                            done = 1;
                            continue;
                        }
                        image = W31_LoadImage(name, &loaded);
                        if (image == NULL)
                            continue;
                        tex.flag = 0;
                        if (U32(name, W31_NAME_KIND) == W31_KIND_TEX) {
                            tex.width = S16(image, 6);
                            tex.height = S16(image, 8);
                            tex.palette = image + 0xc;
                            tex.pixels = image + 0xc + S16(image, 0xa) * 4;
                            tex.key = 0xff;
                            tex.flag = 1;
                        } else {
                            tex.width = S16(image, 0);
                            tex.height = S16(image, 2);
                            tex.palette = SIM_PTR(void, 0x10185760);
                            tex.key = 0xff;
                            tex.pixels = image + 4;
                        }
                        tex.unset14 = 0;
                        handle = &tex;
                        if (width == tex.width)
                            RENDER_SLOT(int(__cdecl *)(W31Texture **, int), 0x109e3d54)(&handle, S16(name, W31_NAME_VALUE));
                    }
                }
                if (Plat_TicksMs() - g_w31TexStart >= 30000u)
                    stop = 1;
            }
            width >>= 1;
        }
    }
}

// FUNCTION: HGSIM 0x10006e52
void __cdecl Render_PreloadTextures(void)
{
    int width = 0x100;
    int pass, group, i, k;

    if (W31_TexMode()) {
        Render_PreloadTexTextures();
        return;
    }
    for (pass = 0; pass < 2; pass++) {
        for (group = 0; group < 8; group++) {
            for (i = 0; i < 0x200; i++) {
                const unsigned char *rec = SIM_PTR(unsigned char, 0x10ac1b00) + i * 14;
                int names, done;

                if ((*(const int16_t *)(rec + 8) & 1) == 0)
                    continue;
                names = *(const int16_t *)rec;
                done = 0;
                for (k = 0; k < 0x20 && !done; k++) {
                    const signed char *name = (const signed char *)W31_NAME(names, k);
                    const int16_t *image;
                    RenderTexture tex;
                    RenderTexture *handle;
                    int isWX;

                    if (name[0] == 0) {
                        done = 1;
                        continue;
                    }
                    isWX = name[0] == 'W' && name[1] == 'X';
                    if (isWX != (pass == 1)) {
                        done = 1;
                        continue;
                    }
                    image = *(const int16_t *const *)(name + W31_NAME_IMAGE);
                    if (image == NULL)
                        continue;
                    tex.width = image[0];
                    tex.height = image[1];
                    tex.pixels = (void *)(image + 2);
                    tex.palette = SIM_PTR(void, 0x10185760);
                    tex.key = 0xff;
                    handle = &tex;
                    if (width == tex.width)
                        RENDER_SLOT(int(__cdecl *)(RenderTexture **, int), 0x109e3d54)(&handle, *(const int16_t *)(name + 0x12));
                }
            }
            width >>= 1;
        }
    }
}

/* Loads the file `name` into *file once and offers its texture, described at `tex`, through *handle. */
static void PreloadFile(const char *name, void **file, RenderTexture *tex, RenderTexture **handle)
{
    if (*file == NULL && name[0] != 0) {
        *file = RENDER_LOAD((void *)name, *SIM_PTR(void *, 0x1014ceb8));
        if (*file != NULL) {
            tex->width = ((const int16_t *)*file)[0];
            tex->height = ((const int16_t *)*file)[1];
            tex->pixels = (unsigned char *)*file + 4;
            *handle = tex;
        }
    }
    if (*file != NULL) {
        tex->palette = SIM_PTR(void, 0x10185760);
        RENDER_SLOT(int(__cdecl *)(RenderTexture **, int), 0x109e3d54)(handle, 0);
    }
}

/*
 * Round 31, T2: 1.2's preload of a file texture (0x1008fd87) in .TEX mode: the image loaded as .TEX (rows not
 * mirrored), its description filled from it. Bug fix: 1.2 fills the description only when it loads the image here
 * and then gives it the palette from a local it set only on that path (stack garbage when the image was loaded
 * before); here the description is filled from the image every time.
 */
static void PreloadTexFile(const char *name, void **file, W31Texture *tex, uint32_t handleAddr)
{
    if (*file == NULL && name[0] != 0) {
        *file = RENDER_LOAD((void *)name, W31_TypeTex());
        if (*file != NULL)
            g_w31TexLoaded++;
        else
            g_w31TexMissing++;
    }
    if (*file != NULL) {
        W31_DescribeFileTex(tex, (const uint8_t *)*file, handleAddr);
        RENDER_SLOT(int(__cdecl *)(W31Texture **, int), 0x109e3d54)(SIM_PTR(W31Texture *, handleAddr), 0);
    }
}

// FUNCTION: HGSIM 0x100a2a66
void __cdecl Render_PreloadFileTextures(void)
{
    if (W31_TexMode()) {
        PreloadTexFile(SIM_PTR(char, 0x1017d0d0), SIM_PTR(void *, 0x1017d0fc), &g_w31FileTex[0], 0x101cfc90);
        PreloadTexFile(SIM_PTR(char, 0x1017d0b0), SIM_PTR(void *, 0x1017d100), &g_w31FileTex[1], 0x101cfcb8);
        return;
    }
    PreloadFile(SIM_PTR(char, 0x1017d0d0), SIM_PTR(void *, 0x1017d0fc), SIM_PTR(RenderTexture, 0x101ccc80),
                SIM_PTR(RenderTexture *, 0x101cfc90));
    PreloadFile(SIM_PTR(char, 0x1017d0b0), SIM_PTR(void *, 0x1017d100), SIM_PTR(RenderTexture, 0x101cfca0),
                SIM_PTR(RenderTexture *, 0x101cfcb8));
}
