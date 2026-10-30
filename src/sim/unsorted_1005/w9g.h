/*
 * Round 9, letter g: the function of HGSIM.DLL at 0x1005f4e0 (reads lang.ini).
 */
#ifndef HG_SIM_UNSORTED_1005_W9G_H
#define HG_SIM_UNSORTED_1005_W9G_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* ---- w9g_lang.c ---- */
/*
 * Reads "lang.ini" (0x10144238, mode "r") line by line (0x100 bytes, cleared once before the loop): a line
 * containing "dll =" (0x10144244) or "sim file =" (0x10144250) is cut at "\t\r\n" (Str_NextToken), the text
 * after its '=' is skipped up to the first letter (class 0x103, the CRT's alpha class: Crt_IsCType when
 * [0x1015965c] > 1, else the table [0x10159450]) and copied to `dll` or `simFile`. Nothing without the file.
 * The original keeps skipping past the end of the line (into the stack) when no letter follows the '='.
 */
void __cdecl Lang_ReadIni(char *dll, char *simFile);

#endif
