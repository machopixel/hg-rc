# zlib in Heavy Gear

- Upstream: https://github.com/madler/zlib (zlib.net), release **1.3.2** (published 2026-02-17), archive
  `https://github.com/madler/zlib/releases/download/v1.3.2/zlib-1.3.2.tar.gz`,
  SHA-256 `bb329a0a2cd0274d05519d61c667c062e06990d72e125ee2dfa8de64f0119d16`.
- Licence: `LICENSE` (the zlib licence; blast.c / blast.h carry the same terms in their headers). `README` and
  `contrib/blast/README` are upstream's.
- Only the decompression files are kept, unchanged: `zlib.h`, `zconf.h`, `zutil.h/.c`, `adler32.c`, `crc32.h/.c`,
  `inflate.h/.c`, `inftrees.h/.c`, `inffast.h/.c`, `inffixed.h`, `uncompr.c`, and `contrib/blast/blast.h/.c`
  (Mark Adler's decoder for PKWARE's Data Compression Library "implode" format).
- They are compiled straight into the program (src/launcher/CMakeLists.txt) and used only by `src/sim/util/unpack.c`,
  which reads the game archive HEAVYG.PRJ in place of the game's zlib.dll / pkware.dll (DECISIONS.md, "The
  decompressors").
- To update: replace these files from a newer release, rebuild, re-run `tools/verify/unpack_native.c` (every packed
  entry of HEAVYG.PRJ against the original DLLs).
