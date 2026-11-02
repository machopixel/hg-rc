/*
 * Round 8, letter u: functions in 0x10031ce0..0x1003ad51 (box test, debug output, plugin scans, display mode,
 * squad records). Helpers that are not converted (or that another round-8 list converts, or that a hook file
 * patches) are called through their original address with SIM_FN; prototypes below.
 * Plugin scans use the C library's _finddata_t (0x118 bytes, name at +0x14) in the caller's frame, like the
 * original; it is only handed to the converted Crt_FindFirst / Crt_FindNext, never to Windows directly.
 */
#ifndef HG_SIM_UNSORTED_1003_W8U_H
#define HG_SIM_UNSORTED_1003_W8U_H

#include <stdint.h>
#include <stdarg.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#define __stdcall
#endif

/* The C library of the DLL: sprintf 0x100fc780, fclose 0x100fd760, _vsnprintf 0x100fe3e0. */
#define W8U_SPRINTF_1003 SIM_FN(int(__cdecl *)(char *, const char *, ...), 0x100fc780)
#define W8U_FCLOSE_1003 SIM_FN(int(__cdecl *)(void *), 0x100fd760)
#define W8U_VSNPRINTF_1003 SIM_FN(int(__cdecl *)(char *, uint32_t, const char *, va_list), 0x100fe3e0)
/* Patched by src/hooks/hooks_sim.c (TapLoadPlugin): (path). */
#define W8U_CALL_1003650d SIM_FN(void(__cdecl *)(const char *), 0x1003650d)
/* Not converted: (index, slot, buffer); returns 0 on failure. */
#define W8U_CALL_1003aecb SIM_FN(int(__cdecl *)(int, int, void *), 0x1003aecb)
/* Not converted: returns 0xa4 whatever it is given (called once with [0x10134f88], once without arguments). */
#define W8U_CALL_100ab1f2_ARG SIM_FN(uint32_t(__cdecl *)(uint32_t), 0x100ab1f2)
#define W8U_CALL_100ab1f2 SIM_FN(uint32_t(__cdecl *)(void), 0x100ab1f2)
/* Not converted: (object). */
#define W8U_CALL_100897c7 SIM_FN(void(__cdecl *)(uint32_t), 0x100897c7)
/* Windows through the DLL's import slots. */
typedef int(__stdcall *W8uMessageBoxFn)(uint32_t window, const char *text, const char *caption, uint32_t type);
typedef int(__stdcall *W8uGetSystemMetricsFn)(int index);
typedef int(__stdcall *W8uAdjustWindowRectFn)(void *rect, uint32_t style, int menu);
typedef int(__stdcall *W8uShowWindowFn)(uint32_t window, int show);
typedef int(__stdcall *W8uUpdateWindowFn)(uint32_t window);
/* Entries of a display driver table (0x1013a2a8 / 0x1013a298, current ones in [0x1015e68c] / [0x1015e688]):
 * +0x10 set mode (a, width, height), +0x14 / +0x18 no arguments; of [0x1015e688]: +0x14 (first, count, palette,
 * 1), +0x1c (palette, 0x3c), +0x20 no arguments. */
typedef int(__cdecl *W8uDriverModeFn)(uint32_t a, int width, int height);
typedef void(__cdecl *W8uDriverCloseFn)(void);
typedef void(__cdecl *W8uDriverPaletteFn)(int first, int count, void *palette, int flag);
typedef int(__cdecl *W8uDriverQueryFn)(void);
typedef void(__cdecl *W8uDriverFadeFn)(void *palette, int steps);
/* Not converted: (rectangle, image file, 0x502e-byte buffer) and (image file, 0x300-byte palette). */
#define W8U_CALL_100ed491 SIM_FN(void(__cdecl *)(int32_t *, void *, void *), 0x100ed491)
#define W8U_CALL_100ed6aa SIM_FN(void(__cdecl *)(void *, void *), 0x100ed6aa)

/*
 * (box, data, segment): the segment's slab overlaps on the three axes (Sim_1009cfd4 against box +0/+4, +8/+0xc,
 * +0x10/+0x14), clipped to [0, length]; when they meet, the faces: Sim_10031ed3(box, data, segment) when the
 * face count +0x18 is 0, else Sim_1002e790(face, data + 0x18, segment) for each face pointer at +0x2c.
 * 1 on a hit, else 0.
 */
int __cdecl Sim_10031ce0(void *box, uint8_t *data, void *seg);

/* Appends text to "debug.log" (opened "at" into [0x1015e500] when not open yet), flushes and closes it. */
void __cdecl Debug_AppendLog(const char *text);

/* Formats into 0x100 bytes (_vsnprintf) and sends it by [0x1015e4f8]: 1 mono screen (Sim_10034777),
 * 2 OutputDebugString, 3 message box "DEBUG Message" on window [0x1015e4fc] when set, 4 Debug_AppendLog. */
void __cdecl Debug_Printf(const char *format, ...);

/*
 * Loads "<[0x1015e530] or sbannr><language letter at 0x1014d1b0 + 2 * [0x101c64f0]>.gif" whole and, with a
 * 0x502e-byte image and a 0x300-byte palette buffer, shows it through the current driver ([0x1015e688] +0x14,
 * +0x20, +0x1c; [0x1015e68c] +0x18) with 0x100ed491 / 0x100ed6aa in the rectangle 0x109e3e00 (Sim_1008d36c).
 */
void __cdecl Sim_100355ad(void);

/* Plugin_ProbeRaster(name, arg) for every "plugin\*.dll"; returns [0x1015e56c] (cleared first). */
uint32_t __cdecl Plugin_ProbeAll(uint32_t arg);

/* Sim_100361d8("plugin\<name>", list) for every "plugin\*.dll"; returns [0x1015e574] (cleared first). */
uint32_t __cdecl Sim_10036108(uint8_t *list);

/* Loads every "plugin\*.dll" with 0x1003650d (the first one by its bare name); returns 0x1015e578, or 0 when
 * none is found. */
uint32_t __cdecl Plugin_LoadAll(void);

/*
 * (mode, a, b, width, height, menu): picks the display driver (mode -1 or 0..6), sizes the window rectangle
 * 0x109e3780 (full screen, or a centred window through AdjustWindowRect), switches the driver when it or the size
 * changed, and asks the drivers for the mode (falling back along their chain while [0x1015e694] is set).
 * 1 on success, 0 otherwise.
 */
int __cdecl Display_SetMode(int mode, uint32_t a, uint32_t b, int width, int height, int menu);

/* (record, key, skip): attaches the part found by Sim_10038dd3 to the unit's "RAHN" node; 1 on success. */
int __cdecl Sim_10038913(uint8_t *rec, int32_t key, int skip);

/* Allocates 0xa4 bytes (the size 0x100ab1f2 gives) for each 0x7e-byte entry at 0x109b9e20 that belongs to the
 * unit +4 and fills each entry's block with 0x1003aecb(entry, slot 3..0, block); 0 when the allocation fails. */
int __cdecl Sim_1003a990(uint8_t *unit);

/* (unit index, level slot 0..3; -1 does nothing): switches every 0x7e-byte entry of the unit to that slot
 * (0x1003aecb, or the list removals); 0 when one switch failed, else 1. src/hooks/hooks_view.c calls it as
 * SetGearLod(id, level). */
int __cdecl Gear_SetLod(uint32_t unit, int slot);

#endif
