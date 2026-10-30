# The conversion checker (tools/verify)

Every converted HGSIM function is proven identical to the original before it is
hooked in. `tools/verify/difftest.py` loads the original `HGSIM.DLL` and the C
code into an emulator (Unicorn), calls both with the same random inputs, and
compares the result registers and every byte of writable memory afterwards. A
function is only kept when there are no differences. No AI is involved in the
checking.

    python3 tools/verify/difftest.py [-jN] HGSIM.DLL tools/verify/spec_xxx.py [function name ...]

* `-jN` splits the functions of a spec over N processes (balanced by trial
  count). A whole spec of 50 functions takes about a minute with `-j4`.
* Needs `python3` with `unicorn`, `pefile` and `capstone`, and
  `i686-w64-mingw32-gcc` to build the C.

## What a spec file contains

`SOURCES` (the .c files to build), `CFLAGS`, and `SPECS`: one entry per
function with `name`, `orig` (original address), `c` (C symbol), `gen(ctx)`
(random arguments), `trials`, and optionally `void=True` (ignore eax),
`nop=[addresses]`, `fp=True` and `tol` (float words within a relative
tolerance count as equal) and `api` (see below).

## Speed (2026-10-19)

The checker used to copy about 12 MB of memory several times per trial. It now
notes which 4 KB pages the function (and the argument generator) wrote and only
saves, compares and restores those pages. The stack and argument areas are
small and always copied whole. This is about 5 times faster per process.
Mistakes are still caught: a deliberately broken C function was flagged, both
in argument memory and in the DLL's own data.

## Windows functions

The DLL calls Windows through its import slots. In the checker every import
slot points at a small stub (the argument sizes come from
`tools/verify/winapi_args.json`, taken from the MinGW import libraries). A stub
logs the function name and arguments, answers with a fixed value (1 unless the
spec's `api` dict supplies `{'HeapAlloc': lambda callNumber, args: value}`),
and returns. A handler with three parameters `(callNumber, args, uc)` can also write results (an out struct) with `uc.mem_write`. The log of the original and of the C version must be identical.
Pointers into the stack are logged as `stack`, because the two runs use
different stack frames.

C code that needs a Windows function calls it through the same slot:

    SIM_API(HeapAllocFn, 0x10ac5658)(heap, 9, size)

(`SIM_API` is in `src/sim/simmem.h`; the number is the import slot address at
the DLL's preferred base.) Functions that use DirectDraw or DirectSound
objects (COM) cannot be checked this way and stay original for now.

Run-time switches (in `hgrc.cfg`, or in `hgrc_extra.cfg` in the game folder, which
dev.cmd does not rewrite): `noconv` installs no converted function, `convoff=TEXT`
leaves out the hooks of every `conv_*.inc` whose name contains TEXT (for bisecting a
fault, e.g. `convoff=conv_w8`), and `soundtrace` logs sound starts and failed or WAV
resource loads to debug.log (see `HooksConv_OnSimLoaded` in src/hooks/hooks_conv.c).

Known limits of every float proof: (1) the checker runs the x87 at 53-bit precision (control word 0x27f); the C (SSE2 doubles) matches the original only if the game also runs at 53-bit precision. (2) When two DIFFERENT NaNs meet in one operation, real x87 keeps the one with the larger payload and SSE keeps the first operand; the emulator behaves the same for both, so this NaN-payload difference is never shown (comparisons, int conversion and non-NaN values are not affected).
