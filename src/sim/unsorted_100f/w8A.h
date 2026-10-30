/*
 * Round 8 (letter A): C runtime code of HGSIM.DLL around 0x100fd000..0x10100000: stream reading, writing,
 * seeking and buffer flushing (streams: +0 next byte, +4 count, +8 buffer, +0xc flags, +0x10 handle number,
 * +0x18 buffer size), vsnprintf, file removal, the current time and the number-to-text helper.
 * The low-level handle functions they use are in ../unsorted_1010/w8A.h. Helpers that are not converted are
 * called through their original address (SIM_FN), Windows through the DLL's import slots.
 */
#ifndef HG_SIM_UNSORTED_100F_W8A_H
#define HG_SIM_UNSORTED_100F_W8A_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#define __stdcall
#endif

/* 0x10105970: (year, month, day, hour, minute, second, daylight flag) -> seconds since 1970 (local time
 * fields of a SYSTEMTIME). */
#define W8A_FN_10105970 SIM_FN(int32_t (__cdecl *)(int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t), \
                               0x10105970)

/* ---- w8A_stream.c ---- */
/* fread: size * count bytes into buf from the stream buffer, whole buffer sizes straight with Crt_Read, the
 * rest through Sim_10105a60. Returns count, or the number of whole items read (flag 0x10 at end of file,
 * 0x20 on error). */
uint32_t __cdecl Crt_FRead(void *buf, uint32_t size, uint32_t count, uint8_t *stream);
/* fwrite: the same for writing (Crt_Write for whole buffer sizes after Crt_FlushStream, single bytes with
 * Crt_FlushBuf). */
uint32_t __cdecl Crt_FWrite(const void *buf, uint32_t size, uint32_t count, uint8_t *stream);
/* fseek: origin 0, 1 (made absolute with Crt_FTell) or 2; flushes the stream and moves the handle with
 * Crt_LSeek. 0, or -1 (error 0x16 for a bad origin or a closed stream). */
int32_t __cdecl Crt_FSeek(uint8_t *stream, int32_t offset, uint32_t origin);
/* _flsbuf: writes the stream buffer (getting one first with Sim_10107020 when it has none) and puts ch in
 * it, or writes ch alone for an unbuffered stream. ch & 0xff, or -1 with flag 0x20. */
int32_t __cdecl Crt_FlushBuf(int32_t ch, void *stream);
/* vsnprintf: 0x10100fd0 on a string stream of room count, then the closing 0 when there is room. */
int32_t __cdecl Crt_VSNPrintf(char *buf, uint32_t count, const char *fmt, void *args);
/* DeleteFileA; 0, or -1 with the mapped Windows error. */
int32_t __cdecl Crt_Remove(const char *path);

/* ---- w8A_misc.c ---- */
/* time: local time from GetLocalTime turned into seconds with 0x10105970; the daylight flag comes from
 * GetTimeZoneInformation, cached for the UTC minute (0x1017e8e0, flag in 0x1017e8d8). Stored to *timer when
 * timer is not 0. */
int32_t __cdecl Crt_Time(int32_t *timer);
/* Writes val in base radix (digits 0-9 a-z) to buf, with a '-' and val negated first when neg is set. */
void __cdecl Crt_NumToText(uint32_t val, char *buf, uint32_t radix, int32_t neg);

#endif
