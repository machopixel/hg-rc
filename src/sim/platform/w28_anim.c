#include "w28.h"
#include "../../platform/plat.h"
#include "../util/access.h"
#include "../win/w4a.h"
#include "../win/w4v.h"
#include "../unsorted_100b/w6g.h"
#include "../unsorted_100b/w8y.h"
#include "../unsorted_100b/w9e.h"
#include "../unsorted_100f/w2e.h"
#include "../unsorted_100f/w8z.h"

#define W28_DRIVER() G32(0x1015e688)        /* the display driver's table */
#define W28_SCREEN() G32(0x1015e68c)        /* the screen driver's table */
#define W28_BOX_X() G32(0x10199440)         /* the animation's 640x480 box on the screen */
#define W28_BOX_Y() G32(0x10199444)
#define W28_ANIM GBYTES(0x1017d5a0)         /* the animation's draw record */

typedef void(__cdecl *W28PaletteFn)(uint32_t first, uint32_t count, void *colours, uint32_t now);
typedef void(__cdecl *W28FadeFn)(void *colours, uint32_t steps);
typedef int32_t(__cdecl *W28AskFn)(void);
typedef void(__cdecl *W28VoidFn)(void);
typedef void(__cdecl *W28RectFn)(uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1);
typedef void(__cdecl *W28DrawFn)(void *record, uint32_t shapes, uint32_t frame, uint32_t x, uint32_t y);
typedef void(__cdecl *W28ClearFn)(void *record, uint32_t colour);
typedef void(__cdecl *W28ShapePalFn)(uint32_t shapes, uint32_t index, void *colours);
typedef uint32_t(__cdecl *W28CountFn)(uint32_t shapes);
typedef void(__cdecl *W28LogFn)(const char *text);

#define W28_DRAW(shapes, frame, x, y) SIM_FN(W28DrawFn, 0x100e91dc)(W28_ANIM, (shapes), (frame), (x), (y))

int g_w28Anim;                             /* 1 while the loading animation runs on the game's thread */
uint32_t g_w28AnimLast;                    /* Plat_TicksMs of its last tick */

static int32_t W28_BufferReady(void)
{
    if (G32(0x1017b920) == 0)
        return -1;
    return ((W28AskFn)(uintptr_t)U32(W28_DRIVER(), 0x20))();
}

static void W28_ShowBox(void)
{
    ((W28RectFn)(uintptr_t)U32(W28_SCREEN(), 0x1c))(W28_BOX_X(), W28_BOX_Y(), W28_BOX_X() + 0x27f,
                                                     W28_BOX_Y() + 0x1df);
}

// FUNCTION: HGSIM 0x100b82a0
void __cdecl Sim_100b82a0(int longWait)
{
    uint32_t size = 0;
    uint8_t *data, *colours;
    char drive;
    const char *name;

    Plat_CurrentDir((char *)GBYTES(0x10199340), 0x100);
    drive = *(char *)GBYTES(0x10199340);
    G32(0x1017d5dc) = 0;
    G32(0x1017d5e0) = 0;
    if (G32(0x1017d5dc) == 0 || *(char *)(uintptr_t)G32(0x1017d5dc) == 0) {
        name = G32(0x1015ec58) == 0 ? (const char *)GBYTES(0x10150a1c) : (const char *)GBYTES(0x10150a24);
        Crt_SPrintf((char *)GBYTES(0x10199340), (const char *)GBYTES(0x10150a34), (int)drive,
                    (const char *)GBYTES(0x10150a2c), name, (const char *)GBYTES(0x10150a18));
    } else {
        name = G32(0x1015ec58) == 0 ? (const char *)(uintptr_t)G32(0x1017d5dc) : (const char *)GBYTES(0x10150a48);
        Crt_SPrintf((char *)GBYTES(0x10199340), (const char *)GBYTES(0x10150a58), (int)drive,
                    (const char *)GBYTES(0x10150a50), name, (const char *)GBYTES(0x10150a44));
    }
    if (G32(0x1017d5e0) == 0 || *(char *)(uintptr_t)G32(0x1017d5e0) == 0)
        Crt_SPrintf((char *)GBYTES(0x101991c0), (const char *)GBYTES(0x10150a7c), (int)drive,
                    (const char *)GBYTES(0x10150a74), (const char *)GBYTES(0x10150a6c),
                    (const char *)GBYTES(0x10150a68));
    else
        Crt_SPrintf((char *)GBYTES(0x101991c0), (const char *)GBYTES(0x10150a98), (int)drive,
                    (const char *)GBYTES(0x10150a90), (const char *)(uintptr_t)G32(0x1017d5e0),
                    (const char *)GBYTES(0x10150a8c));
    Str_Copy((char *)GBYTES(0x10199340), (const char *)GBYTES(0x10150aa8));
    Str_Copy((char *)GBYTES(0x101991c0), (const char *)GBYTES(0x10150ab4));
    data = (uint8_t *)Sim_100ba7d5((char *)GBYTES(0x10199340), (char *)(uintptr_t)G32(0x1014cec0), &size);
    if (data == 0)
        return;
    G32(0x1017d5d0) = (uint32_t)(uintptr_t)Heap_AllocRaw(size);
    Mem_Move((void *)(uintptr_t)G32(0x1017d5d0), data, size);
    Sim_100bb2b0((const uint32_t *)GBYTES(0x10199340), (const char *)(uintptr_t)G32(0x1014cec0));
    Sim_100b9203((char *)GBYTES(0x10199340), (char *)(uintptr_t)G32(0x1014cec0));
    colours = (uint8_t *)Heap_AllocZero(0x300);
    if (colours == 0)
        return;
    ((W28PaletteFn)(uintptr_t)U32(W28_DRIVER(), 0x14))(0, 0x100, colours, 1);
    Mem_Move(GBYTES(0x1017d5b8), GBYTES(0x109e3e20), 0x14);
    G32(0x1017d5bc) = G32(0x109e3d14) - 1;
    G32(0x1017d5c0) = G32(0x109e3d18) - 1;
    G32(0x1017d5a0) = (uint32_t)(uintptr_t)GBYTES(0x1017d5b8);
    G32(0x1017d5ac) = G32(0x109e3d14) - 1;
    G32(0x1017d5b0) = G32(0x109e3d18) - 1;
    W28_BOX_X() = (uint32_t)((GS32(0x1017d5a4) + GS32(0x1017d5ac)) >> 1) - 0x140;
    W28_BOX_Y() = (uint32_t)((GS32(0x1017d5a8) + GS32(0x1017d5b0)) >> 1) - 0xf0;
    if (W28_BufferReady() != 0) {
        SIM_FN(W28LogFn, 0x10083495)((const char *)GBYTES(0x10150ac0));
        return;
    }
    G32(0x1017d5b8) = G32(0x109e3e20);
    G32(0x1017d5bc) = G32(0x109e3e24);
    SIM_FN(W28ClearFn, 0x100eb070)(W28_ANIM, 0xf0);
    SIM_FN(W28ShapePalFn, 0x100ed7e6)(G32(0x1017d5d0), 0, colours);
    ((W28PaletteFn)(uintptr_t)U32(W28_DRIVER(), 0x14))(0, 0x100, colours, 1);
    W28_DRAW(G32(0x1017d5d0), 0, W28_BOX_X(), W28_BOX_Y());
    ((W28FadeFn)(uintptr_t)U32(W28_DRIVER(), 0x1c))(colours, longWait != 0 ? 0x3c : 2);
    Heap_FreeRaw(colours);
    if (*(char *)GBYTES(0x101991c0) != 0) {
        data = (uint8_t *)Sim_100ba7d5((char *)GBYTES(0x101991c0), (char *)(uintptr_t)G32(0x1014cec0), &size);
        if (data == 0)
            return;
        G32(0x1017d5d4) = (uint32_t)(uintptr_t)Heap_AllocRaw(size);
        Mem_Move((void *)(uintptr_t)G32(0x1017d5d4), data, size);
        Sim_100bb2b0((const uint32_t *)GBYTES(0x101991c0), (const char *)(uintptr_t)G32(0x1014cec0));
        Sim_100b9203((char *)GBYTES(0x101991c0), (char *)(uintptr_t)G32(0x1014cec0));
        G32(0x1017d5d8) = SIM_FN(W28CountFn, 0x100ed8c3)(G32(0x1017d5d4));
        G32(0x1017d59c) = W28_BOX_X();
        G32(0x1017d598) = W28_BOX_Y();
        W28_DRAW(G32(0x1017d5d4), 0, G32(0x1017d59c), G32(0x1017d598));
    }
    W28_ShowBox();
    if (U32(W28_DRIVER(), 0) == 1) {
        if (W28_BufferReady() == 0) {
            W28_DRAW(G32(0x1017d5d0), 0, 0, 0);
            if (G32(0x1017d5d4) != 0)
                W28_DRAW(G32(0x1017d5d4), 0, G32(0x1017d59c), G32(0x1017d598));
            if (G32(0x1017b920) != 0)
                ((W28VoidFn)(uintptr_t)U32(W28_SCREEN(), 0x18))();
        }
        return;
    }
    /* deliberate change: no Windows timer thread; W28_AnimPoll ticks on the game's thread */
    G32(0x1017d5cc) = 0;
#ifdef W28A_MUTATE /* the checker's negative check only */
    G32(0x10199448) = 11;
#else
    G32(0x10199448) = 10;
#endif
    g_w28Anim = 1;
    g_w28AnimLast = Plat_TicksMs();
}

// FUNCTION: HGSIM 0x100b8834
void __stdcall Sim_100b8834(uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t e)
{
    (void)a, (void)b, (void)c, (void)d, (void)e;
    if (G32(0x1015e538) == 0 || G32(0x1017d5d0) == 0)
        return;
    if (W28_BufferReady() != 0)
        return;
    G32(0x1017d5b8) = G32(0x109e3e20);
    G32(0x1017d5bc) = G32(0x109e3e24);
    W28_DRAW(G32(0x1017d5d0), 0, W28_BOX_X(), W28_BOX_Y());
    if (G32(0x1017d5d4) != 0)
        W28_DRAW(G32(0x1017d5d4), G32(0x1017d5e4), G32(0x1017d59c), G32(0x1017d598));
    W28_ShowBox();
    if (G32(0x1017d5d8) != 0)
        G32(0x1017d5e4) = (uint32_t)((GS32(0x1017d5e4) + 1) % GS32(0x1017d5d8));
}

void __cdecl W28_AnimPoll(void)
{
    static int busy;
    uint32_t now;

    if (!g_w28Anim || busy)
        return;
    if (G32(0x1017d5d0) == 0) {            /* StopSupanim (Sim_100b8938) has freed it */
        g_w28Anim = 0;
        return;
    }
    now = Plat_TicksMs();
    if (now - g_w28AnimLast < 250)
        return;
    g_w28AnimLast = now;
    busy = 1;
    Sim_100b8834(0, 0, 0, 0, 0);
    busy = 0;
}
