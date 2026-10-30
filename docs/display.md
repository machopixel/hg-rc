# Menu screen (SDL3 display)

The hgrc.cfg word `sdldisplay` (dev.cmd option B, through the menus) shows the
menu screens through SDL3 instead of DirectDraw, and the launcher then does not
relaunch itself with the Windows "256Color" compatibility layer or run the sim
in a second process to get away from it.

## What the menu does

HGSHELL.DLL draws through one DirectDraw block (`0x10177874` to `0x101794ad`):
Init, CreateSurfaces, LockBuffer, Flip, BlitFlip, Blit, StretchBlit, BlitRect,
Fill and the palette writes and fades. The picture is 640x480 in 256 colours
(16-bit in an alternative mode; 320x200 exists), double buffered: the game
locks an off-screen buffer, draws, copies it into the back buffer and flips.
Palette values are 6-bit and shifted left by two before `SetEntries`.
Smacker videos (9 calls to SMACKW32.DLL, `0x100314b3` to `0x10031f1f`) draw into
the same locked buffer.

## What replaces it

- `hooks_ddraw.c` points the `DirectDrawCreate` and `DirectDrawEnumerateA`
  entries of HGSHELL's import table at stand-ins (`hook_import.c`). The objects
  have the real method tables (device, surface, palette); surfaces are plain
  memory. Blt, BltFast (with source colour key), Flip chain, Lock/Unlock,
  palettes and colour fill are done in full; the rest answers "not
  implemented".
- Whenever the front surface changes (Unlock, Blt, Flip, palette change) it is
  turned into a 32-bit picture and given to `platform/display.h`; `display_sdl3.c`
  wraps the game window, makes an SDL renderer and shows it, scaled with the
  proportions kept.
- `Flip` waits for 1/60 s since the last flip, as the real one waited for the
  monitor.
- The window is made borderless and covers the monitor; the picture is scaled
  up as far as fits and centred (`display_sdl3.c`, `Layout`), black bars around.
- The shell's mouse code reads the cursor in window pixels and expects picture
  pixels, with the window rectangle at 0,0 (`0x10026bca`, `0x10026a9a`,
  `0x100269b3`). HGSHELL's import table entries for `ScreenToClient`,
  `ClientToScreen` and `GetWindowRect` therefore point at stand-ins that convert
  with the same layout. The shell's range check rejects points outside the
  picture, so clicks on the black bars do nothing.
- The shell's window procedure also takes the mouse messages (`0x200` to `0x209`)
  and reads the position from lParam (`0x1002e9b4`, cases `0x200` to `0x205`).
  The launcher's window procedure converts lParam with
  `HooksDDraw_TranslateMouse` before handing the message to the shell.

## Log lines (debug.log)

`Display: SDL3 640x480 picture, renderer ...`, `DirectDraw stand-in: mode
640x480x8`, `DirectDraw stand-in closed (flips, blits, palette writes, locks)`.
