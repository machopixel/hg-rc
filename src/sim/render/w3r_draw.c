#include "w3r.h"
#include "w2m.h"
#include "w31.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x100d1e10
void __cdecl Render_SubmitVerts_100d1e10(void *surface, void *pixels, short width, short height, int count, void *verts, int unused1, int unused2)
{
    /* round 31: 1.2's 7-dword description, flag 0 (HGVK reads the flag; the 1.0 layout left it to the stack) */
    W31Texture tex;
    W31Texture *handle = &tex;

    (void)unused1;
    (void)unused2;
    if (height > 0x100)
        height = 0x100;
    if (height > width)
        height = width;
    if (G32(0x1017e610) == 0) {
        const uint8_t *src = (const uint8_t *)(uintptr_t)G32(0x1015cff0);
        uint8_t *dst = GBYTES(0x10185c60);
        int row, col;

        for (row = 0; row < 32; row++)
            for (col = 0; col < 256; col++)
                dst[((31 - row) << 8) + col] = src[(row << 8) + col];
        RENDER_SLOT(void(__cdecl *)(void *), 0x109e3d24)(dst);
        G32(0x1017e610) = 1;
    }
    tex.width = width;
    tex.height = height;
    tex.pixels = pixels;
    tex.palette = GBYTES(0x10185760);
    tex.unset14 = 0;
    tex.flag = 0;
    /* tex.key stays unset, as in the original */
    RENDER_SLOT(int(__cdecl *)(int, void *, void *, int, void *), 0x109e3d30)(6, surface, verts, count, &handle);
}

// FUNCTION: HGSIM 0x100a78d9
int __cdecl Sim_100a78d9(int arg)
{
    return ((int(__cdecl *)(int))(uintptr_t)G32(0x1017d15c))(arg);
}
