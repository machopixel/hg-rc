# hg-rc
Heavy Gear Recompilation Project

A port of Activision's Heavy Gear (1997) to modern Windows. The game logic is
decompiled to C17 and kept as it was; rendering, controls, audio and video move
to a new platform layer (SDL3 by default).

This repository holds no game files. Tools and builds read them from your own
install of Heavy Gear.

## Approach

1. Decompile with Ghidra, one function at a time, into C.
2. Inject rewritten functions into the original DLLs through a hook layer, so
   the game stays playable at every step.
3. Replace the platform code (DirectDraw, DirectSound, DirectInput, Smacker,
   Win32 window code) with the new layer.
4. Keep the simulation ticking at its original rate, independent of frame rate.

The full plan, with milestones, lives in the project's plan document.

## Milestones

- [x] M0 Repository and layout
- [x] M1 Asset tools: read every entry of HEAVYG.PRJ
- [x] M2 Launcher: HG.EXE rewritten in C (`src/launcher`, see `docs/launcher.md`)
- [x] M3 Hook layer: one HGSIM function replaced at runtime (`src/hooks`, see `docs/hooks.md`)
- [ ] M4 Renderer: the DX5DRAW plugin interface on a modern GPU API
- [ ] M5 Sim beachhead: HGSIM memory, file and math modules

## Original binaries

All built with Visual C++ 5.0 in November 1997, plain C, no debug symbols.

| Module | Role | Exports |
| --- | --- | --- |
| HG.EXE | Launcher | none |
| HGSHELL.DLL | Menus, campaign, multiplayer | ShellMain, ShellWindowProc |
| HGSIM.DLL | Simulation and engine | SimMain, SimWindowProc |
| PLUGIN\*.DLL | Renderers (DirectDraw, Glide, software) | 16 to 19 functions |

## Layout

| Path | Contents |
| --- | --- |
| `src/launcher/` | HG.EXE in C |
| `src/hooks/` | Replaces functions inside the original DLLs at runtime |
| `docs/` | Notes on file formats and rewritten modules |
| `tools/prj/` | HEAVYG.PRJ reader and extractor |
| `tools/res/` | Extracts the original icons for the build |

## Building

Needs Visual Studio 2022 (C workload) and CMake 3.21 or newer. Everything
builds 32-bit, because the game DLLs are 32-bit.

```
cmake -S . -B build -A Win32
cmake --build build --config Release
```

The launcher lands in `build\src\launcher\Release\HG.exe`. To give it the
original icons:

```
pip install pefile
python tools\res\extract_icons.py C:\Path\To\HeavyGear\HG.EXE build\icons
cmake -S . -B build -A Win32 -DHG_ICON_DIR=%CD%\build\icons
```

To try it, copy `HG.exe` over `HG.EXE` in a **copy** of your game folder and
run it from there.

## Tools

Need Python 3.8 or newer, nothing else.

```
python tools\prj\prjtool.py verify  C:\Path\To\HeavyGear
python tools\prj\prjtool.py list    C:\Path\To\HeavyGear --type WAV
python tools\prj\prjtool.py extract C:\Path\To\HeavyGear extracted
```

`verify` decodes all 19,545 entries and reports any that fail. A full extract
takes about 20 seconds and writes about 200 MB to `extracted\` (ignored by git).
