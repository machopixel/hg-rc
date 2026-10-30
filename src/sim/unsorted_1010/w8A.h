/*
 * Round 8 (letter A): more of the C runtime linked into HGSIM.DLL (0x10100000..0x1010d000): the low-level
 * file handle functions (seek, write, read, close, size change) on the handle table, small text helpers,
 * local time and the time zone set-up, the environment setter, abort, and two pieces of the floating-point
 * error path.
 *
 * Handle table: 0x10ac3720 holds pointers to blocks of 32 eight-byte slots (+0 Windows handle, +4 flag byte,
 * +5 a byte read ahead), 0x10ac385c the slot count. 0x1017e888 / 0x1017e88c take the error number and the
 * Windows error code. Helpers that are not converted are called through their original address (SIM_FN),
 * Windows through the DLL's import slots.
 */
#ifndef HG_SIM_UNSORTED_1010_W8A_H
#define HG_SIM_UNSORTED_1010_W8A_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#define __stdcall
#endif

/* ---- unconverted helpers, called through their original address ---- */
/* 0x10100fd0: (stream, format, argument list) -> character count or -1: the formatter behind the printf
 * family. */
#define W8A_FN_10100fd0 SIM_FN(int32_t (__cdecl *)(void *, const char *, void *), 0x10100fd0)
/* 0x1010aa60: (pointer to a time in seconds) -> pointer to the static broken-down time (0 for a negative
 * time). */
#define W8A_FN_1010aa60 SIM_FN(uint8_t *(__cdecl *)(const int32_t *), 0x1010aa60)
/* 0x10107670: one signal number. */
#define W8A_FN_10107670 SIM_FN(int32_t (__cdecl *)(int32_t), 0x10107670)
/* 0x1010b7c0: (12-byte result, end pointer out, text, 0, 0, 0, 0): text to the 12-byte number record. */
#define W8A_FN_1010b7c0 SIM_FN(uint32_t (__cdecl *)(uint16_t *, const char **, const char *, int32_t, int32_t, \
                                                   int32_t, int32_t), 0x1010b7c0)
/* 0x10103aa0: the x87 status word (sign-extended). 0x10103ab0: the same, then clears the exception flags.
 * 0x10103ad0: (new, mask) sets the control word bits of mask to new; returns the old word (sign-extended). */
#define W8A_FN_10103aa0 SIM_FN(int32_t (__cdecl *)(void), 0x10103aa0)
#define W8A_FN_10103ab0 SIM_FN(int32_t (__cdecl *)(void), 0x10103ab0)
#define W8A_FN_10103ad0 SIM_FN(int32_t (__cdecl *)(uint32_t, uint32_t), 0x10103ad0)

/* ---- w8A_lowio.c: handle-level file functions ---- */
/* SetFilePointer(handle of fh, offset, 0, origin); clears flag 2 of the slot. The new position, or -1 (error 9
 * for a closed or unknown fh, else the mapped Windows error). */
int32_t __cdecl Crt_LSeek(uint32_t fh, int32_t offset, uint32_t origin);
/* WriteFile of cnt bytes; handles with flag 0x80 (text) write through a 0x400-byte buffer that puts '\r'
 * before each '\n'. Returns the bytes taken from buf (the added '\r' not counted), 0 for a 0x1a first byte on
 * a flag 0x40 handle, else -1. A flag 0x20 handle seeks to its end first. */
int32_t __cdecl Crt_Write(uint32_t fh, const uint8_t *buf, uint32_t cnt);
/* ReadFile of up to cnt bytes (after the read-ahead byte of pipes and devices, flags 0x48); text handles
 * (flag 0x80) turn "\r\n" into '\n' and stop at 0x1a. Returns the bytes left in buf, 0 at end of file or a
 * broken pipe (Windows error 0x6d), else -1. */
int32_t __cdecl Crt_Read(uint32_t fh, uint8_t *buf, uint32_t cnt);
/* CloseHandle (not for 1 / 2 when both share one Windows handle), frees the slot (Sim_101097f0). 0 or -1. */
int32_t __cdecl Crt_Close(uint32_t fh);
/* Grows the file with zero bytes (binary mode for the writes) or cuts it with SetEndOfFile; the position is
 * kept. 0 or -1. */
int32_t __cdecl Crt_ChSize(uint32_t fh, int32_t size);

/* ---- w8A_text.c ---- */
/* Length of s, at most max (reads s[max] when no 0 comes before). */
uint32_t __cdecl Crt_StrNLen(const char *s, uint32_t max);
/* Number of 16-bit characters of s up to and with its 0, or max when there is none in the first max. */
uint32_t __cdecl Crt_WcsNCount(const uint16_t *s, uint32_t max);
/* vsprintf: 0x10100fd0 on a string stream with room 0x7fffffff, then the closing 0. */
int32_t __cdecl Crt_VSPrintf(char *buf, const char *fmt, void *args);
/* Puts the decimal point (byte 0x10159660) after the leading digits of a number text (before an 'e' at the
 * start), moving the rest of the text up by one. */
void __cdecl Crt_ForceDecPoint(char *buf);
/* Fmt_DoubleF with the byte 0x1017e908 set to 1 during the call. */
char *__cdecl Sim_10100f00(const uint32_t *value, char *buf, int ndig);
/* Text to the 12-byte record with 0x1010b7c0, then Sim_10108ce0 of it into out; returns that result. */
int __cdecl Sim_10108da0(uint32_t *out, const char *text);

/* ---- w8A_time.c ---- */
/* Local broken-down time of *t (0 for a negative time): the time zone (0x1015c020) and daylight bias
 * (0x1015c028) applied through 0x1010aa60, isdst (+0x20) set when Sim_1010a0a0 says so. */
uint8_t *__cdecl Crt_LocalTime(const int32_t *t);
/* Time zone set-up from the TZ variable ("XXX[-]h[:m[:s]]YYY") or GetTimeZoneInformation: 0x1015c020 zone
 * seconds, 0x1015c024 daylight flag, 0x1015c028 daylight bias, the names at [0x1015c0b0] / [0x1015c0b4].
 * The return value is what the original leaves in eax (Sim_10109dd0 passes it on). */
uint32_t __cdecl Crt_TzSet(void);

/* ---- w8A_env.c ---- */
/* Sets ("NAME=value") or removes ("NAME=") a variable in the environment copy at [0x1017e8b0], and with
 * primary also in the process (SetEnvironmentVariableA). 0, or -1 on a bad text or a failed block. */
int32_t __cdecl Crt_SetEnv(char *option, int32_t primary);
/* Message 10 (Sim_10108430), signal 0x16 (0x10107670), exit code 3 (Proc_ExitQuick). */
void __cdecl Crt_Abort(void);

/* ---- w8A_math.c: floating-point error path (integer code around the x87 helpers; bit exact) ---- */
/* Fills the 0x54-byte exception record rec from the error flags, *pcw, the status word (0x10103aa0), the
 * operation code and the operand / result doubles, clears the x87 flags (0x10103ab0) and raises the matching
 * floating-point exception (RaiseException); afterwards copies the record's masks, rounding and precision
 * back into *pcw and its result to *result. */
void __cdecl Math_RaiseFpError(uint32_t *rec, uint32_t *pcw, uint32_t flags, uint32_t opcode,
                               const uint32_t *arg1, uint32_t *result);
/* Domain error of the math functions: Math_ReportError(1, op, x, 0, x, cw) unless the dword 0x1015c018 is
 * set; then error 0x21, control word cw restored (0x10103ad0) and x returned (in st(0)). */
double __cdecl Math_DomainError(uint32_t op, double x, uint32_t cw);

#endif
