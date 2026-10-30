/*
 * Round 5 (letter y) functions of HGSIM.DLL in 0x1003xxxx. Plain C, original cdecl convention.
 * They call helpers that are not converted yet through their original address (SIM_FN), so the
 * original code still runs for those. Names carry the original address because the code alone
 * does not say what the data is for.
 */
#ifndef HG_SIM_UNSORTED_1003_W5Y_H
#define HG_SIM_UNSORTED_1003_W5Y_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Scrolls the text screen at 0xb0000 up by one row: copies 0xf00 bytes from 0xb00a0 to 0xb0000 (Sim_100fdf00, memmove). */
void __cdecl Mono_ScrollUp(void);
/*
 * Formats the arguments (printf rules, format text first) into the 0x50-byte buffer at 0x10139f58
 * through Sim_100fe3e0 and returns its result.
 */
int __cdecl Sim_10034891(const char *format, ...);
/*
 * Loads a library by name (LoadLibraryA), keeps the module in 0x1015eb68, lets Sim_1003be30 look up the
 * library's functions into the DLL's pointer table and returns the module.
 */
void *__cdecl Lib_Load_1003c582(const char *name);
/* Builds the path "vfx/<name>.bin" in a 0x100-byte buffer and returns Sim_100461e8(path, 0). */
void *__cdecl Sim_1003cf35(const char *name);
/* Returns 1 when the dword at 0x1015ec84 is 0, otherwise whether Sim_10041d67() is not 0. */
int __cdecl Sim_1003e26a(void);

#endif
