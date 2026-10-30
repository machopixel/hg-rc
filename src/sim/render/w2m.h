/*
 * Glue between HGSIM.DLL and the renderer plugin (wave 3A). The plugin's
 * exports are stored as function pointers in a table in the DLL's data
 * (0x109e3d24..0x109e3d64, one dword each); the converted code calls them
 * through that table exactly like the original, so a replacement plugin keeps
 * working. Slots used here:
 *   0x109e3d24 SetLumaTable      0x109e3d48 LockDisplay (out: dword, returns pointer)
 *   0x109e3d4c UnlockDisplay     0x109e3d50 RefreshDisplay
 *   0x109e3d54 PreloadTexture(texture **, key)
 * (names of the exports: docs/renderer/plugin-api.md).
 */
#ifndef HG_SIM_RENDER_W2M_H
#define HG_SIM_RENDER_W2M_H

#include <stddef.h>
#include <stdint.h>

#include "../simmem.h"

/* A slot of the plugin table read as a function of the given type. */
#define RENDER_SLOT(type, addr) ((type)(*SIM_PTR(void *, addr)))
/* The cache loader (thunk 0x10003413, not converted): (file name, value of a global, 0) -> data or NULL. */
#define RENDER_LOAD(name, arg2) SIM_FN(void *(__cdecl *)(void *, void *, int), 0x10003413)((name), (arg2), 0)

/* Texture description handed to PreloadTexture: size, 8-bit pixels, 256-entry palette, key index. */
typedef struct RenderTexture {
    int32_t width;
    int32_t height;
    void *pixels;
    void *palette;
    int32_t key;
} RenderTexture;

/*
 * Copies the 32x256 byte table of the cached file named by [0x1014cf0c] (loaded
 * on first use into [0x1015cff0]) to 0x10ab1ae0 with the 32 rows in reverse
 * order, then passes it to the plugin's SetLumaTable.
 */
void __cdecl Render_LoadLumaTable(void);

/*
 * Offers the textures of the table at 0x10ac1b00 (0x200 records of 14 bytes,
 * word 0 = texture-name group, word 8 bit 0 = in use) to PreloadTexture. Two
 * passes: names not starting with "WX" first, then those starting with "WX";
 * only textures whose width equals a size that halves after every group of
 * records (0x100, 0x80, ..) are offered, the size is not reset between passes.
 */
void __cdecl Render_PreloadTextures(void);

/*
 * Loads the files named at 0x1017d0d0 and 0x1017d0b0 (once, handles kept in
 * [0x1017d0fc] and [0x1017d100]; file = word width, word height, pixels) and
 * calls PreloadTexture for each file that is loaded.
 */
void __cdecl Render_PreloadFileTextures(void);

/*
 * Methods of the plugin-owned ("class 3") display driver, table 0x1014b838.
 * The 16-bit pixels are reached through LockDisplay; [0x1017b984] says the
 * display is locked.
 */
/* Unlocks the display if it is locked, then refreshes it; returns 0. */
int __cdecl PluginDisp_UnlockRefresh(void);
/* Methods that only refresh the display; return 0. */
int __cdecl PluginDisp_Refresh_100877df(void);
int __cdecl PluginDisp_Refresh_100877ec(void);
int __cdecl PluginDisp_Refresh_100877f9(void);
int __cdecl PluginDisp_Refresh_10087806(void);
int __cdecl PluginDisp_Refresh_10087813(void);
/*
 * Fills a rectangle of 16-bit pixels with `color` (x0, y0, x1, y1 inclusive);
 * returns 0. Quirk kept: the start of the first row is the locked pointer plus
 * stride*height plus width in BYTES, not the position of (x0, y0).
 */
int __cdecl PluginDisp_FillRect(int x0, int y0, int x1, int y1, int color);
/*
 * Marks the display locked, locks it and stores the pixel pointer in the first
 * dword and (LockDisplay's out value - 1) in the second dword of the record
 * that [0x1015e698] points to; returns 0.
 */
int __cdecl PluginDisp_Lock(void);

#endif
