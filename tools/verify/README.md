# Differential checker

Runs an original HGSIM.DLL function and its C conversion in the same emulated
machine (Unicorn) on the same random inputs and compares the return value, the
saved registers and every byte of writable memory afterwards (the stack below
the call is ignored).

    python3 tools/verify/difftest.py <path to HGSIM.DLL> tools/verify/spec_core.py [FunctionName ...]

Needs Python with `unicorn` and `pefile`, and `i686-w64-mingw32-gcc`/`nm`
(the converted files are compiled into one flat image, with `rt.c` standing in
for the C library). A spec file lists `SOURCES` (relative to `src/`) and
`SPECS`: name, `orig` address, `c` symbol, a `gen(ctx)` that sets up the data
tables and returns the arguments, `void=True` when the return value is unused,
and `nop=[addresses]` for single bytes the emulator cannot run (`push ds`).
Converted code reads the original's data through `SIM_PTR` (src/sim/simmem.h).
Functions that call Windows APIs cannot be run this way.
