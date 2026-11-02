# OpenLibm in Heavy Gear

- Upstream: https://github.com/JuliaMath/openlibm, release **v0.8.8** (published 2026-09-02), archive
  `https://github.com/JuliaMath/openlibm/archive/refs/tags/v0.8.8.tar.gz`,
  SHA-256 `721bab18ff37160fd9e903eb211ff26fdee38c3d0041d556b07d76089c435d17`.
- Licence: `LICENSE.md` (BSD / ISC / MIT parts, FDLIBM notices in the source files). `README.md` is upstream's.
- Only the files the game uses are kept, unchanged: `include/openlibm_math.h`, `openlibm_defs.h`,
  `openlibm_complex.h`, every `src/*.h`, and `src/` e_sqrt, e_fmod, s_fabs, e_pow, s_sin, s_cos, k_sin, k_cos,
  e_rem_pio2, k_rem_pio2, s_atan, e_asin, e_acos, s_scalbn, s_copysign, s_floor (`.c`).
- `hg/` holds our two shim headers, found before upstream's (include order `hg`, then `include`): `openlibm_defs.h`
  (no DLL export: the library is compiled into the program) and `openlibm_complex.h` (no complex functions are
  built; `math_private.h` only declares two complex helpers). With them the files build with MSVC and with MinGW.
- They are compiled through `src/sim/libm/` (one wrapper per file), which renames the entry points to `hg_sqrt`,
  `hg_sin` and so on (`src/sim/libm/olm.h`), so they never clash with the compiler's built-in maths or its C runtime.
- To update: replace these files from a newer release, keep `hg/`, rebuild, re-run the libm checks (DECISIONS.md,
  "C runtime replacement").
