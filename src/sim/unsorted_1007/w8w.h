/*
 * Round 8, letter w: functions in 0x10074cea..0x1007e7b6 (setting up and drawing text panels from resources, the
 * files mercs.hud and hgmsn.cfg, the list of saved games). Helpers that are not converted (or that another
 * round-8 list converts) are called through their original address with SIM_FN; prototypes below.
 */
#ifndef HG_SIM_UNSORTED_1007_W8W_H
#define HG_SIM_UNSORTED_1007_W8W_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* The C library sprintf (0x100fc780), fread (0x100fe830), fwrite (0x100febc0) and fclose (0x100fd760) of the DLL. */
#define W8W7_SPRINTF SIM_FN(int(__cdecl *)(char *, const char *, ...), 0x100fc780)
#define W8W7_FREAD SIM_FN(uint32_t(__cdecl *)(void *, uint32_t, uint32_t, void *), 0x100fe830)
#define W8W7_FWRITE SIM_FN(uint32_t(__cdecl *)(const void *, uint32_t, uint32_t, void *), 0x100febc0)
#define W8W7_FCLOSE SIM_FN(int(__cdecl *)(void *), 0x100fd760)
/* Resource loader: upper-cases name and kind in place and returns the loaded data or 0 (0x100ba7d5). */
#define W8W7_CALL_100ba7d5 SIM_FN(uint32_t(__cdecl *)(char *, uint32_t, uint32_t), 0x100ba7d5)
/* Not converted (0x100f0205 is patched by hooks_*.c): the two blitters of a loaded image to a surface. */
#define W8W7_CALL_100f0205 SIM_FN(void(__cdecl *)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t), 0x100f0205)
#define W8W7_CALL_100e91dc SIM_FN(void(__cdecl *)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t), 0x100e91dc)
/* Not converted: fill of a rectangle record (0x100eb070), a panel step (0x1007581a). */
#define W8W7_CALL_100eb070 SIM_FN(void(__cdecl *)(uint32_t, uint32_t), 0x100eb070)
#define W8W7_CALL_1007581a SIM_FN(void(__cdecl *)(void *), 0x1007581a)
/* Not converted: the size of an image (height << 16 | width) (0x100ed743), the height of a font (0x100ecce0). */
#define W8W7_CALL_100ed743 SIM_FN(uint32_t(__cdecl *)(uint32_t, uint32_t), 0x100ed743)
#define W8W7_CALL_100ecce0 SIM_FN(int32_t(__cdecl *)(uint32_t), 0x100ecce0)
/* In another round-8 list: text extent into a rectangle record; text drawing (0x1008de0a, 0x1008e0fe). */
#define W8W7_CALL_1008de0a SIM_FN(int32_t *(__cdecl *)(const char *, uint32_t, int32_t *), 0x1008de0a)
#define W8W7_CALL_1008e0fe SIM_FN(void(__cdecl *)(int32_t *, uint32_t, uint32_t), 0x1008e0fe)
/* Not converted: draws the text in the rectangle with the font (0x1008dba2). */
#define W8W7_CALL_1008dba2 SIM_FN(void(__cdecl *)(int32_t *, const char *, uint32_t), 0x1008dba2)
/* Converted elsewhere but not used here (round 7: its C version broke the game): the CD drive letter or 0. */
#define W8W7_CALL_10099510 SIM_FN(uint32_t(__cdecl *)(void), 0x10099510)
/* In another round-8 list: opens a data file of the game (0x100461e8). */
#define W8W7_CALL_100461e8 SIM_FN(uint32_t(__cdecl *)(const char *, uint32_t), 0x100461e8)

/*
 * Sets up the panel s (+0 frame rectangle record, +0x18 text rectangle record): the image named at s+0x10 (with
 * 'L' at index 2 when [0x101c64f0] is 0 and 'G' at index 7 with [0x1014cb38]) sizes the text rectangle
 * (Rect_ScaleAroundCenter with 16.16 factors), the steps +0x64, +0x6c, +0x74, +0x7c come from s+0x54, code 5
 * scales both rectangles to the screen, and the points +0x58, +0x60, +0x68, +0x70, +0x78 are scaled to the frame.
 * With bit 0x10 in s+4 the text rectangle is moved to the frame's x.
 */
void __cdecl Sim_10074cea(uint8_t *s, int32_t code);

/*
 * Draws the panel s: Sim_10074fce, a fill (bit 0x20 of s+4), the image s+0x14, a frame (bit 4), 0x1007581a;
 * returns 1 when Sim_10076676(s) is 0, else 0 (0 also for a panel without its records).
 */
int32_t __cdecl Sim_10075667(uint8_t *s);

/* Sets [0x10179d20], refreshes the values 0x1096fc00.. and writes the 10 dwords to "mercs.hud". */
void __cdecl Sim_10077d50(void);

/* Reads the 10 dwords of "mercs.hud" (into 0x1096fc40.., 0x1096fbec, 0x1096fc1c, 0x1096fc20); [0x10179d20] = found. */
void __cdecl Sim_10077e34(void);

/*
 * Writes "hgmsn.cfg": a 0x15014-byte block "MW2M", count, two dwords and a byte of the current side's record
 * (0x10473800 + [0x1013bb40] * 0x4fc3a) and an 0x54-byte entry per active member. Returns 1, or 0 when the file
 * could not be opened.
 */
int32_t __cdecl Sim_1007c89b(void);

/*
 * Lays out the two title panels of the table at 0x10149d28 (0x3c bytes each): rectangle scaled to the screen and
 * fitted to its image, mirrored to the left or right, the point +4 scaled, and the font name (0x101380a0 + 0x12 *
 * [0x101c64f0], "FONT" renamed "FNT") loaded to centre the text vertically (+8).
 */
void __cdecl Sim_1007dd90(void);

/*
 * Draws the text in the rectangle measured with font name2 (0x1008de0a) moved by (dx, dy) or centred for a
 * negative value, with the image name1 behind it when name1 is not empty.
 */
void __cdecl Sim_1007e29a(char *name1, char *name2, const char *text, int32_t dx, int32_t dy);

/*
 * Registers the saved games "keating\*.sfl" (also on the CD drive when they are not found) with
 * ProjFile_FindEntry and leaves the directory path at 0x1017aeb0.
 */
void __cdecl Sim_1007e66c(void);

/* Opens "<dir at 0x1017aeb0>\<name>.sfl" with 0x100461e8 when the saved game is registered; else returns 0. */
uint32_t __cdecl Sim_1007e7b6(const char *name);

#endif
