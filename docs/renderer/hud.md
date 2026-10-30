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

`src/hooks/hooks_hud.c` wraps the five drawing functions in the table above.
A wrapper checks that the pane draws into the buffer LockDisplay handed out,
points the window at a scratch buffer, lets the original run (so every result
and side effect is the game's own), puts the window back, and then, when the
original reports it drew, hands the same drawing to HGVK.DLL through
`HgvkHudApi` (`src/plugins/hgvk/hud_api.h`).

`src/plugins/hgvk/hud2d.c` keeps a premultiplied RGBA picture the size of the
4:3 area of the window (`Hud_Frame`, once per frame) and draws into it:

- A line: the hook lets the game's own line routine set its cells on a scratch
  buffer and sends those exact cells as a bitmap, so slanted lines (radar field
  of view) look exactly as in the original. Colour 0 draws nothing.
- A wipe fills the pane's pixels; colour 0 clears them.
- A shape or glyph is decoded (hooks_hud.c) into RGB565 cells and scaled with
  nearest-cell sampling (`HUD_SMOOTH 0` in hud2d.c), so every cell stays a hard-edged
  block like the old HUD and the menus. The game's own images are used as they are; the
  plan is to replace them later.

`vk.c` uploads the picture (`EnsureHud2`) and draws it 1:1 over the picture
after the old overlay. Both overlays blend as premultiplied colour.

Anything not wrapped still draws into the real 640x480 buffer, which is still
shown under the new picture, so nothing the sim draws can go missing. Not
wrapped: pixel write (0x100ef564), ellipse (0x100f094e), row copy
(0x100f1e47), the two-byte text path (0x100f2d3c, 8-bit only) and anything
that writes the buffer directly. F12 in a mission saves both pictures:
`hgvk-hud.bmp` is what is left in the old buffer, `hgvk-hud2.bmp` is the full
resolution picture, and `hgvk.log` counts what was drawn. Whatever shows up in
`hgvk-hud.bmp` is the next thing to wrap.

## Settings

- `nohudhi` in `hgrc.cfg` (`dev.cmd` option C): leaves the hooks out, back to
  the stretched 640x480 HUD.
- The hooks are only installed with the Vulkan renderer.

## How it was checked

The shape, clipped shape and glyph decoders were compared against the original
machine code run in an emulator (Unicorn): 220 random shapes, clip rectangles
and fonts, every cell the original wrote matched what the hooks hand over. Lines
and the scaling are checked by eye with F12.

## Not done

Widescreen placement of HUD panes (left, centre, right). The 3D view is still
the 4:3 picture centred in the window, so the HUD sits inside it; moving panes to
the window edges only makes sense once the 3D view fills a widescreen window.
