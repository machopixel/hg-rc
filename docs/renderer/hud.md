# The HUD at full resolution

The sim draws its whole HUD (radar, gauges, message box, weapon icons, text,
crosshair) into one 640x480 RGB565 buffer. With the Vulkan renderer that buffer
used to be stretched over the picture, so the HUD was a blown-up 640x480 image.
Now the same drawing is repeated at the window's resolution.

## What the sim uses

A statically linked Miles VFX style 2D library, in an 8-bit set and a 16-bit
clone (`HWFLAG` picks). The Vulkan renderer runs the 16-bit clone
(0x100ef51c to 0x100f2f01). A window is `{buffer, pitch in bytes - 1,
height - 1}` and a pane is `{window, x0, y0, x1, y1}`. LockDisplay returns the
pitch in bytes, the sim stores it as "width - 1" and addresses rows with it; the
code clips only to the pane, so panes never go past column 639.

| Address | VFX name | Arguments (cdecl) |
| --- | --- | --- |
| 0x100ef727 | line draw | pane, x0, y0, x1, y1, mode (0), color. Returns 0 drawn, 2 off screen. |
| 0x100f0205 | shape draw, clipped | pane, shape table, number, x, y. 0 when drawn. |
| 0x100f0727 | shape draw, no clip | pane, shape, x, y, one spare slot (used as a local). |
| 0x100f0865 | pane wipe | pane, color. Fills the whole pane. |
| 0x100f1c93 | character draw | pane, x, y, font, code, lookaside. Returns the glyph's width. |
| 0x100f1e10 | string draw | calls the character draw per byte |
| 0x100f01cc / 0x100f01ea | LUT set / get | the palette index to RGB565 table at 0x10156dcc |

Colours reach every primitive already as RGB565 words (from the LUT). 0x0000
means transparent, because the overlay always treated it so.

Shape format: bounds at +8 (x0, y0, x1, y1 relative to the origin), rows from
+0x18. Row codes: `0` ends the row, `1` skips the next byte's count of cells,
an even `b` repeats the next byte `b/2` times, an odd `b` above 1 copies the
next `b/2` bytes. Every byte is a palette index looked up in the LUT. A font is
height at +8 and glyph offsets at +0x10 + 4 * code; a glyph is its width
(dword) and width x height index bytes, each passed through the caller's
256-byte lookaside (0xFF is transparent), then the LUT.

## How it works

Since round 30 the library's routines are C (`src/sim/render/w30_vfx.c`, proven exact against the originals) and
their entry points (`src/sim/render/w30_hud.c`) do the full-resolution copy themselves: when a pane draws into the
renderer's HUD buffer, the routine draws into a scratch buffer instead (so the 640x480 layer does not get it) and
the same drawing is handed to HGVK.DLL through `HgvkHudApi` (`src/plugins/hgvk/hud_api.h`). `src/hooks/hooks_hud.c`
only finds that interface and supplies the scratch and bitmap memory; it patches nothing.

- A line: the routine sets its cells on the cleared scratch buffer and those exact cells go over as a bitmap, so
  slanted lines (radar field of view) look exactly as the original.
- A wipe fills the pane's pixels; colour 0 clears them.
- A shape or glyph is decoded into RGB565 cells and scaled with nearest-cell sampling (`HUD_SMOOTH 0` in hud2d.c).
- Text goes through the string routine, which calls the character entry point per character.
- The 8-bit set's 1-bit font routine (0x100f2d3c / 0x100f2f01) writes bytes, not RGB565, and has no copy.

## Settings

- `nohudhi` in `hgrc.cfg` (`dev.cmd` option C): no full-resolution copy, back to
  the stretched 640x480 HUD (the converted routines still draw it).
- The full-resolution copy is only on with the Vulkan renderer.

## How it was checked

The converted routines are compared with the original machine code in the
checker (`tools/verify/spec_w30_vfx.py`): tens of thousands of random windows,
panes, lines, shapes and fonts, the whole 640x480 buffer exact (DECISIONS.md,
round 30). The full-resolution copy and the scaling are checked by eye with F12.

## Not done

Widescreen placement of HUD panes (left, centre, right). The 3D view is still
the 4:3 picture centred in the window, so the HUD sits inside it; moving panes to
the window edges only makes sense once the 3D view fills a widescreen window.
