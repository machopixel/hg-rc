/*
 * Round 9, letter f: the plugin list loader 0x10036876 of HGSIM.DLL.
 */
#ifndef HG_SIM_UNSORTED_1003_W9F_H
#define HG_SIM_UNSORTED_1003_W9F_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Patched by hooks_sim.c: loads one renderer plugin DLL by path (0x1003650d). */
#define W9F_CALL_1003650d SIM_FN(void(__cdecl *)(const char *), 0x1003650d)

/* ---- w9f_plugin.c ---- */
/*
 * Opens the list file `name` ("r"); without it returns Plugin_LoadAll(). Otherwise every line's first token
 * (separators " \t\r\n") is loaded as "plugin\<token>" with 0x1003650d, the file is closed and the address
 * 0x1015e578 is returned.
 */
uint32_t __cdecl Plugin_LoadList(const char *name);

#endif
