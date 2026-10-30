# Launcher (HG.EXE)

`src/launcher` is a C rewrite of the 34 KB `HG.EXE`. Of its 147 functions,
30 are the game's own code (0x401000 to 0x4028d0); the rest is the Visual C++
5.0 runtime, which the modern compiler supplies.

## What it does

1. Deletes `nmsave.cfg` and `hgprm.cfg`, loads the error messages, refuses to
   run on a 16-colour desktop or when a "Heavy Gear" window already exists.
2. Loads `ANET2.DLL` and creates a 640x480 window (borderless when the desktop
   is no larger than that).
3. Runs `HGSHELL.DLL` with the script `intro`. The shell writes the chosen
   mission to `hgprm.cfg`; bytes 0x240 on are the sim command line.
4. Unloads ANET2 unless that command line contains `/N` or `-N` (network game).
5. Runs `HGSIM.DLL` with that command line, then the shell again with `sim`,
   and loops. A DLL returning -1 is fatal; a low byte of 0xFF means quit.

Running `HG.EXE <args>` skips the shell and starts the sim directly with
`<args>` as its command line.

Both DLLs share the launcher's window. They pick who receives its messages by
sending `WM_USER+30` (default), `WM_USER+31` (sim) or `WM_USER+32` (shell).

## DLL entry points

| Export | Signature (stdcall) |
| --- | --- |
| `ShellMain` | `int (HINSTANCE self, HINSTANCE null, char *script, int 1, HWND window)` |
| `SimMain` | `int (HINSTANCE self, HINSTANCE null, char *cmdLine, int 0, int 1, HWND window)` |
| `ShellWindowProc`, `SimWindowProc` | standard `WNDPROC` |

## Function map

| Address | Function | File |
| --- | --- | --- |
| 0x401000-0x401130 | Mono screen and log debug output | debug.c |
| 0x4011e0 | UnloadAnet | main.c |
| 0x401350 | LoadAnet | main.c |
| 0x401370 | Decrypt | strtable.c |
| 0x401400 | DecryptFile | strtable.c |
| 0x4014b0, 0x401500, 0x401540 | EncOpen, EncRead, EncClose | strtable.c |
| 0x401570 | StrTable_Load | strtable.c |
| 0x401700 | CompareEntries | strtable.c |
| 0x401740 | ReadEntry | strtable.c |
| 0x401990 | ReadBlock | strtable.c |
| 0x401ae0 | Unescape | strtable.c |
| 0x401d80 | ApplyOptions | strtable.c |
| 0x401f90 | StrTable_Free | strtable.c |
| 0x401ff0 | StrTable_Lookup | strtable.c |
| 0x402030 | ReadShellFileName | main.c |
| 0x402150 | LoadMessages | main.c |
| 0x4022a0 | MainWndProc | main.c |
| 0x402330 | CreateMainWindow | main.c |
| 0x4024b0 | RunSim | main.c |
| 0x402550 | RunShell | main.c |
| 0x402600 | IsAlreadyRunning | main.c |
| 0x402640 | WinMain | main.c |

Each function carries a `// FUNCTION: HG 0x...` line for later comparison
tooling.

## Translation files

`lang.ini` names the shell's translation file (`shell file = shell.txt`). A
name without `.txt` is decrypted to `temp.txt` first. The format is blocks
separated by blank lines:

```
:OPTIONS:
linenum

:ENGLISH:
Heavy Gear is already running.

:TRANSLATED:
Heavy Gear läuft bereits.
```

Lines in a block are joined with spaces and C escapes (`\n`, `\x41`, `\101`)
are resolved. Options apply to every later entry: `elbonian`/`xtdelbon`
(accented test text), `stars` (masked text), `linenum` (source line prefix).
The retail game ships no `shell.txt`, so the English text is used.

## Differences from the original

Behaviour is kept, including dead code, except for these memory bugs:

- **Message table freed too early.** The original freed the translation
  table right after looking up its messages, leaving translated messages
  pointing at freed memory. The table is now kept.
- **strcat onto a string literal.** The original appended the "See Technical
  Help" text onto the looked-up 256-colour message. Untranslated, that wrote
  past the literal and garbled the 16-colour error. Now joined in a buffer.
- **Elbonian buffer one byte short.** Allocates one more byte.
- **Shell file name buffer.** The original wrote the name from `lang.ini` over
  the 12-byte `"shell.txt"` literal; now a `MAX_PATH` buffer.

Added for modern Windows:

- **256-colour compatibility layer.** The DLLs use 8-bit DirectDraw, which
  shows only a blank window on current Windows. Windows applies its
  "Reduced color mode: 8-bit" fix to the original exe automatically, but
  not to a rebuilt one. The launcher restarts itself once with
  `__COMPAT_LAYER=256Color` (set `HG_NO_COMPAT=1` to skip). This goes away
  when the renderer is replaced.
- **No DEP or ASLR, no manifest, subsystem 5.01.** Linker settings that make
  the process look like a 1997 one; with DEP on, `HGSHELL.DLL` fails to load.
- **Renderer switch.** The shell writes `/3dfx` into the sim's command line
  and the sim picks its renderer plugin by that switch alone (`/3dfx` Glide,
  `/d3d` DirectDraw/Direct3D, none software). Glide needs `glide2x.dll`, which
  no longer exists, and the sim crashes at a null function pointer when the
  plugin will not load. The launcher rewrites `/3dfx` to `/d3d` unless
  `glide2x.dll` is present. `HG_RENDERER=glide|d3d|soft` overrides it.
- **`crash.log`.** An unhandled crash in the launcher or in a game DLL is
  written to `crash.log` next to the exe: the faulting address as
  `MODULE+RVA`, the registers, and the stack values that point into loaded
  modules. Windows still shows its own crash report afterwards.
- **`HG_DEBUG`.** Set to 2 (debugger output) or 4 (`debug.log`) to turn on the
  original's debug messages, plus logging of DLL load errors.

Kept as found, and worth knowing:

- The fatal DLL error passes the number 0x485 as the message text.
- `GetDC(NULL)` is never released.
- The direct-to-sim path checks for `/N` and ignores the result, so ANET2 is
  always unloaded there.
- Debug output mode 1 writes to the DOS monochrome screen at 0xB0000 and
  would crash on Windows; nothing turns it on.
