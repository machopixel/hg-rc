/*
 * Round 7, letter q: C runtime code of HGSIM.DLL at 0x100fd1c0..0x100fef50 (the Visual C++ library the DLL
 * carries): number parsing, upper-casing, quicksort, formatted output to streams, stream flushing and position,
 * directory search, and the retrying allocator.
 *
 * Stream records are used as offsets (as in w6j.h): +0 next byte, +4 bytes left, +8 buffer, +0xc flags,
 * +0x10 file handle, +0x18 buffer size. The error dword is 0x1017e888.
 *
 * Callees that are not converted are called through their original address (SIM_FN):
 *   0x10100fd0 (stream, format, pointer to the variable arguments)   the formatting engine
 *   0x100febc0 (data, size, count, stream)                          writes items to a stream
 *   0x10105750 (file handle, data, count)                           writes bytes to a file handle
 *   0x10105fe0 (file handle, offset, origin)                        moves the file position
 *   0x101053c0 (locale, flags, source, count, out, out size, 0)     maps characters through the locale
 */
#ifndef HG_SIM_UNSORTED_100F_W7Q_H
#define HG_SIM_UNSORTED_100F_W7Q_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

typedef int(__cdecl *W7qOutputFn)(void *stream, const char *format, void *args);
typedef uint32_t(__cdecl *W7qWriteItemsFn)(const void *data, uint32_t size, uint32_t count, void *stream);
typedef int(__cdecl *W7qWriteFn)(uint32_t fd, uint32_t data, int count);
typedef int32_t(__cdecl *W7qSeekFn)(int32_t fd, int32_t offset, int origin);
typedef int(__cdecl *W7qMapStringFn)(uint32_t locale, uint32_t flags, const uint8_t *src, int count, uint8_t *dst,
                                     int dstSize, uint32_t codePage);
typedef int(__cdecl *W7qCompareFn)(const void *, const void *);

#define W7Q_OUTPUT SIM_FN(W7qOutputFn, 0x10100fd0)
#define W7Q_WRITE_ITEMS SIM_FN(W7qWriteItemsFn, 0x100febc0)
#define W7Q_WRITE SIM_FN(W7qWriteFn, 0x10105750)
#define W7Q_SEEK SIM_FN(W7qSeekFn, 0x10105fe0)
#define W7Q_MAP_STRING SIM_FN(W7qMapStringFn, 0x101053c0)

/* ---- w7q_parse.c ---- */
/*
 * _atoi64: skips bytes of class 8 (space), takes one '-' or '+', then adds decimal digits (class 4) with
 * wrapping 64-bit arithmetic; negated for '-'. Returned in edx:eax. Classes come from Crt_IsCType when the
 * dword 0x1015965c is above 1, else from the table at [0x10159450].
 */
int64_t __cdecl Crt_AToI64(const char *s);
/*
 * strtol / strtoul core: skips spaces, takes a sign, reads digits and letters in `base` (0 = from the prefix:
 * "0x"/"0X" hexadecimal, "0" octal, else decimal; a "0x" prefix is also skipped for base 16). flags bit 1 =
 * unsigned result; the function adds 2 (negative), 8 (digits seen) and 4 (overflow). On overflow (or a signed
 * value out of range) stores 0x22 in the error dword and returns the limit. *end (when not null) gets the first
 * unused byte, or the string itself when no digit was read. A base below 0, 1 or above 36 returns 0.
 */
uint32_t __cdecl Crt_StrToXL(const char *s, const char **end, int base, uint32_t flags);
/*
 * toupper: with no locale (dword 0x1017e950 is 0) only 'a'..'z' change. Otherwise characters below 0x100
 * without the lower-case class (2) come back unchanged, and the others are mapped through 0x101053c0 (one byte,
 * or two when the high byte is a lead byte, flag 0x80 in the upper half of its class word).
 */
int __cdecl Crt_ToUpper(int c);

/* ---- w7q_sort.c ---- */
/*
 * qsort: sorts `count` elements of `width` bytes with `compare` (median-of-position pivot, runs of 8 or fewer
 * elements go to Sort_SelectionSort, an explicit stack of 30 ranges keeps the smaller part).
 */
void __cdecl Crt_QSort(uint8_t *base, uint32_t count, uint32_t width, W7qCompareFn compare);
/*
 * malloc core: 0 for sizes above 0xffffffe0, 0 bytes count as 1; Heap_AllocBlock, and while that fails and
 * `retry` is non-zero, asks the new-handler (Sim_10106590) and tries again until it answers 0.
 */
void *__cdecl Heap_MallocRetry(uint32_t size, int retry);

/* ---- w7q_stream.c ---- */
/* fprintf: formats into the stream with 0x10100fd0, with the temporary buffer of Sim_10104ae0 around it. */
int __cdecl Crt_FPrintf(void *stream, const char *format, ...);
/* printf: Crt_FPrintf on the standard output stream at 0x10159b68. */
int __cdecl Crt_Printf(const char *format, ...);
/* fputs: writes the string (without its 0) with 0x100febc0; 0 when every byte went out, else -1. */
int __cdecl Crt_FPuts(const char *s, void *stream);
/*
 * fflush: a null stream flushes all streams (Sim_100fe310(0)). Otherwise Crt_FlushStream, and for streams with
 * flag 0x4000 also the file handle (Sim_101056f0). 0 on success, -1 on failure.
 */
int __cdecl Crt_FFlush(void *stream);
/*
 * Writes the pending bytes of a stream that is open for writing (flags & 3 == 2) and buffered (0x108):
 * on a short write sets flag 0x20 and returns -1; a stream with flag 0x80 also loses flag 2. Always resets the
 * next-byte pointer to the buffer and the count to 0.
 */
int __cdecl Crt_FlushStream(void *stream);
/*
 * ftell: the file position (0x10105fe0(handle, 0, 1)) corrected by the bytes waiting in the buffer, counting
 * each '\n' twice for text-mode handles (flag 0x80 in the handle table 0x10ac3720). -1 on error (error 0x16
 * for a stream that is neither reading nor writing).
 */
int32_t __cdecl Crt_FTell(void *stream);

/* ---- w7q_find.c ---- */
/*
 * _findfirst: FindFirstFileA; on success fills the 0x118-byte result (attributes, 0 for 0x80; creation,
 * access and write times through Time_FromFileTime; size; name) and returns the search handle. On failure maps
 * GetLastError to the error dword (2, 3, 18 -> 2; 8 -> 0xc; else 0x16) and returns -1.
 */
int32_t __cdecl Crt_FindFirst(const char *spec, uint32_t *out);
/* _findnext: like Crt_FindFirst with FindNextFileA on `handle`; 0 on success, -1 on failure. */
int __cdecl Crt_FindNext(uint32_t handle, uint32_t *out);

#endif
