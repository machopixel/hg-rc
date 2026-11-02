/*
 * C runtime replacement, the rest (DECISIONS.md, "C runtime replacement: the rest"): time, local time, the
 * environment, leaving the program and assert, on the standard C library and the platform layer in place of Windows
 * and the DLL's unconverted code.
 */
#ifndef HG_SIM_CRT_HG_REST_H
#define HG_SIM_CRT_HG_REST_H

#include <stdint.h>

/* Seconds since 1970-01-01 00:00 UTC (time). */
int32_t hg_time(void);
/* The local time of *t in the original's struct tm layout (nine ints: sec, min, hour, mday, mon, year since 1900,
 * wday, yday, isdst) in a static record, or NULL for a time it cannot convert (localtime). */
int32_t *hg_localtime(const int32_t *t);
/* The text of environment variable name, or NULL (getenv). */
char *hg_getenv(const char *name);
/* Leaves the program with code: after the exit routines (ExitProcess) or at once (TerminateProcess). */
void hg_exit_process(uint32_t code);
void hg_terminate_process(uint32_t code);
/* assert (0x100ff860, reached through its hook): the original's message box text and its three answers. */
void __cdecl Crt_Assert(const char *expr, const char *file, uint32_t line);

#endif
