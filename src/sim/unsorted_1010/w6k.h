/*
 * Round 6 (letter k): functions of the C runtime linked into HGSIM.DLL
 * (0x10106660..0x1010d340): the small-block heap regions, the file handle
 * table, stream push-back, command line splitting, string helpers and a few
 * wrappers. Names carry the original address where the code alone does not
 * say what the data means. Helpers that are not converted are called through
 * their original address (SIM_FN), Windows through the DLL's import slots.
 */
#ifndef HG_SIM_UNSORTED_1010_W6K_H
#define HG_SIM_UNSORTED_1010_W6K_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#define __stdcall
#endif

/* ---- unconverted helpers, called through their original address ---- */
/* 0x100fef30: size -> new block or 0. */
#define W6K_FN_100fef30 SIM_FN(void *(__cdecl *)(uint32_t), 0x100fef30)
/* 0x1010afa0: seven dwords (two counted strings and the dwords at 0x1017ebc8 / 0x1017ebc4), result 0..3. */
#define W6K_FN_1010afa0 SIM_FN(int (__cdecl *)(uint32_t, uint32_t, const char *, uint32_t, const char *, uint32_t, uint32_t), 0x1010afa0)
/* 0x10108430: one number; writes a message. */
#define W6K_FN_10108430 SIM_FN(void (__cdecl *)(uint32_t), 0x10108430)
/* 0x101009b0: one number; does not come back in the game (calls the exit routine). */
#define W6K_FN_101009b0 SIM_FN(void (__cdecl *)(uint32_t), 0x101009b0)
/* 0x10109df0: no arguments. */
#define W6K_FN_10109df0 SIM_FN(uint32_t (__cdecl *)(void), 0x10109df0)
/* 0x10103940: the double given as two dwords -> 1..4 for the special values, else 0. */
#define W6K_FN_10103940 SIM_FN(int (__cdecl *)(uint32_t, uint32_t), 0x10103940)
/* 0x1010b7c0: (12-byte out, &end pointer, text, 0, 0, 0, 0). */
#define W6K_FN_1010b7c0 SIM_FN(int (__cdecl *)(void *, const char **, const char *, uint32_t, uint32_t, uint32_t, uint32_t), 0x1010b7c0)
/* 0x100ff280: character -> character (used by Sim_1010d240). */
#define W6K_FN_100ff280 SIM_FN(uint32_t (__cdecl *)(uint32_t), 0x100ff280)
/* 0x1010d390: opens the console output handle into 0x1015c494. */
#define W6K_FN_1010d390 SIM_FN(void (__cdecl *)(void), 0x1010d390)

/* ---- w6k_sbh.c: the small-block heap regions ----
 * A region header is 0x2020 bytes: +0 next, +4 previous region, +8 and +0xc
 * pointers into its entry table, +0x10 start of its 4 MB reserved address
 * range (-1 = region unused), +0x14 its end, +0x18 1024 entries of two dwords,
 * one per 4 KB page. The first region is the static one at 0x10159e38; its +4
 * (0x10159e3c) is the last region. A page has +0 a pointer into its map,
 * +4 a count, the map of 0xf0 bytes at +8 and the byte 0xff at +0xf8.
 */
/* Creates a region (the static one when it is unused, else a 0x2020-byte heap block): reserves 4 MB, commits the
 * first 64 KB, links it and fills the entry table and the 16 page headers. Returns it, or 0. */
uint32_t __cdecl Sim_10106660(void);
/* Frees the address range of a region and unlinks and frees the header (the static region is marked unused
 * instead). Returns the result of the last Windows call. */
uint32_t __cdecl Sim_101067d0(uint32_t region);
/* Decommits up to n pages whose entry first dword is 0xf0, walking back from the last region; releases a region
 * in which it decommitted pages when all its entries are then -1. */
void __cdecl Sim_10106830(int32_t n);
/* Gives the map byte back to the page entry of p in region, clears it and sets the second dword to 0xf1; when the
 * entry becomes 0xf0, counts 0x1017e970 up and at 32 decommits 16 pages. */
void __cdecl Sim_10106960(uint32_t region, uint32_t p, uint8_t *mapbyte);
/* Looks for n free map bytes in a page (avail = free count from the entry); returns the address or 0. */
uint32_t __cdecl Sim_10106c00(uint32_t page, uint32_t avail, uint32_t n);
/* Checks all regions and pages; 0 when consistent, -1..-9 for the first problem found. */
int32_t __cdecl Sim_10106e50(void);

/* ---- w6k_stream.c: stream structures (+0 pointer, +4 count, +8 buffer, +0xc flags, +0x14 one-byte buffer,
 * +0x18 buffer size) ---- */
/* Gives the stream a 4 KB buffer from 0x100fef30 (flag 8), or its own 2-byte buffer at +0x14 (flag 4). */
void __cdecl Sim_10107020(uint8_t *stream);
/* Puts the byte c back in front of the stream; returns c & 0xff or -1. */
int32_t __cdecl Sim_101095f0(int32_t c, uint8_t *stream);
/* Writes one character to the console handle at 0x1015c494 (opened on first use); returns it or -1. */
int32_t __cdecl Sim_1010d2f0(uint32_t c);
/* Converts the 16-bit character to multibyte text at s; returns the byte count, 0 for s == 0, or -1 with
 * 0x2a in 0x1017e888. */
int32_t __cdecl Sim_10109050(char *s, uint32_t wc);

/* ---- w6k_handles.c: the handle table (64 blocks of 32 eight-byte slots, pointers at 0x10ac3720,
 * slot count in 0x10ac385c) ---- */
/* Returns the number of the first slot whose flag bit 1 is clear (its dword set to -1), adding a block from
 * 0x100fef30 when needed; -1 when none. */
int32_t __cdecl Sim_10109680(void);
/* Stores value in a slot whose dword is -1 (also SetStdHandle for 0..2 when 0x1017e900 is 1); 0, or -1 with 9 in
 * 0x1017e888. */
int32_t __cdecl Sim_10109740(uint32_t fh, uint32_t value);
/* Sets a used slot back to -1 (SetStdHandle(.., 0) for 0..2 when 0x1017e900 is 1); 0, or -1 with 9 in 0x1017e888. */
int32_t __cdecl Sim_101097f0(uint32_t fh);
/* 0x101099a0(a, b, 0x40, c). */
uint32_t __cdecl Sim_10109980(uint32_t a, uint32_t b, uint32_t c);
/* _sopen (file name, open flags, share flag 0x10..0x40, permission) -> handle number, or -1 (C runtime replacement,
 * files: opens through Plat_FileCreate). */
int32_t __cdecl Crt_SOpen(const char *path, uint32_t oflag, uint32_t shflag, uint32_t pmode);

/* ---- w6k_text.c: strings ---- */
/* Compares n characters with 0x1010afa0 (flag 1); 0 for n == 0, 0x7fffffff when it fails, else its result - 2. */
int32_t __cdecl Sim_101078d0(const char *a, const char *b, uint32_t n);
/* Searches the "NAME=value" pointer list at [0x1017e8b0] for name (len characters followed by '=' or the end);
 * returns its index, or minus the number of entries. */
int32_t __cdecl Sim_1010b4b0(const char *name, uint32_t len);
/* Copies s into a new block from 0x100fef30; 0 when s is 0 or there is no block. */
char *__cdecl Sim_1010c9f0(const char *s);
/* Compares at most n characters ignoring case (ASCII when the dword at 0x1017e950 is 0, otherwise through
 * 0x100ff280); -1, 0 or 1. */
int32_t __cdecl Sim_1010d240(const uint8_t *a, const uint8_t *b, uint32_t n);
/* Sets the first min(n, strlen(s)) characters of s to c; returns s. */
char *__cdecl Crt_StrNSet(char *s, uint32_t c, uint32_t n);
/* Splits a command line into words (quotes, backslashes, two-byte characters flagged 4 in 0x1017eac1). Writes
 * the pointers to argv and the text to args when they are not 0; counts words + 1 and bytes. */
void __cdecl Sim_10107b20(const uint8_t *cmd, uint32_t *argv, uint8_t *args, int32_t *nargs, int32_t *nchars);
/* Writes '0' and ndig digits of the text at [st+0xc] (padded with '0') to buf, rounds on the next digit; a carry
 * into the first place counts [st+4] up, otherwise the leading '0' is removed. */
void __cdecl Sim_10108e60(char *buf, int32_t ndig, uint8_t *st);

/* ---- w6k_misc.c ---- */
/* Shows a message box through user32.dll functions looked up once (0x1017e9a8..0x1017e9b0); 0 when not found. */
int32_t __cdecl Sim_10107840(const char *text, const char *caption, uint32_t type);
/* Code page: -2 GetOEMCP, -3 GetACP, -4 the dword at 0x1017e960 (each sets 0x1017ebdc to 1), else cp itself. */
uint32_t __cdecl Sim_10107f20(uint32_t cp);
/* When 0x1017e8fc is 1 (or 0 with 0x1017e900 == 1): 0x10108430(0xfc), the callback at 0x1017ebe8, 0x10108430(0xff). */
void __cdecl Sim_101083f0(void);
/* Parses text with 0x1010b7c0 and converts the 12-byte result with Sim_10108d00 into out. */
int __cdecl Sim_10108e20(uint32_t *out, const char *text);
/* 0x101009b0(2). */
void __cdecl Sim_10109040(void);
/* Calls 0x10109df0 once (flag 0x1017ece0); returns its result, or the flag when it was already set. */
uint32_t __cdecl Sim_10109dd0(void);
/* Computes a day number (from a weekday rule or a date) and a time in milliseconds into 0x1015c0b8..0x1015c0c0
 * (which == 1) or 0x1015c0c8..0x1015c0d0 (with the offset at 0x1015c028, wrapped into one day). */
void __cdecl Sim_1010a310(int32_t which, int32_t type, int32_t year, int32_t month, int32_t week, int32_t dow,
                          int32_t day, int32_t hour, int32_t min, int32_t sec, int32_t msec);
/* Classifies a double: 0x200/4 infinities, 2/1 NaNs, 0x80/0x10 denormals, 0x40/0x20 zeros, 0x100/8 normals. */
int __cdecl Sim_1010a9c0(double x);

#endif
