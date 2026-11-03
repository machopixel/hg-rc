/*
 * Detail and view-distance hooks for HGSIM.DLL: the finest model LOD at any
 * distance, every grid cell of the world drawn, a far plane that covers the
 * whole map, the graphics options locked on, and polygon corners at their
 * exact sub-pixel positions. Game logic is untouched:
 * these functions only feed the renderer.
 *
 * Research and addresses: docs/renderer/detail-and-distance.md.
 * hgrc.cfg word "nodetail" turns all of this off; "nolod" and "nosubpixel"
 * leave out just those hooks.
 */
#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hook.h"
#include "../launcher/debug.h"
#include "../sim/fx/w32.h"

/*
 * The converted functions these hooks wrap (Windows calls round, step 1): the hooks call them, not the DLL's
 * original code behind a trampoline (DECISIONS.md, "The last original code").
 */
void __cdecl Sim_100d1f90(void *camera);                       /* render/w3s_camera.c */
void __cdecl Sim_100d2306(uint8_t *cam);                        /* view/w3r_camera.c */
void *__cdecl Render_ProjectVertex(unsigned char *vertex);      /* render/w2n_project.c */
void __cdecl Sim_100988fc(uint32_t unused, uint32_t value);     /* unsorted_1009/w5c_calls.c */
void __cdecl Sim_100989a1(int unused, int value);               /* unsorted_1009/w5x_calls.c */
int __cdecl Sim_100585c4(int v);                                /* unsorted_1005/w2f_globals.c */
int __cdecl Sim_100ac603(int value);                            /* unsorted_100a/w2g_misc.c */

/* Polygon buffers. The original asks for a 1 MiB arena and 3000 sort entries. */
#define POLY_ARENA_KIB 16384
#define POLY_SORT_ENTRIES 32000
#define POLY_SOFT_LIMIT (POLY_SORT_ENTRIES - 100)

/* The world grid is 9x9 cells of 55562 units, so this covers all of it. */
#define VIEW_FAR 1000000.0f

/*
 * Level of detail. Static models keep their LOD nodes sorted by switch
 * distance, so the first node is the finest; a huge camera LOD scale makes
 * RenderObject pick it at any distance. Gears pick their level (0 finest .. 3)
 * in UpdateGearLods, which gives level 0 only to the nearest gear and turns
 * the same scale into int distances (an overflow at 1e20, so every gear got
 * level 3); that function is replaced below to give every gear level 0.
 */
#define LOD_SCALE_FINEST 1e20f

/* Data in HGSIM.DLL, as RVAs (see the research doc for each). */
#define RVA_MAIN_CAMERA 0x14CA00   /* static camera struct; +0x40 far plane */
#define RVA_CHILD_RANGE 0x185B60   /* (far - near) / 2: sub-objects drawn within it */
#define RVA_POLY_SOFT_LIMIT 0x17D140
#define RVA_GRID_ON 0x17C8C4
#define RVA_NO_GRID_LIST 0x17C8CC
#define RVA_ONE_GRID 0x17C8E8
#define RVA_CELLS 0x17BA28         /* [9][9] cells, row stride 0x18C, cell 0x2C */
#define RVA_CELL_LIST 0x36F2A0     /* NULL-terminated list, 82 entries at most */
#define RVA_CAM_ROW 0x36F3F8
#define RVA_CAM_COL 0x36F400
#define RVA_CULL_FN 0x14CB40       /* int (*)(void *volume): 0 = visible */
#define RVA_ALL_CELLS 0x87C86
#define RVA_PROJECT_SCALE 0x12BA78 /* float 1.0: screen x = x * (scale / z) */
#define RVA_SCREEN_CX 0x185BD4     /* int screen centre, from SetCurrentCamera */
#define RVA_SCREEN_CY 0x185BF4
#define RVA_PLUGIN_RENDER 0x9E3D30 /* the renderer plugin's Render, filled at load */
#define RVA_GEAR_COUNT 0x15DC64    /* int number of gears in the mission */
#define RVA_GEARS 0x9E8B40         /* gear pointers; +4 id, +0x1c flags, +0x24 LOD level */
#define RVA_PLAYER_GEAR 0x17B8C8   /* int index of the player's gear */
#define RVA_SET_GEAR_LOD 0x3AD51   /* int SetGearLod(int id, int level): 0 = failed */

#define GRID_CELLS 9
#define GRID_CELL_SIZE 55562.0f
#define GRID_ORIGIN_X 254008.0f
#define GRID_ORIGIN_Z 246044.5f
#define CELL_LIST_MAX 82

static unsigned char *g_sim;

static void WrapPluginRender(void);

#define SIM(rva) (g_sim + (rva))

typedef struct SortEntry {
    void *record;
    float depth;
} SortEntry;

static int ConfigHasWord(const char *word)
{
    char buf[32];
    FILE *fp = fopen("hgrc.cfg", "r");
    int found = 0;

    if (fp == NULL)
        return 0;
    while (!found && fscanf(fp, "%31s", buf) == 1)
        found = _stricmp(buf, word) == 0;
    fclose(fp);
    return found;
}

/*
 * AllocPolyBuffers(arenaKiB, sortEntries): called once by RenderInit with
 * (1024, 3000). Larger buffers so a whole-map view does not drop polygons.
 */
int __cdecl Sim_100a8816(int arenaKiB, int sortEntries);   /* unsorted_100a/w5x_calls.c */

// FUNCTION: HGSIM 0x100a8816
static int __cdecl HookedAllocPolyBuffers(int arenaKiB, int sortEntries)
{
    if (arenaKiB < POLY_ARENA_KIB)
        arenaKiB = POLY_ARENA_KIB;
    if (sortEntries < POLY_SORT_ENTRIES)
        sortEntries = POLY_SORT_ENTRIES;
    return Sim_100a8816(arenaKiB, sortEntries);
}

/*
 * SortByDepth(first, last): quicksort of the inclusive range, farthest first.
 * The same partitioning as the original, so the result (including the order
 * of equal depths) is identical, but it recurses only into the smaller half,
 * so the stack stays small with the bigger buffers. Like the original it may
 * read the depth one entry past `last`; the sort array is followed by spare
 * space.
 */
// FUNCTION: HGSIM 0x100a4700
static void __cdecl SortByDepth(SortEntry *first, SortEntry *last)
{
    while (last > first) {
        SortEntry *i = first, *j = last + 1, t;
        float pivot = first->depth;

        for (;;) {
            do {
                i++;
            } while (i->depth > pivot && i <= last);
            do {
                j--;
            } while (!(j->depth >= pivot) && j >= first);
            if (i >= j)
                break;
            t = *i;
            *i = *j;
            *j = t;
        }
        t = *first;
        *first = *j;
        *j = t;

        if (j - first < last - j) {
            if (first < j - 1)
                SortByDepth(first, j - 1);
            first = j + 1;
        } else {
            if (j + 1 < last)
                SortByDepth(j + 1, last);
            last = j - 1;
        }
    }
}

/*
 * VisibleCells(camera): the list of grid cells the object pass walks. The
 * original takes the camera's cell and the 5 or 8 cells in front of it, which
 * is what limits the view to about 55k-111k units. This lists every cell,
 * nearest ring first, so if the polygon buffers ever fill it is the far
 * scenery that is dropped. The fall-backs are the original's.
 */
typedef int(__cdecl *CullFn)(void *volume);

// FUNCTION: HGSIM 0x10087d83
static void __cdecl HookedVisibleCells(const float *camera)
{
    void **list = (void **)SIM(RVA_CELL_LIST);
    CullFn cull = *(CullFn *)SIM(RVA_CULL_FN);
    int count = 0, row, col, ring;

    if (*(int *)SIM(RVA_GRID_ON) == 0 || camera == NULL) {
        list[0] = *(void **)SIM(RVA_NO_GRID_LIST);
        list[1] = NULL;
        return;
    }
    /* As the original: truncate, add 0.5, truncate again. */
    row = (int)((float)(int)((camera[0] + GRID_ORIGIN_X) / GRID_CELL_SIZE) + 0.5f);
    col = (int)((float)(int)((camera[2] + GRID_ORIGIN_Z) / GRID_CELL_SIZE) + 0.5f);
    *(int *)SIM(RVA_CAM_ROW) = row;
    *(int *)SIM(RVA_CAM_COL) = col;
    if (row < 0 || row >= GRID_CELLS || col < 0 || col >= GRID_CELLS) {
        ((void(__cdecl *)(void))SIM(RVA_ALL_CELLS))();
        return;
    }

    for (ring = 0; ring < GRID_CELLS; ring++) {
        int r, c;

        if (ring > 0 && *(int *)SIM(RVA_ONE_GRID) != 0)
            break;
        for (r = row - ring; r <= row + ring; r++) {
            for (c = col - ring; c <= col + ring; c++) {
                unsigned char *cell;
                void *entry, *volume;

                if (r != row - ring && r != row + ring && c != col - ring && c != col + ring)
                    continue; /* inside the ring: listed already */
                if (r < 0 || r >= GRID_CELLS || c < 0 || c >= GRID_CELLS)
                    continue;
                cell = SIM(RVA_CELLS) + r * 0x18C + c * 0x2C;
                entry = *(void **)(cell + 0x24);
                if (entry == NULL || *(void **)((unsigned char *)entry + 8) == NULL)
                    continue; /* empty cell */
                volume = *(void **)(cell + 0x28);
                /* The camera's own cell is always listed, as in the original. */
                if (ring > 0 && volume != NULL && cull(volume) != 0)
                    continue;
                if (count < CELL_LIST_MAX)
                    list[count++] = entry;
            }
        }
    }
    list[count] = NULL;
}

/*
 * SetCurrentCamera(camera): copies the camera into the globals the renderer
 * uses. The main camera gets a far plane past the whole map; sub-objects are
 * drawn at any distance; the polygon soft limit follows the bigger buffers
 * (RenderInit writes 2900 there after allocating them).
 */

// FUNCTION: HGSIM 0x100d1f90
static void __cdecl HookedSetCurrentCamera(void *camera)
{
    if (camera == SIM(RVA_MAIN_CAMERA)) {
        float *farPlane = (float *)((unsigned char *)camera + 0x40);

        if (*farPlane < VIEW_FAR)
            *farPlane = VIEW_FAR;
    }
    Sim_100d1f90(camera);
    WrapPluginRender();
    *(float *)SIM(RVA_CHILD_RANGE) = FLT_MAX;
    *(int *)SIM(RVA_POLY_SOFT_LIMIT) = POLY_SOFT_LIMIT;
}

/*
 * Sub-pixel vertex positions. ProjectVertex stores each corner's screen
 * position as whole pixels (it truncates x * (1/z)), which made sense for the
 * 1997 software rasteriser but makes polygons wobble once the 640x480 picture
 * is scaled up. The exact position is remembered here, keyed by what the game
 * later hands the plugin for that corner (the whole-pixel x and y and the
 * camera depth z), and a wrapper around the plugin's Render puts it back.
 * A corner the table does not know (clipped edges, the 2D overlay) keeps its
 * whole-pixel position, as before.
 */
#define SUBPIXEL_SLOTS (1u << 18)

typedef struct SubpixelSlot {
    int ix, iy;
    DWORD zBits;
    float fx, fy;
} SubpixelSlot;

static SubpixelSlot *g_subpixel;

static unsigned SubpixelIndex(int ix, int iy, DWORD zBits)
{
    unsigned h = (unsigned)ix * 73856093u ^ (unsigned)iy * 19349663u ^ zBits * 83492791u;

    return (h ^ (h >> 15)) & (SUBPIXEL_SLOTS - 1);
}


// FUNCTION: HGSIM 0x100a62c4
static void *__cdecl HookedProjectVertex(unsigned char *vertex)
{
    int done = vertex[0x29] & 1; /* projected already this frame */
    void *result = Render_ProjectVertex(vertex);

    if (!done && g_subpixel != NULL) {
        const float *p = (const float *)vertex;
        int ix = *(const int *)(vertex + 0x18), iy = *(const int *)(vertex + 0x1C);
        float s = *(const float *)SIM(RVA_PROJECT_SCALE) / p[2];
        float fx = p[0] * s + (float)*(const int *)SIM(RVA_SCREEN_CX);
        float fy = (float)*(const int *)SIM(RVA_SCREEN_CY) - p[1] * s;

        /* Only within a pixel of the game's own result, in case of surprises. */
        if (fx - (float)ix < 1.01f && (float)ix - fx < 1.01f && fy - (float)iy < 1.01f &&
            (float)iy - fy < 1.01f) {
            SubpixelSlot *slot;
            DWORD zBits;

            memcpy(&zBits, &p[2], sizeof(zBits));
            slot = &g_subpixel[SubpixelIndex(ix, iy, zBits)];
            slot->ix = ix;
            slot->iy = iy;
            slot->zBits = zBits;
            slot->fx = fx;
            slot->fy = fy;
        }
    }
    return result;
}

/* The plugin's Render(op, ctx, vertices, count, texture); vertices are 36 bytes. */
typedef int(__cdecl *PluginRenderFn)(int op, void *ctx, unsigned char *verts, int count,
                                     void *tex);
static PluginRenderFn g_pluginRender;

static int __cdecl SubpixelRender(int op, void *ctx, unsigned char *verts, int count, void *tex)
{
    int i;

    if (verts != NULL && g_subpixel != NULL) {
        for (i = 0; i < count; i++) {
            float *v = (float *)(verts + (size_t)i * 36);
            int ix = (int)v[0], iy = (int)v[1];
            DWORD zBits;
            const SubpixelSlot *slot;

            if ((float)ix != v[0] || (float)iy != v[1])
                continue; /* not a whole-pixel corner */
            memcpy(&zBits, &v[2], sizeof(zBits));
            slot = &g_subpixel[SubpixelIndex(ix, iy, zBits)];
            if (slot->ix == ix && slot->iy == iy && slot->zBits == zBits) {
                v[0] = slot->fx;
                v[1] = slot->fy;
            }
        }
    }
    return g_pluginRender(op, ctx, verts, count, tex);
}

/* The plugin is loaded after the hooks go in, so wrap its Render once it is there. */
static void WrapPluginRender(void)
{
    PluginRenderFn *slot = (PluginRenderFn *)SIM(RVA_PLUGIN_RENDER);

    if (*slot == NULL || *slot == SubpixelRender)
        return;
    if (g_subpixel == NULL)
        g_subpixel = (SubpixelSlot *)calloc(SUBPIXEL_SLOTS, sizeof(SubpixelSlot));
    if (g_subpixel == NULL)
        return;
    g_pluginRender = *slot;
    *slot = SubpixelRender;
}

/*
 * CameraViewport(camera): works out the camera's projection, including the
 * LOD scale at +0xb8 that model LOD switch distances are multiplied by.
 */

// FUNCTION: HGSIM 0x100d2306
static void __cdecl HookedCameraViewport(void *camera)
{
    Sim_100d2306((uint8_t *)camera);
    *(float *)((unsigned char *)camera + 0xB8) = LOD_SCALE_FINEST;
}

/*
 * UpdateGearLods, run every frame. The original gives level 0 to the nearest
 * gear only, level 1 to the next two and to others within 13000 x LOD scale
 * (six at most), then 2 or 3 by distance. Its rules that do not depend on
 * distance stay: the player's gear always 0, and other gears with flag 2
 * always 1.
 */
typedef int(__cdecl *SetGearLodFn)(int id, int level);

// FUNCTION: HGSIM 0x1003b534
static void __cdecl UpdateGearLods(void)
{
    SetGearLodFn setGearLod = (SetGearLodFn)SIM(RVA_SET_GEAR_LOD);
    unsigned char **gears = (unsigned char **)SIM(RVA_GEARS);
    int count = *(int *)SIM(RVA_GEAR_COUNT);
    int player = *(int *)SIM(RVA_PLAYER_GEAR);
    int i;

    for (i = 0; i < count; i++) {
        unsigned char *gear = gears[i];
        int level = i != player && (*(short *)(gear + 0x1C) & 2) ? 1 : 0;

        if (*(int *)(gear + 0x24) != level && setGearLod(*(int *)(gear + 4), level))
            *(int *)(gear + 0x24) = level;
    }
}

/* Option setters, value 0 off / 1 on (-1 asks for the current value). */

/* The two texture setters take (unused, value): the value is the second argument
 * and is stored at +0x14 / +0x18 of the settings saved to HGSND.CFG. */

// FUNCTION: HGSIM 0x100988fc
static int __cdecl HookedSetTextures(int unused, int value)
{
    (void)value;
    Sim_100988fc((uint32_t)unused, 1);
    return 1;       /* the original leaves the value it was given in eax */
}

// FUNCTION: HGSIM 0x100989a1
static int __cdecl HookedSetTerrainTextures(int unused, int value)
{
    (void)value;
    Sim_100989a1(unused, 1);
    return 1;       /* the original leaves the value it was given in eax */
}

// FUNCTION: HGSIM 0x10090d15
static int __cdecl HookedSetParticles(int value)
{
    /* round 32: the converted switch (the DLL's original keeps 1.0's record size, wrong for 1.2's particle pool) */
    return Sim_10090d15(value == -1 ? -1 : 1);
}

// FUNCTION: HGSIM 0x100585c4
static int __cdecl HookedSetLights(int value)
{
    return Sim_100585c4(value == -1 ? -1 : 1);
}

// FUNCTION: HGSIM 0x100ac603
static int __cdecl HookedSetShadows(int value)
{
    return Sim_100ac603(value == -1 ? -1 : 1);
}

static const unsigned char kPrologueAllocPolyBuffers[] = {0x55, 0x8B, 0xEC, 0x51,
                                                          0x8B, 0x45, 0x0C};
static const unsigned char kPrologueSortByDepth[] = {0x55, 0x8B, 0xEC, 0x83, 0xEC, 0x20};
static const unsigned char kPrologueVisibleCells[] = {0x55, 0x8B, 0xEC, 0x83, 0xEC, 0x2C};
static const unsigned char kPrologueSetCurrentCamera[] = {0x55, 0x8B, 0xEC, 0x83, 0xEC, 0x08};
static const unsigned char kPrologueProjectVertex[] = {0x55, 0x8B, 0xEC, 0x83, 0xEC, 0x10};
static const unsigned char kPrologueCameraViewport[] = {0x55, 0x8B, 0xEC, 0x83, 0xEC, 0x48};
static const unsigned char kPrologueUpdateGearLods[] = {0x55, 0x8B, 0xEC, 0x81, 0xEC,
                                                        0x90, 0x01, 0x00, 0x00};
static const unsigned char kPrologueTextureOption[] = {0x55, 0x8B, 0xEC, 0x8B, 0x45, 0x08};
static const unsigned char kPrologueSetParticles[] = {0x55, 0x8B, 0xEC, 0x83, 0xEC, 0x08};
static const unsigned char kPrologueQueryOption[] = {0x55, 0x8B, 0xEC, 0x83, 0x7D, 0x08, 0xFF};

#define VIEW_HOOK(rva_, name_, prologue_, fn_, original_)                                   \
    {.module = "HGSIM.DLL", .rva = (rva_), .name = (name_), .expect = (prologue_),         \
     .stolen = sizeof(prologue_), .replacement = (void *)(fn_), .original = (original_)}

static const HookDesc g_viewHooks[] = {
    VIEW_HOOK(0xA8816, "AllocPolyBuffers", kPrologueAllocPolyBuffers, HookedAllocPolyBuffers, NULL),
    VIEW_HOOK(0xA4700, "SortByDepth", kPrologueSortByDepth, SortByDepth, NULL),
    VIEW_HOOK(0x87D83, "VisibleCells", kPrologueVisibleCells, HookedVisibleCells, NULL),
    VIEW_HOOK(0xD1F90, "SetCurrentCamera", kPrologueSetCurrentCamera, HookedSetCurrentCamera, NULL),
    VIEW_HOOK(0xA62C4, "ProjectVertex", kPrologueProjectVertex, HookedProjectVertex, NULL),
    VIEW_HOOK(0xD2306, "CameraViewport", kPrologueCameraViewport, HookedCameraViewport, NULL),
    VIEW_HOOK(0x3B534, "UpdateGearLods", kPrologueUpdateGearLods, UpdateGearLods, NULL),
    VIEW_HOOK(0x988FC, "SetTexturesOption", kPrologueTextureOption, HookedSetTextures, NULL),
    VIEW_HOOK(0x989A1, "SetTerrainTexturesOption", kPrologueTextureOption, HookedSetTerrainTextures, NULL),
    VIEW_HOOK(0x90D15, "SetParticles", kPrologueSetParticles, HookedSetParticles, NULL),
    VIEW_HOOK(0x585C4, "SetMultipleLights", kPrologueQueryOption, HookedSetLights, NULL),
    VIEW_HOOK(0xAC603, "SetShadows", kPrologueQueryOption, HookedSetShadows, NULL),
};

void HooksView_OnSimLoaded(HMODULE sim)
{
    int count;
    size_t i;

    if (ConfigHasWord("nodetail")) {
        DebugPrintf("HGSIM.DLL: detail and view-distance hooks off (nodetail)\n");
        return;
    }
    /* the hooks run converted functions now (no trampolines to the original): left out of the untouched run */
    if (ConfigHasWord("noconv")) {
        DebugPrintf("HGSIM.DLL: detail and view-distance hooks off (noconv)\n");
        return;
    }
    g_sim = (unsigned char *)sim;
    count = 0;
    for (i = 0; i < sizeof(g_viewHooks) / sizeof(g_viewHooks[0]); i++) {
        const HookDesc *hook = &g_viewHooks[i];

        /* Words in hgrc.cfg leave single hooks out, to compare. */
        if (((hook->rva == 0xD2306 || hook->rva == 0x3B534) && ConfigHasWord("nolod")) ||
            (hook->rva == 0xA62C4 && ConfigHasWord("nosubpixel"))) {
            DebugPrintf("HGSIM.DLL: %s left out by hgrc.cfg\n", hook->name);
            continue;
        }
        count += Hook_InstallModule(sim, "HGSIM.DLL", hook, 1);
    }
    DebugPrintf("HGSIM.DLL: %d detail and view-distance hook(s) installed\n", count);
}
