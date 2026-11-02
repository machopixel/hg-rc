/*
 * The game archive's two packing methods (HEAVYG.PRJ; DECISIONS.md, "Windows calls: the decompressors"), in place of
 * the DLLs the archive names, pkware.dll and zlib.dll, and their Uncompress(out, &size, packed, packedSize):
 *
 *   Unpack_Zlib:   a zlib stream (RFC 1950 / 1951), through zlib's uncompress (third_party/zlib); data after the
 *                  stream is ignored.
 *   Unpack_Pkware: PKWARE's Data Compression Library "implode" format (first byte 0 binary / 1 ASCII literals, then
 *                  the dictionary size 4..6 = 1, 2 or 4 KB), through zlib's contrib/blast; the game's data is all
 *                  binary.
 *
 * *size is the room at out on the way in and the bytes written on the way out. 0 is success; anything else a
 * failure (the game only tests for 0): bad data, data that ends early, or more output than the room.
 */
#ifndef HG_SIM_UTIL_UNPACK_H
#define HG_SIM_UTIL_UNPACK_H

#include <stdint.h>

int Unpack_Zlib(uint8_t *out, uint32_t *size, const uint8_t *packed, uint32_t packedSize);
int Unpack_Pkware(uint8_t *out, uint32_t *size, const uint8_t *packed, uint32_t packedSize);

#endif
