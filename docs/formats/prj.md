# HEAVYG.PRJ / HEAVYG.MAP asset archive

`HEAVYG.PRJ` (96 MB) holds almost all game assets. `HEAVYG.MAP` is its index.
All integers are little-endian. Verified against the retail files: all 19,545
entries tile the PRJ with no gaps and decode to their stated size
(`tools/prj/prjtool.py verify`).

## HEAVYG.MAP

| Offset | Size | Field |
| --- | --- | --- |
| 0 | 2 | Magic `YL` |
| 2 | 2 | Zero |
| 4 | 4 | Number of types (20) |
| 8 | 8 x types | Per type: `char ext[4]`, `u32 count` (e.g. `WTB` 10345, `XEL` 1508) |
| … | 4 | Unknown, always 0 |
| … | 4 | Number of compressors (2) |
| … | 16 x n | Compressor DLL names: `pkware.dll`, `zlib.dll` |
| … | 20 x entries | Entries, below |

Entry (20 bytes):

| Size | Field |
| --- | --- |
| 8 | Name, NUL-padded (may hold junk after the NUL) |
| 4 | Type / extension, NUL-padded |
| 4 | Offset of the entry header in HEAVYG.PRJ |
| 4 | Always `NONE` |

Entries are grouped by type in the order of the type table, sorted by name.

## HEAVYG.PRJ entry

| Size | Field |
| --- | --- |
| 12 | Tag: `C:\PRJ\` + group letter + type, NUL-padded, not always terminated (`C:\PRJ\AWAVX`) |
| 2 | Compression: 0 none, 1 PKWARE DCL implode, 2 zlib |
| 4 | Unpacked size |
| 4 | Packed size |
| packed size | Data |

Counts in the retail archive: 2,302 stored, 1,419 PKWARE, 15,824 zlib.

## Groups

The letter after `C:\PRJ\` splits the archive into 25 groups (`T` 4,419 entries,
`O` 3,609, `G` 2,627, `W` 2,042, `B` 1,772, `V`, `A`, `M`, `H`, `D`, `S`, …).
Their meaning is not known yet. `A` holds sounds, `B` full-screen PCX
backgrounds, `M` the BWD/BRF mission files.

Six names repeat inside one group with different data (`B47UNDER.PCX` six
times in `B`, `B20UNDER.PCX`, `O_1CRGZW.SDF`, …). Which copy the game loads is
an open question for the decompilation.
