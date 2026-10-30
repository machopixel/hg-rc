# Converted simulation code (HGSIM.DLL)

The original HGSIM.DLL is being replaced function by function with C that behaves exactly the same.
The plan and the order are in the Claude Doc "Heavy Gear sim core migration plan".

## Folders (`src/sim/`)

| Folder | What is in it |
| --- | --- |
| `mem/` | The memory cache module (cache records, hash tables, string helpers). |
| `core/` | The fixed-speed driver (`tick.c`) and the timer module (`timer.c`). |
| `util/` | Small helpers called from many places: table lookups, ring tables, object field accessors, record appenders. |
| `view/` | Rectangle scaling to the display size, 3x4 matrix helpers, object lists. |
| `unsorted_<xxxx>/` (1000 to 1010) | Functions whose purpose is not shown by the code yet, held by the address block they sit in (0x1003..., 0x1004..., 0x100d...). They move to a named folder once someone proves what they are for. |
| `simmem.h`, `simmem.c` | Access to the original DLL's data (`SIM_PTR`), through the address it has at its preferred base. |

The files named `w2*_*.c` are wave 2: functions that only call functions converted earlier. Round 2 (letters e to l) also takes functions whose callees are converted in the same batch. Their `Sim_<address>` bodies call the converted C functions by name.

Rules for all of it:

- Plain C17. Every function keeps the original calling convention and has `// FUNCTION: HGSIM 0x...` above it.
- A name that describes a function is used only when the code proves what it is for; otherwise it is `Sim_<address>`.
- Nothing in `src/sim` knows about the hook layer. `src/hooks/hooks_core.c` and `src/hooks/hooks_conv.c` (with the `conv_*.inc` lists) put each function in place of the original. `hgrc.cfg` word `noconv` turns all of them off.

## Checking a conversion

`tools/verify/difftest.py` runs the original function and the C version in the same emulated machine on the same random inputs and compares the result and all memory (see `tools/verify/README.md`).
Each group has a spec file in `tools/verify/` (`spec_core.py`, `spec_util.py`, `spec_view.py`, `spec_1003.py`, `spec_1004.py`, `spec_100d.py`, `spec_w2a.py` to `spec_w2l.py`).
New hook lines are produced with `tools/verify/hookgen.py`.
A function is hooked in only after it shows 0 mismatches.
