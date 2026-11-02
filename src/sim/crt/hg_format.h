/*
 * C runtime replacement, formatting (DECISIONS.md, "C runtime replacement: formatting"): the printf family on SDL3's
 * own formatter (SDL_snprintf, the same code on every system) in place of the DLL's engine 0x10100fd0. The game's
 * format strings are walked here and each conversion is handed to SDL_snprintf on its own, so the conversions SDL
 * does not know are done here: the space flag ("% 2.2d"). sscanf is SDL_vsscanf.
 */
#ifndef HG_SIM_CRT_HG_FORMAT_H
#define HG_SIM_CRT_HG_FORMAT_H

#include <stdarg.h>
#include <stddef.h>

/* Formats into buf (at most size - 1 characters and a closing 0 when size > 0; buf may be NULL for size 0).
 * Returns the length of the whole result, as if there were room. */
int hg_vformat(char *buf, size_t size, const char *fmt, va_list ap);
/* sscanf: the number of fields stored, or -1 (EOF) when the text ends before the first conversion. */
int hg_vscan(const char *text, const char *fmt, va_list ap);

#endif
