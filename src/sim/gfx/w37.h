/*
 * DLL removal step 1 (DECISIONS.md, "DLL removal step 1: graphics library, Vulkan only"): the routines of the sim's
 * 2D graphics library (the 8-bit VFX set at 0x100e84f0..0x100f9ed7) that the Vulkan build still runs, in C.
 * The callers keep calling them at their original addresses (SIM_FN), where conv_w37.inc hooks them, as round 30
 * did for the same library's 16-bit routines.
 */
#ifndef HG_SIM_GFX_W37_H
#define HG_SIM_GFX_W37_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* ---- w37_info.c: record fields, the colour table 0x10156dcc, GIF headers ---- */
/* A font's height (font +8). */
int32_t __cdecl Sim_100ecce0(const uint8_t *font);
/* A shape table's shape count (table +4). */
int32_t __cdecl Sim_100ed8c3(const uint8_t *table);
/* A glyph's width: the first dword of glyph `code` (font +0x10 + 4 * code holds its offset from the font). */
int32_t __cdecl Sim_100eccf3(const uint8_t *font, int32_t code);
/* The first dword of shape `number` (table +8 + 8 * number holds its offset from the table). */
uint32_t __cdecl Sim_100ed743(const uint8_t *table, int32_t number);
/* Shape `number`'s origin: x (+8) << 16 | y (+0xc) & 0xffff. Two identical copies in the DLL. */
uint32_t __cdecl Sim_100f1f86(const uint8_t *table, int32_t number);
uint32_t __cdecl Sim_100ed7bc(const uint8_t *table, int32_t number);
/* Shape `number`'s size: (x1 - x0 + 1) << 16 | (y1 - y0 + 1) & 0xffff (bounds at +8..+0x14). Two copies. */
uint32_t __cdecl Sim_100f1f52(const uint8_t *table, int32_t number);
uint32_t __cdecl Sim_100ed788(const uint8_t *table, int32_t number);
/* Shape `number`'s palette (its dword at +0xc: offset of {count, then count x {index, r, g, b}}) into the 3-byte
 * palette `rgb`; nothing without one. A count of 0 runs 2^32 times, as in the original. */
void __cdecl Sim_100ed7e6(const uint8_t *table, int32_t number, uint8_t *rgb);
/* The colour table 0x10156dcc: entry `index` (the result keeps the index's top 16 bits, as the original's ax). */
uint32_t __cdecl Sim_100f01ea(int32_t index);
/* Sets entry `index` to the low 16 bits of `colour`; returns the index (the original's eax). */
int32_t __cdecl Sim_100f01cc(int32_t index, uint32_t colour);
/* The whole table (0x200 bytes) from `table`. */
void __cdecl Sim_100f01ad(const uint32_t *table);
/* The code block the library modifies at run time: start (0x100f4fd4) and code segment through the pointers,
 * returns its length 0x4ec6. */
uint32_t __cdecl Sim_100f9eb7(uint32_t *start, uint32_t *segment);
/* A GIF file's first image size: width (descriptor +5) << 16 | height (+7), after the global colour table. */
uint32_t __cdecl Sim_100ed70b(const uint8_t *gif);
/* A GIF file's palette as 6-bit components: the global table, then the first image's local table over it. */
void __cdecl Sim_100ed6aa(const uint8_t *gif, uint8_t *rgb);

/* ---- w37_text.c: 8-bit text ---- */
/*
 * Character draw (pane, x, y, font, code, lookaside): the glyph (font +0x10 + 4 * code: width, then width x height
 * bytes; height at font +8) at pane-relative (x, y), cut to the pane and its window {buffer, pitch - 1, height - 1};
 * without a lookaside the bytes are copied, with one each byte goes through it (0xff transparent). Returns the
 * glyph's width (-1 for an empty window, -2 for an empty pane).
 */
int32_t __cdecl Sim_100ecd13(const uint8_t *pane, int32_t x, int32_t y, const uint8_t *font, int32_t code,
                             const uint8_t *lookaside);
/* String draw: the characters one after another (the first even for an empty text); the last one's result. */
int32_t __cdecl Sim_100ecea6(const uint8_t *pane, int32_t x, int32_t y, const uint8_t *font, const uint8_t *text,
                             const uint8_t *lookaside);

/* ---- w37_gif.c: GIF pictures ---- */
/*
 * Row copy (pane, y, line, count): `count` bytes of `line` to the pane's row y from its left edge, cut to the pane.
 * -1 / -2 as the character draw; else the original's EAX: the destination, or the negative clip amount that ended it.
 */
uint32_t __cdecl Sim_100ecedd(const uint8_t *pane, int32_t y, const uint8_t *line, uint32_t count);
/*
 * GIF decoder (pane, file, state record of 0x502e bytes): the first image's LZW data, row by row (interlaced rows
 * in the four passes) through the row copy into the pane, through the 1-row buffer 0x10153b95; the pane is kept
 * in [0x10154ea8]. Returns the file's background colour (byte +0xb).
 */
uint32_t __cdecl Sim_100ed491(const uint8_t *pane, const uint8_t *gif, uint8_t *st);

/* ---- w37_draw.c: pixels, lines, fills ---- */
/* Pixel set (pane, x, y, colour) at pane-relative (x, y): the old byte; -1 / -2 as the character draw, -3 outside
   the pane. */
int32_t __cdecl Sim_100e84f0(const uint8_t *pane, int32_t x, int32_t y, uint32_t colour);
/* Pixel read (pane, x, y): the byte; -1 / -2 / -3 as the pixel set. */
int32_t __cdecl Sim_100e85cb(const uint8_t *pane, int32_t x, int32_t y);
/* Pane fill (pane, colour): the pane cut to its window; 0 (-1 / -2 as the character draw). */
int32_t __cdecl Sim_100eb070(const uint8_t *pane, uint32_t colour);
/*
 * Shape draw without clipping (pane, shape, x, y, slot): the shape's run-length rows (bounds at +8..+0x14, rows
 * from +0x18) at pane-relative (x, y). The original keeps the window's pitch in the fifth argument's stack slot.
 * Returns 0.
 */
int32_t __cdecl Sim_100e95e0(const uint8_t *pane, const uint8_t *shape, int32_t x, int32_t y, int32_t slot);
/*
 * Shape draw (pane, shape table, number, x, y): shape number of the table (offset at +8 + 8 * number) cut to the
 * pane: -4 for empty bounds, -3 wholly outside, the unclipped routine when wholly inside, else rows above the pane
 * skipped and each row cut on the left and / or right. -1 / -2 as the character draw, else 0.
 */
int32_t __cdecl Sim_100e91dc(const uint8_t *pane, const uint8_t *table, int32_t number, int32_t x, int32_t y);
/* The two above with every colour through the 256-byte remap table 0x10154eac. */
int32_t __cdecl Sim_100e9b9e(const uint8_t *pane, const uint8_t *shape, int32_t x, int32_t y, int32_t slot);
int32_t __cdecl Sim_100e96c6(const uint8_t *pane, const uint8_t *table, int32_t number, int32_t x, int32_t y);
/*
 * Checkerboard fill (pane, x0, y0, x1, y1, colour): every other cell of the rectangle (pane-relative, cut to the
 * pane), the rows shifted by one in turn from the bottom row up. -4 for an empty rectangle, else 0 (-1 / -2 as the
 * character draw). A shifted row of width 1 sets the cell right of the rectangle (as the original).
 */
int32_t __cdecl Sim_100e90a3(const uint8_t *pane, int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint32_t colour);

/* ---- w37_line.c: lines ---- */
/*
 * Line draw (pane, x0, y0, x1, y1, mode, colour): pane-relative ends cut to the pane by outcodes with 0.32 fixed-point
 * rounding (as the 16-bit routine 0x100ef727), cells by x- / y-major, diagonal or straight stepping; mode 0 sets the
 * colour byte, mode 1 maps each cell through the 256-byte table `colour` points to; modes above 1 (a callback, no
 * caller) draw nothing. Returns 2 for a line wholly outside, else 1 when it was cut, 0 when not (straight lines: 0,
 * the original an unset flag). -1 / -2 as the character draw.
 */
int32_t __cdecl Sim_100e86a1(const uint8_t *pane, int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t mode,
                             uint32_t color);

/*
 * Ellipse outline / filled ellipse (pane, x, y, rx, ry, colour): midpoint stepping about pane-relative (x, y), each
 * cell (outline) or row (filled) cut to the pane; a radius of 0 draws the line (x - rx, y - ry) .. (x + rx, y + ry)
 * and returns its result. -1 / -2 as the character draw, else the original's last EAX.
 */
int32_t __cdecl Sim_100eb6e8(const uint8_t *pane, int32_t x, int32_t y, int32_t rx, int32_t ry, uint32_t colour);
int32_t __cdecl Sim_100eba29(const uint8_t *pane, int32_t x, int32_t y, int32_t rx, int32_t ry, uint32_t colour);

/* ---- w37_rotate.c: the rotated / scaled shape draw ---- */
/*
 * Shape `number` of the table at pane-relative (x, y), turned by `angle` (tenths of a degree) and scaled by the
 * 16.16 factors sx / sy about its origin; flags bit 0 draws through the remap table 0x10154eac, bit 1 keeps the
 * scratch image as it is (else the shape is drawn upright into it first, 0xff transparent). Without a turn or
 * scale: the shape draw 0x100e91dc / 0x100e96c6 and its result. Else the quad is filled from the image (vertex table
 * 0x10154fac, step table 0x10154ffc); returns -1 / -2 as the character draw, else the original's last EAX.
 */
int32_t __cdecl Sim_100e9cd1(const uint8_t *pane, const uint8_t *table, int32_t number, int32_t x, int32_t y,
                             uint8_t *image, int32_t angle, int32_t sx, int32_t sy, uint32_t flags);

/* ---- w37_encode.c: the shape encoder ---- */
/*
 * Shape encoder packet (mode, back, xpos), state in the DLL's globals 0x1015365d..0x10153691: 0 starts a row, 1 puts
 * out different cells, 2 a run of equal ones, 3 keeps transparent ones pending, 4 ends the row; the cells are the
 * source up to [0x10153669] less `back`. The original keeps EAX and ECX for its caller.
 */
void __cdecl Sim_100eae93(int32_t mode, int32_t back, int32_t xpos);
/* One row of `count` cells from [0x10153665] into packets (`key` transparent). */
void __cdecl Sim_100ead0a(uint32_t count, uint32_t key, int32_t xpos);
/*
 * Encodes the pane's cells that are not `key` into a run-length shape at `out` (0: only measures): header {pane size,
 * x << 16 | y, bounding box}, then the rows of the box. Returns the size in bytes (-1 / -2 as the character draw).
 */
int32_t __cdecl Sim_100ea9f5(const uint8_t *pane, uint32_t key, int32_t x, int32_t y, uint8_t *out);

/* ---- w37_math.c: rotation ---- */
/* The two 16.16 wave values of an angle in tenths of a degree (wrapped into 0..3600) from the quarter table in the
   library's code (0x100ebd27). */
void __cdecl Sim_100ecb3b(int32_t angle, int32_t *s, int32_t *c);
/*
 * A point {x, y} scaled about `centre` by the 16.16 factors sx / sy and turned by `angle` (0x100ecb3b), rounded
 * 16.16 products: out = {x's - y'c + cx, y's + x'c + cy}. Returns the y'c term (the original's EAX).
 */
int32_t __cdecl Sim_100ecc19(const int32_t *in, int32_t *out, const int32_t *centre, int32_t angle, int32_t sx,
                             int32_t sy);

#endif
