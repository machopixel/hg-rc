# Hook layer

The launcher replaces functions inside the original DLLs while the game runs,
so the decompiled C can take over one function at a time and the game stays
playable. The engine is `src/hooks/hook.c`; the list of replaced functions is
`src/hooks/hooks_sim.c`.

## How a hook works

1. The launcher loads `HGSHELL.DLL` or `HGSIM.DLL` and calls
   `Hooks_OnModuleLoaded` before running it. Both DLLs are loaded again on
   every switch between menu and mission, so hooks are installed each time.
2. Each hook names a module, an RVA (offset from the module base, so it works
   when Windows moves the DLL) and the original bytes at that spot.
3. If the bytes match, the first 5 bytes of the function become `jmp` to the
   replacement and the rest of the listed prologue is filled with `nop`.
   If they do not match, the hook is skipped, so another build of the game is
   never patched blindly.
4. If the hook asks for it, a trampoline is built: a copy of the overwritten
   bytes plus a jump back, so the replacement can still call the original.
5. A *tap* is a hook that only watches. It gets a pointer to the call's
   arguments, then the original runs unchanged. Taps are how we find out what
   the 1997 code is doing before replacing it.

The listed prologue must contain no relative jumps or calls (they would move
with the copy). Pick a length of 5 or more that ends on an instruction
boundary, usually `push ebp; mov ebp,esp; sub esp,N`.

## Adding a hook

1. Find the function in the disassembly. Its address at the preferred base
   (0x10000000 for the DLLs) goes in a `// FUNCTION: HGSIM 0x...` comment.
2. Write the replacement with the same calling convention and arguments.
3. Add an entry to `g_hooks` with the RVA and the original prologue bytes.

Set `HG_NO_HOOKS=1` to run the game with no hooks, and `HG_DEBUG=4` to log what
was installed to `debug.log`.

## Hooks so far

| Module | Function | RVA | What it does now |
| --- | --- | --- | --- |
| HGSIM.DLL | SystemError | 0xBC280 | Tap: writes the error code to `trace.log`. |
| HGSIM.DLL | LoadPlugin | 0x3650D | Tap: writes the renderer DLL it tries to load, its type id, what its `CheckFunc` returns and which type the game asked for, to `trace.log`. |
| HGSIM.DLL | PluginChoice | 0x3695B | Tap: writes which renderer or sound plugin the game picked. |
| HGSIM.DLL | CdAudioInit | 0x9DFA0 | Reports "no CD music" without the "Cannot aquire audio CD device" box. Returns 1 as the original does on failure; callers run without music. |
| HGSIM.DLL | AllocPolyBuffers | 0xA8816 | Asks for a 16 MiB polygon arena and 32000 sort entries instead of 1 MiB and 3000. |
| HGSIM.DLL | SortByDepth | 0xA4700 | The original quicksort, same results, but recursing only into the smaller half so big lists cannot overflow the stack. |
| HGSIM.DLL | VisibleCells | 0x87D83 | Lists every grid cell, nearest ring first, instead of the camera cell and the few in front of it. |
| HGSIM.DLL | SetCurrentCamera | 0xD1F90 | Main camera far plane at least 1,000,000; sub-objects at any distance; polygon soft limit raised. |
| HGSIM.DLL | ProjectVertex | 0xA62C4 | Remembers each corner's exact screen position (the game truncates to whole pixels); a wrapper around the plugin's Render puts it back, so polygons no longer wobble when the picture is scaled up. |
| HGSIM.DLL | CameraViewport | 0xD2306 | LOD scale 1e20, so static models always use their first (finest) LOD node. |
| HGSIM.DLL | UpdateGearLods | 0x3B534 | Replaced: every gear gets LOD level 0, not just the nearest one. |
| HGSIM.DLL | option setters | 0x988FC, 0x989A1, 0x90D15, 0x585C4, 0xAC603 | Textures, terrain textures, particles, multiple lights and shadows always on. |

| HGSIM.DLL | VFX line, shape (clipped and unclipped), pane wipe, character draw | 0xEF727, 0xF0205, 0xF0727, 0xF0865, 0xF1C93 | The 2D drawing library behind the HUD. The original runs as before (into a scratch buffer), then the same drawing is repeated at the window's resolution in HGVK.DLL. See `docs/renderer/hud.md`. The word `nohudhi` leaves them out (`dev.cmd` option C). |

The last seven live in `src/hooks/hooks_view.c` (research in
`docs/renderer/detail-and-distance.md`). The word `nodetail` in `hgrc.cfg`
leaves them out (`dev.cmd` option 6).
