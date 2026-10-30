/*
 * Round 25: game file loading, the resource and memory cache (original source C:\projx\Code\sim\Mem\sysfile.c /
 * syscache.c; DECISIONS.md, "Round 25"). The C library calls (malloc 0x100fef30 / free 0x100fefe0 on the game heap
 * [0x1017b8f8], fseek, fread, assert 0x100ff860, ...) stay HGSIM's own, at their original addresses.
 */
#ifndef HG_SIM_UNSORTED_100B_W25_H
#define HG_SIM_UNSORTED_100B_W25_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Reads the archive entry at offset of the open archive [0x1017e1c0]: its 0x16-byte header (12-byte tag, compression
 * word +0xc, unpacked size +0xe, packed size +0x12) to tag (8 bytes) / type (3 characters) when given, the sizes to
 * *unpacked / *packed, then the data into a new block (malloc, the heap [0x1017b914] when set, else [0x1017b900]):
 * stored as it is, or unpacked by the decoder [0x1017dfd4 + 0x20 * compression] (from the buffer [0x1017b958] when
 * it holds the packed size, else from a temporary block). Returns the block, 0 when the decoder fails.
 */
void *__cdecl Sim_100bc70d(uint32_t offset, uint32_t *unpacked, uint32_t *packed, char *tag, char *type);

#endif
