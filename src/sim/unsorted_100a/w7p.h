/*
 * Round 7, letter p: the scrambled file records of 0x100ae344..0x100ae68f. A record is three dwords:
 * { FILE *stream; position (the scramble seed of the next byte); mode (1 "wb", 2 "ab", 3 "r+b") }.
 * The C library stream functions of the DLL that are not converted yet are called through their address.
 */
#ifndef HG_SIM_UNSORTED_100A_W7P_H
#define HG_SIM_UNSORTED_100A_W7P_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Not converted: fwrite(buffer, size, count, stream) of the DLL's C library; returns the count written. */
#define W7P_CALL_100febc0 SIM_FN(uint32_t(__cdecl *)(const void *, uint32_t, uint32_t, uint32_t), 0x100febc0)
/* Not converted: fclose(stream) of the DLL's C library. */
#define W7P_CALL_100fd760 SIM_FN(int(__cdecl *)(uint32_t), 0x100fd760)

/*
 * Opens name with mode "wb" (string at 0x1014d1e8) into a new 12-byte record (Heap_Malloc): stream, position 0,
 * mode 1. Returns 0 when the allocation fails, and frees the record and returns 0 when the open fails.
 */
void *__cdecl FileRec_OpenWrite(const char *name);
/* The same with "ab" (0x1014d1ec), mode 2. */
void *__cdecl FileRec_OpenAppend(const char *name);
/* The same with "r+b" (0x1014d1f0), mode 3. */
void *__cdecl FileRec_OpenUpdate(const char *name);
/*
 * Writes count bytes of src scrambled: for mode 2 first Sim_100ae64d(rec, 0, 2) (0 is returned when that fails);
 * then in pieces of 0x200 bytes (and the rest): the piece is copied to a stack buffer, scrambled with
 * Sim_1003c7b0(buffer, n, position), written with fwrite(buffer, 1, n, stream); a short write ends it. The
 * position grows by each written piece. Returns the number of bytes written in whole pieces.
 */
uint32_t __cdecl FileRec_WriteScrambled(void *rec, const uint8_t *src, uint32_t count);
/* fclose(stream); when that answers non-zero returns it (the record stays), else frees the record and returns 0. */
int __cdecl FileRec_Close(void *rec);

#endif
